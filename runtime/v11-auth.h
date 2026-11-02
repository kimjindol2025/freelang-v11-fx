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
/* Node Buffer base64 accepts URL alphabet, omitted padding and ignored bytes. */
static size_t va_unbase64(const char*s,size_t n,unsigned char*out){
 unsigned bits=0,buffer=0;size_t used=0;
 for(size_t i=0;i<n;i++){unsigned char c=s[i];int v=-1;
  if(c=='=')break;
  if(c>='A'&&c<='Z')v=c-'A';else if(c>='a'&&c<='z')v=c-'a'+26;
  else if(c>='0'&&c<='9')v=c-'0'+52;else if(c=='+'||c=='-')v=62;else if(c=='/'||c=='_')v=63;
  if(v<0)continue;
  buffer=(buffer<<6)|(unsigned)v;bits+=6;
  if(bits>=8){bits-=8;out[used++]=(unsigned char)(buffer>>bits);}
 }return used;
}
static int va_scrypt_cost(FLValue value,uint64_t*out){
 TUnits text=t_units(value);double numbers[3]={NAN,NAN,NAN};
 for(size_t start=0;start<=text.n;){
  size_t end=start;while(end<text.n&&text.p[end]!=',')end++;
  size_t eq=start;while(eq<end&&text.p[eq]!='=')eq++;
  size_t left=start,right=eq;while(left<right&&t_space(text.p[left]))left++;while(right>left&&t_space(text.p[right-1]))right--;
  if(right==left+1){int index=text.p[left]=='N'?0:text.p[left]=='r'?1:text.p[left]=='p'?2:-1;
   if(index>=0){size_t last=eq<end?eq+1:end;while(last<end&&text.p[last]!='=')last++;
    numbers[index]=eq<end?t_double(t_parse(t_from_units(text.p+eq+1,last-eq-1),0)):NAN;}}
  if(end==text.n)break;start=end+1;
 }
 free(text.p);
 for(int i=0;i<3;i++){double d=numbers[i];if(!isfinite(d)||d<0||d>UINT32_MAX||floor(d)!=d)return 0;
  out[i]=d?(uint64_t)d:(i==0?16384:i==1?8:1);}
 if(out[0]<2||(out[0]&(out[0]-1)))return 0;
 /* Bound CPU as well as OpenSSL's 32 MiB memory budget. */
 return out[0]<=(1u<<24)&&out[1]<=(1u<<24)/out[0]&&out[2]<=(1u<<24)/out[0]/out[1];
}
static int va_nibble(unsigned char c){
 if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;
}
static FLValue va_verify_password(FLValue password,FLValue stored){
 if(stored.tag!=FL_STRING)return fl_bool(false);
 TBuffer wire=va_bytes(stored,"auth_verify_password");int valid=0;
 if(wire.n>VA_RANDOM_MAX){va_drop(&wire);return fl_bool(false);}
 if(wire.n>=8&&!memcmp(wire.p,"$scrypt$",8)){
  if(password.tag!=FL_STRING){va_drop(&wire);return fl_bool(false);}
  char*params=wire.p+8;size_t remaining=wire.n-8;char*salt=memchr(params,'$',remaining);
  char*key=salt?memchr(salt+1,'$',remaining-(size_t)(salt+1-params)):NULL;
  if(salt&&key&&!memchr(key+1,'$',wire.n-(size_t)(key+1-wire.p))){
   uint64_t cost[3];FLValue param=t_bytes(params,(size_t)(salt-params),0);
   if(va_scrypt_cost(param,cost)){
    size_t sn=(size_t)(key-salt-1),kn=wire.n-(size_t)(key+1-wire.p);
    unsigned char*sb=malloc(sn+1),*kb=malloc(kn+1),*computed=malloc(kn+1);
    if(sb&&kb&&computed){
     size_t sl=va_unbase64(salt+1,sn,sb),kl=va_unbase64(key+1,kn,kb);
     if(kl){TBuffer pw=va_bytes(password,"auth_verify_password");
      int ok=EVP_PBE_scrypt(pw.p?pw.p:"",pw.n,sb,sl,cost[0],cost[1],cost[2],32u<<20,computed,kl);
      valid=ok==1&&CRYPTO_memcmp(computed,kb,kl)==0;va_drop(&pw);}
     OPENSSL_cleanse(sb,sn+1);OPENSSL_cleanse(kb,kn+1);OPENSSL_cleanse(computed,kn+1);
    }free(sb);free(kb);free(computed);
   }
  }
 }else{
  char*colon=wire.n?memchr(wire.p,':',wire.n):NULL;
  if(colon){size_t hn=wire.n-(size_t)(colon+1-wire.p);char*extra=memchr(colon+1,':',hn);if(extra)hn=(size_t)(extra-colon-1);
   unsigned char expected[33],computed[32];size_t count=0;
   for(size_t i=0;i+1<hn&&count<33;i+=2){int a=va_nibble(colon[1+i]),b=va_nibble(colon[2+i]);if(a<0||b<0)break;expected[count++]=(a<<4)|b;}
   if(count==32){TBuffer pw=va_bytes(t_js_string(password),"auth_verify_password");EVP_MD_CTX*ctx=EVP_MD_CTX_new();unsigned length=0;
    if(ctx&&EVP_DigestInit_ex(ctx,EVP_sha256(),NULL)==1&&EVP_DigestUpdate(ctx,wire.p,(size_t)(colon-wire.p))==1&&EVP_DigestUpdate(ctx,pw.p?pw.p:"",pw.n)==1&&EVP_DigestFinal_ex(ctx,computed,&length)==1)
     valid=length==32&&CRYPTO_memcmp(expected,computed,32)==0;
    EVP_MD_CTX_free(ctx);va_drop(&pw);}
   OPENSSL_cleanse(expected,sizeof(expected));OPENSSL_cleanse(computed,sizeof(computed));
  }
 }
 va_drop(&wire);ERR_clear_error();return fl_bool(valid);
}
static FLValue va_auth(const char*op,FLValue argv){
 FLVector*av=(FLVector*)argv.obj;size_t n=av->len;FLValue a=n?av->data[0]:fl_nil(),b=n>1?av->data[1]:fl_nil();
 if(!strcmp(op,"auth-verify-password"))return va_verify_password(a,b);
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
