/* Independent wire-format oracles. RFC 7914 keys are public fixed test vectors. */
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char*base;
static void fail(void){fputs("AUTH_FIXTURE_ERROR\n",stderr);exit(1);}
static FILE*file(const char*name,const char*mode){char path[4096];if(snprintf(path,sizeof(path),"%s/%s",base,name)>=(int)sizeof(path))fail();FILE*f=fopen(path,mode);if(!f)fail();return f;}
static void hex(const unsigned char*p,size_t n,char*out){for(size_t i=0;i<n;i++)sprintf(out+i*2,"%02x",p[i]);}
static unsigned char nibble(char c){return c>='0'&&c<='9'?c-'0':c-'a'+10;}
static void vector(FILE*f,const char*password,const char*salt,const char*params,const char*key,int valid,int comma){
 unsigned char bytes[64];char sb[256],kb[100];for(int i=0;i<64;i++)bytes[i]=(nibble(key[i*2])<<4)|nibble(key[i*2+1]);
 EVP_EncodeBlock((unsigned char*)sb,(const unsigned char*)salt,strlen(salt));EVP_EncodeBlock((unsigned char*)kb,bytes,64);
 fprintf(f,"%s{\"password\":\"%s\",\"stored\":\"$scrypt$%s$%s$%s\",\"valid\":%s}",comma?",":"",password,params,sb,kb,valid?"true":"false");
}
static void token(FILE*f,const char*name,const char*ts,const char*key,int comma){unsigned char digest[32];unsigned n=0;char h[65];if(!HMAC(EVP_sha256(),key,strlen(key),(const unsigned char*)ts,strlen(ts),digest,&n)||n!=32)fail();hex(digest,32,h);fprintf(f,"%s\"%s\":\"%s.%.16s\"",comma?",":"",name,ts,h);}
static void create(void){
 const char*def="7023bdcb3afd7348461c06cd81fd38ebfda8fbba904f8e3ea9b543f6545da1f2d5432955613f0fcf62d49705242a9af9e61e85dc0d651e40dfcf017b45575887";
 FILE*f=file("password-fixtures.json","wb");fputs("[",f);
 vector(f,"","","N=16,r=1,p=1","77d6576238657b203b19ca42c18a0497f16b4844e3074ae8dfdffa3fede21442fcd0069ded0948f8326a753a0fc81f17e8d3e0fb2e0d3628cf35e20c38d18906",1,0);
 vector(f,"password","NaCl","N=1024,r=8,p=16","fdbabe1c9d3472007856e7190d01e9fe7c6ad7cbc8237830e77376634b3731622eaf30d92e22a3886ff109279d9830dac727afb94a83ee6d8360cbdfa2cc0640",1,1);
 vector(f,"pleaseletmein","SodiumChloride","N=16384,r=8,p=1",def,1,1);
 vector(f,"pleaseletmein","SodiumChloride","p=1, r=8, N=16384",def,1,1);
 vector(f,"pleaseletmein","SodiumChloride","N=0,r=0,p=0",def,1,1);
 vector(f,"wrong","SodiumChloride","N=16384,r=8,p=1",def,0,1);
 vector(f,"pleaseletmein","SodiumChloride","N=3,r=8,p=1",def,0,1);
 vector(f,"pleaseletmein","SodiumChloride","N=1048576,r=8,p=1",def,0,1);
 unsigned char digest[32];unsigned n=0;char encoded[65];const char*legacy="0123456789abcdefpublic-legacy-fixture";
 if(!EVP_Digest(legacy,strlen(legacy),digest,&n,EVP_sha256(),NULL)||n!=32)fail();hex(digest,32,encoded);
 fprintf(f,",{\"password\":\"public-legacy-fixture\",\"stored\":\"0123456789abcdef:%s\",\"valid\":true}",encoded);
 fprintf(f,",{\"password\":\"wrong\",\"stored\":\"0123456789abcdef:%s\",\"valid\":false}",encoded);
 fputs(",{\"password\":\"x\",\"stored\":\"$scrypt$broken\",\"valid\":false},{\"password\":\"x\",\"stored\":\"bad\",\"valid\":false}]",f);fclose(f);
 unsigned char random[32];char key[65];if(RAND_bytes(random,sizeof(random))!=1)fail();hex(random,32,key);
 f=file("csrf.key","wb");fwrite(key,1,64,f);fclose(f);
 f=file("csrf.json","wb");fputs("{",f);token(f,"now","1800000000",key,0);token(f,"boundary","1799996400",key,1);token(f,"expired","1799996399",key,1);token(f,"future","1800000001",key,1);token(f,"padded"," +1800000000tail",key,1);token(f,"invalid","no-time",key,1);fputs("}",f);fclose(f);
 f=file("password.txt","wb");fwrite("한😀",1,7,f);fputc(0,f);fwrite("끝",1,3,f);fclose(f);OPENSSL_cleanse(random,sizeof(random));OPENSSL_cleanse(key,sizeof(key));
 puts("AUTH_SYNTHETIC_FIXTURES=READY");
}
static size_t read_file(const char*name,unsigned char*out,size_t max){FILE*f=file(name,"rb");size_t n=fread(out,1,max,f);if(ferror(f)||!feof(f))fail();fclose(f);return n;}
static int decode(const char*s,unsigned char*out){size_t n=strlen(s);int r=EVP_DecodeBlock(out,(const unsigned char*)s,n);if(r<0)return-1;while(n&&s[--n]=='=')r--;return r;}
static void validate(void){
 unsigned char stored[1024],password[1024];size_t n=read_file("created.phc",stored,sizeof(stored)-1),pn=read_file("password.txt",password,sizeof(password));stored[n]=0;
 const char*prefix="$scrypt$N=16384,r=8,p=1$";if(n!=137||strncmp((char*)stored,prefix,24))fail();
 char*salt=(char*)stored+24,*split=strchr(salt,'$');if(!split)fail();*split++=0;
 unsigned char sb[128],kb[128],computed[64];if(decode(salt,sb)!=16||decode(split,kb)!=64)fail();
 if(EVP_PBE_scrypt((const char*)password,pn,sb,16,16384,8,1,32u<<20,computed,64)!=1||CRYPTO_memcmp(computed,kb,64))fail();
 OPENSSL_cleanse(password,sizeof(password));OPENSSL_cleanse(computed,sizeof(computed));puts("PASSWORD_CREATE_FORMAT_AND_KDF=PASS");
}
int main(int argc,char**argv){if(argc!=3)return 1;base=argv[2];if(!strcmp(argv[1],"create"))create();else if(!strcmp(argv[1],"validate"))validate();else return 1;return 0;}
