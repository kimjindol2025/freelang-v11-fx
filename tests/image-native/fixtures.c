/* Synthetic images only. Links to the same pinned codecs as the worker. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <png.h>
#include <jpeglib.h>
#include <gif_lib.h>
static void require(int yes){if(!yes){fputs("FIXTURE_ERROR\n",stderr);exit(1);}}
static void png(const char *name,unsigned w,unsigned h){
    unsigned char *pixels=malloc((size_t)w*h*4);require(pixels!=NULL);
    for(size_t i=0;i<(size_t)w*h;i++){pixels[i*4]=i%251;pixels[i*4+1]=i%241;pixels[i*4+2]=i%239;pixels[i*4+3]=(i%3)?255:0;}
    png_image p;memset(&p,0,sizeof p);p.version=PNG_IMAGE_VERSION;p.width=w;p.height=h;p.format=PNG_FORMAT_RGBA;
    require(png_image_write_to_file(&p,name,0,pixels,0,NULL));png_image_free(&p);free(pixels);
}
static void jpeg(const char *name,int progressive){
    FILE *out=fopen(name,"wb");require(out!=NULL);struct jpeg_compress_struct c;struct jpeg_error_mgr e;
    c.err=jpeg_std_error(&e);jpeg_create_compress(&c);jpeg_stdio_dest(&c,out);c.image_width=1600;c.image_height=800;c.input_components=3;c.in_color_space=JCS_RGB;
    jpeg_set_defaults(&c);jpeg_set_quality(&c,90,TRUE);if(progressive)jpeg_simple_progression(&c);jpeg_start_compress(&c,TRUE);
    unsigned char row[4800];while(c.next_scanline<c.image_height){for(int x=0;x<1600;x++){row[x*3]=x%251;row[x*3+1]=c.next_scanline%241;row[x*3+2]=128;}JSAMPROW r[]={row};require(jpeg_write_scanlines(&c,r,1)==1);}
    jpeg_finish_compress(&c);jpeg_destroy_compress(&c);require(!fclose(out));
}
static void gif(void){
    int error=0;GifFileType *g=EGifOpenFileName("animated.gif",0,&error);require(g!=NULL);EGifSetGifVersion(g,true);
    GifColorType colors[2]={{255,0,0},{0,255,0}};ColorMapObject *map=GifMakeMapObject(2,colors);require(map!=NULL);
    require(EGifPutScreenDesc(g,20,10,1,0,map)!=GIF_ERROR);
    for(int frame=0;frame<2;frame++){
        unsigned char extension[4]={1,1,0,1};require(EGifPutExtension(g,GRAPHICS_EXT_FUNC_CODE,4,extension)!=GIF_ERROR);
        require(EGifPutImageDesc(g,0,0,20,10,false,NULL)!=GIF_ERROR);
        for(int y=0;y<10;y++){GifPixelType row[20];for(int x=0;x<20;x++)row[x]=(x+y+frame)%2;require(EGifPutLine(g,row,20)!=GIF_ERROR);}
    }
    require(EGifCloseFile(g,&error)!=GIF_ERROR);GifFreeMapObject(map);
}
static void truncate_copy(const char *source,const char *target){
    FILE *in=fopen(source,"rb"),*out=fopen(target,"wb");require(in&&out);unsigned char data[24];size_t n=fread(data,1,sizeof data,in);require(n==sizeof data);require(fwrite(data,1,n,out)==n);require(!fclose(in)&&!fclose(out));
}
int main(int argc,char **argv){
    require(argc==2&&!chdir(argv[1]));jpeg("baseline.jpg",0);jpeg("progressive.jpg",1);png("alpha.png",1600,800);png("tiny.png",20,10);png("protected.png",20,10);gif();
    truncate_copy("baseline.jpg","truncated.jpg");truncate_copy("alpha.png","truncated.png");truncate_copy("animated.gif","truncated.gif");
    FILE *fake=fopen("fake.gif","wb");require(fake!=NULL);require(fputs("GIF89aTHIS_IS_NOT_AN_IMAGE",fake)>=0);require(!fclose(fake));
    require(!symlink("alpha.png","link.png"));require(!symlink("protected.png","link-output.png"));puts("IMAGE_FIXTURES=PASS");return 0;
}
