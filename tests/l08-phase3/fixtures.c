#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static FILE *output(const char *base,const char *name){char path[4096];snprintf(path,sizeof(path),"%s/%s",base,name);FILE*f=fopen(path,"wb");if(!f)exit(1);return f;}
static void b64(FILE*f,const char*s){const char*abc="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";unsigned bits=0,value=0;for(;*s&&*s!='=';s++){const char*p=strchr(abc,*s);if(!p)exit(1);value=(value<<6)|(unsigned)(p-abc);bits+=6;if(bits>=8){bits-=8;fputc((value>>bits)&255,f);}}fclose(f);}
static void bytes(FILE*f,const unsigned char*p,size_t n){if(fwrite(p,1,n,f)!=n)exit(1);}
int main(int argc,char**argv){if(argc!=2)return 1;
b64(output(argv[1],"image.png"),"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+aBVkAAAAASUVORK5CYII=");
b64(output(argv[1],"image.gif"),"R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7");
/* One-pixel baseline grayscale JPEG: zero DC and EOB codes, padded with ones. */
FILE*f=output(argv[1],"image.jpg");unsigned char soi[]={255,216,255,219,0,67,0};bytes(f,soi,sizeof(soi));for(int i=0;i<64;i++)fputc(1,f);
unsigned char sof[]={255,192,0,11,8,0,1,0,1,1,1,17,0};bytes(f,sof,sizeof(sof));
for(int table=0;table<2;table++){unsigned char dht[]={255,196,0,20};bytes(f,dht,4);fputc(table?16:0,f);fputc(1,f);for(int i=1;i<16;i++)fputc(0,f);fputc(0,f);}
unsigned char scan[]={255,218,0,8,1,1,0,0,63,0,63,255,217};bytes(f,scan,sizeof(scan));fclose(f);
f=output(argv[1],"allbytes");for(int i=0;i<256;i++)fputc(i,f);fclose(f);
f=output(argv[1],"invalid");unsigned char invalid[]={0xe1,0x80,'X',0,0xff};bytes(f,invalid,sizeof(invalid));fclose(f);
return 0;}
