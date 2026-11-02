/* Add a valid private ancillary chunk containing every byte to the PNG fixture. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static void be(FILE*f,uint32_t n){for(int i=3;i>=0;i--)fputc((n>>(i*8))&255,f);}
int main(int argc,char**argv){if(argc!=3)return 1;FILE*in=fopen(argv[1],"rb");if(!in)return 1;unsigned char data[4096];size_t n=fread(data,1,sizeof(data),in);fclose(in);if(n<12||memcmp(data+n-8,"IEND",4))return 1;FILE*out=fopen(argv[2],"wb");if(!out)return 1;fwrite(data,1,n-12,out);be(out,256);unsigned char chunk[260];memcpy(chunk,"raNd",4);for(int i=0;i<256;i++)chunk[i+4]=i;fwrite(chunk,1,sizeof(chunk),out);uint32_t crc=~0u;for(size_t i=0;i<sizeof(chunk);i++){crc^=chunk[i];for(int bit=0;bit<8;bit++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}be(out,~crc);fwrite(data+n-12,1,12,out);return fclose(out)?1:0;}
