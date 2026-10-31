/* Deterministic visible test pattern, not product image processing. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void be32(unsigned char *p,uint32_t n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static void chunk(FILE*f,const char*type,const unsigned char*data,size_t n){
 unsigned char size[4];be32(size,(uint32_t)n);if(fwrite(size,1,4,f)!=4||fwrite(type,1,4,f)!=4)exit(1);
 uint32_t crc=~0u;
 for(size_t i=0;i<n+4;i++){unsigned char byte=i<4?(unsigned char)type[i]:data[i-4];crc^=byte;for(int k=0;k<8;k++)crc=(crc>>1)^(0xedb88320u&-(crc&1u));}
 if(n&&fwrite(data,1,n,f)!=n)exit(1);be32(size,~crc);if(fwrite(size,1,4,f)!=4)exit(1);
}
int main(int argc,char**argv){
 if(argc!=2)return 2;enum{width=320,height=180,row=width*3+1,length=row*height};
 unsigned char *raw=malloc(length),*z=malloc(length+32);if(!raw||!z)return 1;
 for(int y=0;y<height;y++){
  raw[y*row]=0;
  for(int x=0;x<width;x++){
   unsigned char*p=raw+y*row+1+x*3;
   int sky=y<95,land=y>=95+((x/30)%3)*8;
   p[0]=land?40:sky?130:85;p[1]=land?110:sky?195:150;p[2]=land?85:sky?220:155;
   if((x-250)*(x-250)+(y-38)*(y-38)<18*18){p[0]=249;p[1]=218;p[2]=135;}
  }
 }
 size_t at=0,offset=0;z[at++]=0x78;z[at++]=1;
 while(offset<length){size_t n=length-offset;if(n>65535)n=65535;z[at++]=(offset+n==length)?1:0;
  z[at++]=n;z[at++]=n>>8;z[at++]=~n;z[at++]=(~n)>>8;memcpy(z+at,raw+offset,n);at+=n;offset+=n;}
 uint32_t a=1,b=0;for(size_t i=0;i<length;i++){a=(a+raw[i])%65521;b=(b+a)%65521;}be32(z+at,(b<<16)|a);at+=4;
 FILE*f=fopen(argv[1],"wb");if(!f)return 1;unsigned char signature[]={137,80,78,71,13,10,26,10};if(fwrite(signature,1,8,f)!=8)return 1;
 unsigned char ihdr[13]={0};be32(ihdr,width);be32(ihdr+4,height);ihdr[8]=8;ihdr[9]=2;
 chunk(f,"IHDR",ihdr,13);chunk(f,"IDAT",z,at);chunk(f,"IEND",NULL,0);
 int rc=fclose(f);free(raw);free(z);return rc?1:0;
}
