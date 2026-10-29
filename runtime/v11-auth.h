/* Native implementations of the app's v11 authentication contracts. */
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#define VA_RANDOM_MAX (1024u*1024u)
static FLValue va_error(const char*op,const char*type,const char*why){char message[192];snprintf(message,sizeof(message),"%s: %s",op,why);fl_throw(fl_make_error(type,message));return fl_nil();}
static void va_drop(TBuffer*b){if(b->p){OPENSSL_cleanse(b->p,b->n);free(b->p);}memset(b,0,sizeof(*b));}
static TBuffer va_bytes(FLValue value,const char*op){
 if(value.tag!=FL_STRING)va_error(op,"TypeError","string required");
 TUnits text=t_units(value);TBuffer out={0};t_encode(&out,text.p,text.n,1);free(text.p);return out;
}
static void va_hex(const unsigned char*bytes,size_t n,char*out){static const char*digits="0123456789abcdef";for(size_t i=0;i<n;i++){out[i*2]=digits[bytes[i]>>4];out[i*2+1]=digits[bytes[i]&15];}out[n*2]=0;}
static int64_t va_now(const char*op){struct timespec ts;if(clock_gettime(CLOCK_REALTIME,&ts))va_error(op,"RuntimeError","clock failed");return ts.tv_sec;}
static void va_mac(FLValue data,FLValue secret,unsigned char digest[32],const char*op){
 if(data.tag!=FL_STRING||secret.tag!=FL_STRING)va_error(op,"TypeError","string data and key required");
 TBuffer text=va_bytes(data,op),key=va_bytes(secret,op);unsigned n=0;
 if(key.n>INT_MAX){va_drop(&text);va_drop(&key);va_error(op,"RangeError","key too large");}
 unsigned char empty=0;unsigned char*result=HMAC(EVP_sha256(),key.p?(const void*)key.p:(const void*)&empty,(int)key.n,(const unsigned char*)(text.p?text.p:""),text.n,digest,&n);
 va_drop(&text);va_drop(&key);
 if(!result||n!=32){ERR_clear_error();va_error(op,"RuntimeError","HMAC failed");}
}
static double va_seconds(FLValue value){
 TUnits text=t_units(value);size_t i=0;while(i<text.n&&t_space(text.p[i]))i++;
 int negative=0;if(i<text.n&&(text.p[i]=='+'||text.p[i]=='-'))negative=text.p[i++]=='-';
 size_t first=i;double number=0;while(i<text.n&&text.p[i]>='0'&&text.p[i]<='9')number=number*10+text.p[i++]-'0';
 free(text.p);return i==first?NAN:negative?-number:number;
}
static FLValue va_auth(const char*op,FLValue argv){
 FLVector*av=(FLVector*)argv.obj;size_t n=av->len;FLValue a=n?av->data[0]:fl_nil(),b=n>1?av->data[1]:fl_nil();
 if(!strcmp(op,"auth-bearer-extract")){
  FLValue headers=fl_map_get(a,fl_str_val("headers"));FLValue header=fl_map_get(headers,fl_str_val("authorization"));
  if(header.tag==FL_NIL)header=fl_map_get(headers,fl_str_val("Authorization"));
  if(header.tag!=FL_STRING)return fl_nil();
  TUnits text=t_units(header);const char*prefix="Bearer ";int match=text.n>=7;
  for(size_t i=0;match&&i<7;i++)if(text.p[i]!=(unsigned char)prefix[i])match=0;
  FLValue out=match?t_from_units(text.p+7,text.n-7):fl_nil();free(text.p);return out;
 }
 if(!strcmp(op,"auth-password-needs-rehash")){
  if(a.tag!=FL_STRING)return va_error("auth_password_needs_rehash","TypeError","string required");
  FLString*s=(FLString*)a.obj;return fl_bool(s->len<8||memcmp(s->data,"$scrypt$",8));
 }
 if(!strcmp(op,"auth-random-token")){
  double count=n?t_numeric(a)?t_double(a):NAN:32;
  if(n&&!t_numeric(a))return va_error("auth_random_token","TypeError","numeric byte count required");
  if(!isfinite(count)||count<0||count>VA_RANDOM_MAX)return va_error("auth_random_token","RangeError","byte count outside native resource limit");
  size_t size=(size_t)floor(count);if(!size)return fl_str_val("");
  unsigned char*bytes=malloc(size);char*hex=malloc(size*2+1);
  if(!bytes||!hex){free(bytes);free(hex);return va_error("auth_random_token","RuntimeError","allocation failed");}
  if(RAND_bytes(bytes,(int)size)!=1){OPENSSL_cleanse(bytes,size);free(bytes);free(hex);ERR_clear_error();return va_error("auth_random_token","RuntimeError","secure random failed");}
  va_hex(bytes,size,hex);OPENSSL_cleanse(bytes,size);free(bytes);FLValue out=fl_str_val(hex);free(hex);return out;
 }
 if(!strcmp(op,"auth-hash-password")){
  TBuffer password=va_bytes(a,"auth_hash_password");unsigned char salt[16],key[64];char salt64[25],key64[89],out[192];
  if(RAND_bytes(salt,sizeof(salt))!=1){va_drop(&password);ERR_clear_error();return va_error("auth_hash_password","RuntimeError","secure random failed");}
  int ok=EVP_PBE_scrypt(password.p?password.p:"",password.n,salt,sizeof(salt),16384,8,1,32u<<20,key,sizeof(key));va_drop(&password);
  if(ok!=1){OPENSSL_cleanse(key,sizeof(key));ERR_clear_error();return va_error("auth_hash_password","RuntimeError","scrypt failed");}
  EVP_EncodeBlock((unsigned char*)salt64,salt,sizeof(salt));EVP_EncodeBlock((unsigned char*)key64,key,sizeof(key));OPENSSL_cleanse(key,sizeof(key));
  snprintf(out,sizeof(out),"$scrypt$N=16384,r=8,p=1$%s$%s",salt64,key64);OPENSSL_cleanse(key64,sizeof(key64));return fl_str_val(out);
 }
 if(!strcmp(op,"auth-csrf-token")){
  char ts[32],signature[17],token[64];unsigned char digest[32];snprintf(ts,sizeof(ts),"%lld",(long long)va_now("auth_csrf_token"));
  va_mac(fl_str_val(ts),a,digest,"auth_csrf_token");va_hex(digest,8,signature);OPENSSL_cleanse(digest,sizeof(digest));snprintf(token,sizeof(token),"%s.%s",ts,signature);return fl_str_val(token);
 }
 if(!strcmp(op,"auth-csrf-verify")){
  if(!fl_v11_truthy(a))return fl_bool(false);
  if(a.tag!=FL_STRING)return va_error("auth_csrf_verify","TypeError","string token required");
  TUnits text=t_units(a);size_t split=SIZE_MAX;
  for(size_t i=0;i<text.n;i++)if(text.p[i]=='.'){if(split!=SIZE_MAX){free(text.p);return fl_bool(false);}split=i;}
  if(split==SIZE_MAX){free(text.p);return fl_bool(false);}
  FLValue timestamp=t_from_units(text.p,split),signature=t_from_units(text.p+split+1,text.n-split-1);size_t units=text.n-split-1;free(text.p);
  double age=(double)va_now("auth_csrf_verify")-va_seconds(timestamp);if(isnan(age)||age<0||age>3600)return fl_bool(false);
  unsigned char digest[32];char expected[17];va_mac(timestamp,b,digest,"auth_csrf_verify");va_hex(digest,8,expected);OPENSSL_cleanse(digest,sizeof(digest));
  if(units!=16)return fl_bool(false);
  TBuffer actual=va_bytes(signature,"auth_csrf_verify");if(actual.n!=16){va_drop(&actual);return va_error("auth_csrf_verify","RangeError","Input buffers must have the same byte length");}
  int equal=CRYPTO_memcmp(actual.p,expected,16)==0;va_drop(&actual);return fl_bool(equal);
 }
 return va_error("auth","RuntimeError","unknown authentication operation");
}
