/* Isolated codec executable: only inherited input/output descriptors, no paths.
 * Links against the pinned, separately built codec libraries. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <png.h>
#include <jpeglib.h>
#include <gif_lib.h>

typedef struct { unsigned w,h; int alpha,frames,jpeg; unsigned char *rgba; } Image;
static uint64_t pixel_limit,byte_limit;
static void die(const char *error){
    printf("{\"ok\":false,\"error\":\"%s\",\"data\":null}\n",error);
    fflush(stdout);exit(1);
}
static uint64_t number(const char *s){
    char *end;unsigned long long n=strtoull(s,&end,10);
    if(!*s||*end||*s=='-'||!n)die("INVALID_ARGUMENT");return n;
}
static void dimensions(unsigned w,unsigned h){
    if(!w||!h)die("INVALID_IMAGE");
    if((uint64_t)w*h>pixel_limit||(uint64_t)w*h>SIZE_MAX/4)die("PIXEL_LIMIT");
}
static unsigned char *pixels(unsigned w,unsigned h){
    dimensions(w,h);unsigned char *p=calloc((size_t)w*h,4);
    if(!p)die("ALLOCATION_FAILED");return p;
}
static Image read_png(FILE *in){
    png_image p;memset(&p,0,sizeof p);p.version=PNG_IMAGE_VERSION;
    if(!png_image_begin_read_from_stdio(&p,in))die("INVALID_IMAGE");
    dimensions(p.width,p.height);
    Image im={.w=p.width,.h=p.height,.alpha=!!(p.format&PNG_FORMAT_FLAG_ALPHA),.frames=1};
    im.rgba=pixels(im.w,im.h);p.format=PNG_FORMAT_RGBA;
    if(!png_image_finish_read(&p,NULL,im.rgba,0,NULL))die("INVALID_IMAGE");
    png_image_free(&p);return im;
}
static void jpeg_error(j_common_ptr p){(void)p;die("INVALID_IMAGE");}
static void jpeg_message(j_common_ptr p,int level){if(level<0)jpeg_error(p);}
static Image read_jpeg(FILE *in){
    struct jpeg_decompress_struct d;struct jpeg_error_mgr e;
    memset(&d,0,sizeof d);d.err=jpeg_std_error(&e);e.error_exit=jpeg_error;e.emit_message=jpeg_message;
    jpeg_create_decompress(&d);jpeg_stdio_src(&d,in);
    if(jpeg_read_header(&d,TRUE)!=JPEG_HEADER_OK)die("INVALID_IMAGE");
    dimensions(d.image_width,d.image_height);d.out_color_space=JCS_RGB;
    jpeg_start_decompress(&d);
    Image im={.w=d.output_width,.h=d.output_height,.frames=1,.jpeg=1};
    im.rgba=pixels(im.w,im.h);
    unsigned char *row=malloc((size_t)im.w*3);if(!row)die("ALLOCATION_FAILED");
    while(d.output_scanline<d.output_height){
        unsigned y=d.output_scanline;JSAMPROW rows[]={row};
        if(jpeg_read_scanlines(&d,rows,1)!=1)die("INVALID_IMAGE");
        for(unsigned x=0;x<im.w;x++){memcpy(im.rgba+((size_t)y*im.w+x)*4,row+x*3,3);im.rgba[((size_t)y*im.w+x)*4+3]=255;}
    }
    if(!jpeg_finish_decompress(&d))die("INVALID_IMAGE");
    jpeg_destroy_decompress(&d);free(row);return im;
}
static int gif_read(GifFileType *gif,GifByteType *buffer,int length){
    if(length<0)return 0;return (int)fread(buffer,1,(size_t)length,(FILE*)gif->UserData);
}
static Image read_gif(FILE *in){
    int error=0;
    GifFileType *g=DGifOpen(in,gif_read,&error);if(!g)die("INVALID_IMAGE");
    Image im={.w=g->SWidth,.h=g->SHeight};
    int transparent=NO_TRANSPARENT_COLOR;GifRecordType record;
    while(1){
        if(DGifGetRecordType(g,&record)==GIF_ERROR)die("INVALID_IMAGE");
        if(record==TERMINATE_RECORD_TYPE)break;
        if(record==EXTENSION_RECORD_TYPE){
            int code;GifByteType *block=NULL;
            if(DGifGetExtension(g,&code,&block)==GIF_ERROR)die("INVALID_IMAGE");
            if(code==GRAPHICS_EXT_FUNC_CODE&&block){GraphicsControlBlock control;
                if(DGifExtensionToGCB(block[0],block+1,&control)==GIF_ERROR)die("INVALID_IMAGE");
                transparent=control.TransparentColor;
            }
            while(block)if(DGifGetExtensionNext(g,&block)==GIF_ERROR)die("INVALID_IMAGE");
        }else if(record==IMAGE_DESC_RECORD_TYPE){
            if(DGifGetImageDesc(g)==GIF_ERROR)die("INVALID_IMAGE");
            if(!im.rgba)im.rgba=pixels(im.w,im.h);
            GifImageDesc d=g->Image;
            if(d.Width<=0||d.Height<=0||d.Left<0||d.Top<0||
               (uint64_t)d.Left+d.Width>im.w||(uint64_t)d.Top+d.Height>im.h)die("INVALID_IMAGE");
            ColorMapObject *colors=d.ColorMap?d.ColorMap:g->SColorMap;
            if(!colors)die("INVALID_IMAGE");
            GifPixelType *row=malloc(d.Width);if(!row)die("ALLOCATION_FAILED");
            if(im.frames==0){
                im.alpha=transparent!=NO_TRANSPARENT_COLOR;
                if(!im.alpha&&g->SColorMap&&g->SBackGroundColor<g->SColorMap->ColorCount){
                    GifColorType bg=g->SColorMap->Colors[g->SBackGroundColor];
                    for(size_t i=0;i<(size_t)im.w*im.h;i++){im.rgba[i*4]=bg.Red;im.rgba[i*4+1]=bg.Green;im.rgba[i*4+2]=bg.Blue;im.rgba[i*4+3]=255;}
                }
            }
            static const int starts[]={0,4,2,1},steps[]={8,8,4,2};
            int passes=d.Interlace?4:1;
            for(int pass=0;pass<passes;pass++)for(int y=d.Interlace?starts[pass]:0;y<d.Height;y+=d.Interlace?steps[pass]:1){
                if(DGifGetLine(g,row,d.Width)==GIF_ERROR)die("INVALID_IMAGE");
                for(int x=0;x<d.Width;x++){
                    int index=row[x];if(index>=colors->ColorCount)die("INVALID_IMAGE");
                    if(im.frames==0&&index!=transparent){
                        GifColorType c=colors->Colors[index];unsigned char *p=im.rgba+((size_t)(d.Top+y)*im.w+d.Left+x)*4;
                        p[0]=c.Red;p[1]=c.Green;p[2]=c.Blue;p[3]=255;
                    }
                }
            }
            free(row);if(im.frames==INT_MAX)die("INVALID_IMAGE");im.frames++;transparent=NO_TRANSPARENT_COLOR;
        }else die("INVALID_IMAGE");
    }
    if(!im.frames)die("INVALID_IMAGE");
    if(DGifCloseFile(g,&error)==GIF_ERROR)die("INVALID_IMAGE");return im;
}
static Image resize_image(Image *source,unsigned edge){
    unsigned longest=source->w>source->h?source->w:source->h;
    Image out=*source;out.rgba=NULL;
    if(edge<longest){out.w=(uint64_t)source->w*edge/longest;out.h=(uint64_t)source->h*edge/longest;if(!out.w)out.w=1;if(!out.h)out.h=1;}
    out.rgba=pixels(out.w,out.h);
    for(unsigned y=0;y<out.h;y++)for(unsigned x=0;x<out.w;x++){
        unsigned sx=(uint64_t)x*source->w/out.w,sy=(uint64_t)y*source->h/out.h;
        memcpy(out.rgba+((size_t)y*out.w+x)*4,source->rgba+((size_t)sy*source->w+sx)*4,4);
    }
    return out;
}
static void write_image(FILE *out,Image *im,int quality){
    if(im->jpeg){
        struct jpeg_compress_struct c;struct jpeg_error_mgr e;memset(&c,0,sizeof c);
        c.err=jpeg_std_error(&e);e.error_exit=jpeg_error;e.emit_message=jpeg_message;
        jpeg_create_compress(&c);jpeg_stdio_dest(&c,out);c.image_width=im->w;c.image_height=im->h;c.input_components=3;c.in_color_space=JCS_RGB;
        jpeg_set_defaults(&c);jpeg_set_quality(&c,quality,TRUE);jpeg_start_compress(&c,TRUE);
        unsigned char *row=malloc((size_t)im->w*3);if(!row)die("ALLOCATION_FAILED");
        while(c.next_scanline<c.image_height){for(unsigned x=0;x<im->w;x++)memcpy(row+x*3,im->rgba+((size_t)c.next_scanline*im->w+x)*4,3);JSAMPROW rows[]={row};if(jpeg_write_scanlines(&c,rows,1)!=1)die("IO_ERROR");}
        jpeg_finish_compress(&c);jpeg_destroy_compress(&c);free(row);
    }else{
        png_image p;memset(&p,0,sizeof p);p.version=PNG_IMAGE_VERSION;p.width=im->w;p.height=im->h;p.format=PNG_FORMAT_RGBA;
        if(!png_image_write_to_stdio(&p,out,0,im->rgba,0,NULL))die("IO_ERROR");png_image_free(&p);
    }
    if(fflush(out)||fsync(fileno(out)))die("IO_ERROR");
}
int main(int argc,char **argv){
    if(argc!=7)return 2;
    pixel_limit=number(argv[1]);byte_limit=number(argv[2]);unsigned quality=number(argv[3]);
    unsigned edges[2]={strtoul(argv[4],NULL,10),strtoul(argv[5],NULL,10)};
    FILE *in=fdopen(3,"rb");if(!in)die("IO_ERROR");
    struct stat st;if(fstat(3,&st)||!S_ISREG(st.st_mode))die("IO_ERROR");
    if(st.st_size<0||(uint64_t)st.st_size>byte_limit)die("SOURCE_LIMIT");
    /* Snapshot bounded bytes before decoding; concurrent growth cannot bypass
       max_bytes, nor can a decoder retain the caller's mutable input stream. */
    size_t size=(size_t)st.st_size;
    unsigned char *source=malloc(size?size:1);if(!source)die("ALLOCATION_FAILED");
    if(fread(source,1,size,in)!=size)die("INVALID_IMAGE");
    if(fgetc(in)!=EOF)die("SOURCE_LIMIT");if(ferror(in))die("IO_ERROR");
    struct stat after;
    if(fstat(3,&after)||st.st_size!=after.st_size||st.st_mtim.tv_sec!=after.st_mtim.tv_sec||
       st.st_mtim.tv_nsec!=after.st_mtim.tv_nsec||st.st_ctim.tv_sec!=after.st_ctim.tv_sec||
       st.st_ctim.tv_nsec!=after.st_ctim.tv_nsec)die("FILE_CHANGED");
    if(fclose(in))die("IO_ERROR");
    in=fmemopen(source,size,"rb");if(!in)die("IO_ERROR");
    unsigned char magic[8];if(fread(magic,1,8,in)!=8)die("INVALID_IMAGE");rewind(in);
    Image im;const char *mime;
    if(!png_sig_cmp(magic,0,8)){mime="image/png";im=read_png(in);}
    else if(magic[0]==0xff&&magic[1]==0xd8){mime="image/jpeg";im=read_jpeg(in);}
    else if(!memcmp(magic,"GIF87a",6)||!memcmp(magic,"GIF89a",6)){mime="image/gif";im=read_gif(in);}
    else die("INVALID_IMAGE");
    if(fclose(in))die("IO_ERROR");free(source);
    if(*argv[6]&&strcmp(argv[6],mime))die("TYPE_MISMATCH");
    unsigned sizes[2][2]={{0}};
    for(int i=0;i<2;i++)if(edges[i]){
        Image resized=resize_image(&im,edges[i]);sizes[i][0]=resized.w;sizes[i][1]=resized.h;
        FILE *out=fdopen(4+i,"wb");if(!out)die("IO_ERROR");write_image(out,&resized,quality);free(resized.rgba);
        if(fclose(out))die("IO_ERROR");
    }
    printf("{\"ok\":true,\"error\":\"\",\"data\":{\"width\":%u,\"height\":%u,\"mime\":\"%s\",\"alpha\":%s,\"frames\":%d,\"jpeg_quality\":%u,\"outputs\":[",im.w,im.h,mime,im.alpha?"true":"false",im.frames,quality);
    int count=0;for(int i=0;i<2;i++)if(edges[i])printf("%s{\"width\":%u,\"height\":%u,\"mime\":\"%s\"}",count++?",":"",sizes[i][0],sizes[i][1],im.jpeg?"image/jpeg":"image/png");
    puts("]}}");free(im.rgba);return fflush(stdout)?2:0;
}
