/* v11 ordinary-interpreter contracts. Included once by core.c. */
#include <limits.h>
#include <float.h>
#include <errno.h>
#include <openssl/rand.h>
#include "ryu/d2s.c"
#include "unicode15.h"
#include "v11-ops.h"
#define NEL(a) (sizeof(a)/sizeof((a)[0]))
typedef struct {
     uint16_t *p;
     size_t n,cap;

} TUnits;
typedef struct {
     char *p;
     size_t n,cap;

} TBuffer;
static void *t_resize(void *p,size_t n) {
    void *r=realloc(p,n?n:1);
    if(!r)fl_throw(fl_str_val("text allocation failed"));
    return r;
}

static void t_unit(TUnits *u,uint16_t c){
    if(u->n==u->cap){
        u->cap=u->cap?u->cap*2:32;
        u->p=t_resize(u->p,u->cap*sizeof(*u->p));
    }
    u->p[u->n++]=c;
}

static void t_put(TBuffer *b,const char *p,size_t n){
    if(n>SIZE_MAX-b->n-1)fl_throw(fl_str_val("text too large"));
    if(b->n+n+1>b->cap){
        b->cap=(b->n+n+1)*2;
        b->p=t_resize(b->p,b->cap);
    }
    memcpy(b->p+b->n,p,n);
    b->n+=n;
    b->p[b->n]=0;
}

static void t_ascii(TBuffer *b,const char *p){
    t_put(b,p,strlen(p));
}

static void t_cp(TBuffer *b,uint32_t c){
    char p[4];
    int n;
    if(c<128){
        p[0]=c;
        n=1;
    }
    else if(c<2048){
        p[0]=0xc0|(c>>6);
        p[1]=0x80|(c&63);
        n=2;
    }
    else if(c<65536){
        p[0]=0xe0|(c>>12);
        p[1]=0x80|((c>>6)&63);
        p[2]=0x80|(c&63);
        n=3;
    }
    else{
        p[0]=0xf0|(c>>18);
        p[1]=0x80|((c>>12)&63);
        p[2]=0x80|((c>>6)&63);
        p[3]=0x80|(c&63);
        n=4;
    }
    t_put(b,p,n);
}

static FLValue t_bytes(const char *p,size_t n,int text){
    if(n>UINT32_MAX)fl_throw(fl_str_val("text too large"));
    FLString*s=fl_arena_alloc(sizeof(*s)+n+1);
    if(!s)fl_throw(fl_str_val("text allocation failed"));
    s->base.type=text?FL_TEXT:FL_STRING;
    s->base.rc=1;
    s->len=n;
    memcpy(s->data,p,n);
    s->data[n]=0;
    return(FLValue){
        .tag=FL_STRING,.obj=(FLObject*)s
    };
}

/* Raw UTF-8 replaces malformed subsequences; branded text retains lone UTF-16 units. */
static TUnits t_units(FLValue v){
    TUnits u={0};
    if(v.tag!=FL_STRING)return u;
    FLString*s=(FLString*)v.obj;
    const unsigned char*p=(const unsigned char*)s->data;
    size_t i=0;
    while(i<s->len){
        uint32_t c=p[i];
        size_t k=1,want=1;
        if(c>=0xc2&&c<=0xf4){
            want=c<0xe0?2:c<0xf0?3:4;
            uint32_t acc=c&((1u<<(7-want))-1);
            while(k<want&&i+k<s->len){
                unsigned x=p[i+k];
                if((x&0xc0)!=0x80)break;
                if(k==1&&((c==0xe0&&x<0xa0)||(c==0xed&&s->base.type!=FL_TEXT&&x>=0xa0)||(c==0xf0&&x<0x90)||(c==0xf4&&x>=0x90)))break;
                acc=(acc<<6)|(x&63);
                k++;
            }
            c=k==want?acc:0xfffd;
        }
        else if(c>=0x80)c=0xfffd;
        i+=k;
        if(c>0xffff){
            t_unit(&u,0xd800+((c-0x10000)>>10));
            t_unit(&u,0xdc00+((c-0x10000)&1023));
        }
        else t_unit(&u,c);
    }
    return u;
}

static void t_encode(TBuffer*b,const uint16_t*p,size_t n,int replace){
    for(size_t i=0;i<n;i++){
        uint32_t c=p[i];
        if(c>=0xd800&&c<=0xdbff&&i+1<n&&p[i+1]>=0xdc00&&p[i+1]<=0xdfff)c=0x10000+((c-0xd800)<<10)+(p[++i]-0xdc00);
        else if(replace&&c>=0xd800&&c<=0xdfff)c=0xfffd;
        t_cp(b,c);
    }
}

static FLValue t_from_units(const uint16_t*p,size_t n){
    TBuffer b={0};
    t_encode(&b,p,n,0);
    FLValue r=t_bytes(b.p?b.p:"",b.n,1);
    free(b.p);
    return r;
}

static FLValue t_finish(TBuffer*b){
    FLValue r=t_bytes(b->p?b->p:"",b->n,1);
    free(b->p);
    b->p=NULL;
    return r;
}

static FLValue t_text(FLValue v){
    if(v.tag==FL_STRING){
        FLString*s=(FLString*)v.obj;
        int convert=0;
        if(s->base.type==FL_TEXT){
            for(size_t i=0;i+5<s->len;i++)if((unsigned char)s->data[i]==0xed&&(unsigned char)s->data[i+1]>=0xa0&&(unsigned char)s->data[i+1]<=0xaf&&(unsigned char)s->data[i+3]==0xed&&(unsigned char)s->data[i+4]>=0xb0&&(unsigned char)s->data[i+4]<=0xbf){
                convert=1;
                break;
            }
        }
        else for(size_t i=0;i<s->len;i++)if((unsigned char)s->data[i]>=128){
            convert=1;
            break;
        }
        if(!convert)return v;
    }
    TUnits u=t_units(v);
    FLValue r=t_from_units(u.p,u.n);
    free(u.p);
    return r;
}

/* Ryu supplies round-trip digits; v11 uses fixed notation for exponents [-6,21). */
FLValue fl_v11_number(FLValue v){
    double x=v.tag==FL_INT?(double)v.i:v.f;
    if(isnan(x))return fl_str_val("NaN");
    if(isinf(x))return fl_str_val(signbit(x)?"-Infinity":"Infinity");
    if(x==0)return fl_str_val("0");
    char raw[32],digits[32];
    int n=d2s_buffered_n(fabs(x),raw);
    raw[n]=0;
    char*e=strchr(raw,'E');
    int exponent=e?atoi(e+1):0;
    size_t dn=0;
    for(char*p=raw;p!=(e?e:raw+n);p++)if(*p!='.')digits[dn++]=*p;
    TBuffer b={0};
    if(signbit(x))t_ascii(&b,"-");
    int point=exponent+1;
    if(exponent>=21||exponent< -6){
        t_put(&b,digits,1);
        if(dn>1){
            t_ascii(&b,".");
            t_put(&b,digits+1,dn-1);
        }
        char exp[16];
        snprintf(exp,sizeof(exp),"e%s%d",exponent>=0?"+":"",exponent);
        t_ascii(&b,exp);
    }
    else if(point<=0){
        t_ascii(&b,"0.");
        for(int i=0;i< -point;i++)t_ascii(&b,"0");
        t_put(&b,digits,dn);
    }
    else{
        for(int i=0;i<point;i++)t_put(&b,(size_t)i<dn?digits+i:"0",1);
        if((size_t)point<dn){
            t_ascii(&b,".");
            t_put(&b,digits+point,dn-point);
        }
    }
    return t_finish(&b);
}

static double t_double(FLValue v){
    return v.tag==FL_INT?(double)v.i:v.f;
}

static int t_numeric(FLValue v){
    return v.tag==FL_INT||v.tag==FL_FLOAT;
}

static FLValue t_key(FLValue);
static FLValue t_parse(FLValue,int);
static FLValue t_js_string(FLValue);
static void t_render(TBuffer*,FLValue,int,int);
static uint32_t t_array_index(FLValue key){
    if(key.tag!=FL_STRING)return UINT32_MAX;
    FLString*s=(FLString*)key.obj;
    if(!s->len||(s->len>1&&s->data[0]=='0'))return UINT32_MAX;
    uint64_t n=0;
    for(uint32_t i=0;i<s->len;i++){
        if(s->data[i]<'0'||s->data[i]>'9')return UINT32_MAX;
        n=n*10+s->data[i]-'0';
        if(n>=UINT32_MAX)return UINT32_MAX;
    }
    return n;
}

static FLValue t_keys(FLValue v){
    FLValue r=fl_vec_new();
    if(v.tag!=FL_MAP)return r;
    FLMap*m=(FLMap*)v.obj;
    for(uint32_t i=0;i<m->len;i++)r=fl_vec_push(r,m->entries[i].key);
    FLVector*a=(FLVector*)r.obj;
    for(uint32_t i=1;i<a->len;i++){
        FLValue k=a->data[i];
        uint32_t num=t_array_index(k),j=i;
        if(num==UINT32_MAX)continue;
        while(j&&t_array_index(a->data[j-1])>num){
            a->data[j]=a->data[j-1];
            j--;
        }
        a->data[j]=k;
    }
    return r;
}

static void t_quoted(TBuffer*b,FLValue s,int json){
    TUnits u=t_units(s);
    t_ascii(b,"\"");
    for(size_t i=0;i<u.n;i++){
        uint32_t c=u.p[i];
        if(json&&(c=='"'||c=='\\')){
            char p[2]={
                '\\',c
            };
            t_put(b,p,2);
        }
        else if(json&&(c==8||c==9||c==10||c==12||c==13)){
            char p[2]={
                '\\',c==8?'b':c==9?'t':c==10?'n':c==12?'f':'r'
            };
            t_put(b,p,2);
        }
        else if(json&&c<32){
            char p[7];
            snprintf(p,sizeof(p),"\\u%04x",c);
            t_ascii(b,p);
        }
        else if(c>=0xd800&&c<=0xdbff&&i+1<u.n&&u.p[i+1]>=0xdc00&&u.p[i+1]<=0xdfff)t_cp(b,0x10000+((c-0xd800)<<10)+(u.p[++i]-0xdc00));
        else if(c>=0xd800&&c<=0xdfff){
            if(json){
                char p[7];
                snprintf(p,sizeof(p),"\\u%04x",c);
                t_ascii(b,p);
            }
            else t_cp(b,0xfffd);
        }
        else t_cp(b,c);
    }
    t_ascii(b,"\"");
    free(u.p);
}

static void t_render(TBuffer*b,FLValue v,int json,int inner){
    if(t_numeric(v)){
        FLValue s=fl_v11_number(v);
        if(json&&!isfinite(t_double(v)))t_ascii(b,"null");
        else t_ascii(b,((FLString*)s.obj)->data);
        return;
    }
    switch(v.tag){
        case FL_NIL:t_ascii(b,"null");
        break;
        case FL_BOOL:t_ascii(b,v.b?"true":"false");
        break;
        case FL_STRING:if(json||inner)t_quoted(b,v,json);
        else{
            TUnits u=t_units(v);
            t_encode(b,u.p,u.n,1);
            free(u.p);
        }
        break;
        case FL_VECTOR:{
            FLVector*a=(FLVector*)v.obj;
            t_ascii(b,"[");
            for(uint32_t i=0;i<a->len;i++){
                if(i)t_ascii(b,json?",":", ");
                t_render(b,a->data[i],json,1);
            }
            t_ascii(b,"]");
            break;
        }
        case FL_MAP:{
            FLValue keys=t_keys(v);
            FLVector*a=(FLVector*)keys.obj;
            t_ascii(b,"{");
            int written=0;
            for(uint32_t i=0;i<a->len;i++){
                FLValue k=a->data[i];
                if(!json&&k.tag==FL_STRING&&!strncmp(((FLString*)k.obj)->data,"__",2))continue;
                if(written++)t_ascii(b,json?",":", ");
                t_quoted(b,k,json);
                t_ascii(b,json?":":": ");
                t_render(b,fl_map_get(v,k),json,1);
            }
            t_ascii(b,"}");
            break;
        }
        case FL_FN:t_ascii(b,"<fn:λ>");
        break;
        default:break;
    }
}

static FLValue t_js_string(FLValue v){
    if(v.tag==FL_STRING)return t_text(v);
    if(t_numeric(v))return fl_v11_number(v);
    if(v.tag==FL_NIL)return fl_str_val("null");
    if(v.tag==FL_BOOL)return fl_str_val(v.b?"true":"false");
    if(v.tag==FL_MAP)return fl_str_val("[object Object]");
    if(v.tag==FL_VECTOR){
        TBuffer b={0};
        FLVector*a=(FLVector*)v.obj;
        for(uint32_t i=0;i<a->len;i++){
            if(i)t_ascii(&b,",");
            if(a->data[i].tag!=FL_NIL){
                FLValue s=t_js_string(a->data[i]);
                t_put(&b,((FLString*)s.obj)->data,((FLString*)s.obj)->len);
            }
        }
        return t_finish(&b);
    }
    return fl_str_val("<fn:λ>");
}

static FLValue t_key(FLValue v){
    return t_js_string(v);
}

FLValue fl_v11_key(FLValue v){
    return t_key(v);
}

FLValue fl_v11_literal(FLValue v){
    if(v.tag==FL_STRING){
        v=t_parse(v,0);
        if(isnan(t_double(v)))fl_throw(fl_str_val("invalid numeric literal"));
    }
    double d=t_double(v);
    char b[128];
    if(isnan(d))return fl_str_val("fl_float(0.0/0.0)");
    if(isinf(d))return fl_str_val(d<0?"fl_float(-1.0/0.0)":"fl_float(1.0/0.0)");
    if(d==0&&signbit(d))return fl_str_val("fl_float(-0.0)");
    if(floor(d)==d&&fabs(d)<=9007199254740991.0)snprintf(b,sizeof(b),"fl_int(%lld)",(long long)d);
    else{
        FLValue s=fl_v11_number(v);
        char*p=((FLString*)s.obj)->data;
        snprintf(b,sizeof(b),"fl_float(%s%s)",p,strchr(p,'.')||strchr(p,'e')?"":".0");
    }
    return fl_str_val(b);
}

bool fl_v11_truthy(FLValue v){
    if(v.tag==FL_NIL)return false;
    if(v.tag==FL_BOOL)return v.b;
    if(t_numeric(v))return t_double(v)!=0&&!isnan(t_double(v));
    if(v.tag==FL_STRING)return ((FLString*)v.obj)->len!=0;
    return true;
}

bool fl_v11_or_false(FLValue v){
    return v.tag==FL_NIL||(v.tag==FL_BOOL&&!v.b);
}

static int t_str_compare(FLValue a,FLValue b){
    TUnits x=t_units(a),y=t_units(b);
    size_t n=x.n<y.n?x.n:y.n;
    int c=0;
    for(size_t i=0;i<n;i++)if(x.p[i]!=y.p[i]){
        c=x.p[i]<y.p[i]?-1:1;
        break;
    }
    if(!c)c=x.n<y.n?-1:x.n>y.n;
    free(x.p);
    free(y.p);
    return c;
}

static int t_equal(FLValue a,FLValue b,int deep){
    if(t_numeric(a)&&t_numeric(b))return t_double(a)==t_double(b);
    if(a.tag!=b.tag)return 0;
    if(a.tag==FL_STRING)return t_str_compare(a,b)==0;
    if(a.tag==FL_NIL)return 1;
    if(a.tag==FL_BOOL)return a.b==b.b;
    if(a.obj==b.obj)return 1;
    if(!deep)return 0;
    if(a.tag==FL_VECTOR){
        FLVector*x=(FLVector*)a.obj,*y=(FLVector*)b.obj;
        if(x->len!=y->len)return 0;
        for(uint32_t i=0;i<x->len;i++)if(!t_equal(x->data[i],y->data[i],1))return 0;
        return 1;
    }
    if(a.tag==FL_MAP){
        FLMap*x=(FLMap*)a.obj,*y=(FLMap*)b.obj;
        if(x->len!=y->len)return 0;
        for(uint32_t i=0;i<x->len;i++)if(!fl_truthy(fl_map_has(b,x->entries[i].key))||!t_equal(x->entries[i].val,fl_map_get(b,x->entries[i].key),1))return 0;
        return 1;
    }
    return 0;
}

static int t_space(uint16_t c){
    return(c>=9&&c<=13)||c==32||c==160||c==5760||(c>=8192&&c<=8202)||c==8232||c==8233||c==8239||c==8287||c==12288||c==0xfeff;
}

static FLValue t_parse(FLValue value,int prefix){
    if(!prefix){
        if(t_numeric(value))return value;
        if(value.tag==FL_NIL)return fl_float(0);
        if(value.tag==FL_BOOL)return fl_float(value.b?1:0);
    }
    FLValue s=t_js_string(value);
    TUnits u=t_units(s);
    size_t lo=0,hi=u.n;
    while(lo<hi&&t_space(u.p[lo]))lo++;
    while(hi>lo&&t_space(u.p[hi-1]))hi--;
    TBuffer b={0};
    t_encode(&b,u.p+lo,hi-lo,0);
    free(u.p);
    const char*p=b.p?b.p:"";
    if(!prefix&&!strcmp(p,"")){
        free(b.p);
        return fl_float(0);
    }
    size_t i=0;
    if(p[i]=='+'||p[i]=='-')i++;
    size_t sign=i;
    if(!strncmp(p+i,"Infinity",8)){
        double d=p[0]=='-'?-INFINITY:INFINITY;
        if(!prefix&&p[i+8])d=NAN;
        free(b.p);
        return fl_float(d);
    }
    if(!prefix&&p[0]=='0'&&(p[1]=='x'||p[1]=='X'||p[1]=='b'||p[1]=='B'||p[1]=='o'||p[1]=='O')){
        int base=p[1]=='x'||p[1]=='X'?16:p[1]=='b'||p[1]=='B'?2:8;
        char*end;
        double d=(double)strtoull(p+2,&end,base);
        if(end==p+2||*end)d=NAN;
        free(b.p);
        return fl_float(d);
    }
    int digits=0;
    while(p[i]>='0'&&p[i]<='9'){
        i++;
        digits++;
    }
    if(p[i]=='.'){
        i++;
        while(p[i]>='0'&&p[i]<='9'){
            i++;
            digits++;
        }
    }
    if(!digits){
        free(b.p);
        return prefix?fl_nil():fl_float(NAN);
    }
    if(p[i]=='e'||p[i]=='E'){
        size_t e=i++;
        if(p[i]=='+'||p[i]=='-')i++;
        size_t start=i;
        while(p[i]>='0'&&p[i]<='9')i++;
        if(start==i)i=e;
    }
    if((!prefix&&p[i])||i==sign){
        free(b.p);
        return prefix?fl_nil():fl_float(NAN);
    }
    b.p[i]=0;
    double d=strtod(b.p,NULL);
    free(b.p);
    return fl_float(d);
}

static int64_t t_index(FLValue v,int64_t length){
    double d=t_numeric(v)?t_double(v):t_double(t_parse(v,0));
    if(isnan(d))d=0;
    d=trunc(d);
    if(d<=-((double)length))return 0;
    if(d<0)return length+(int64_t)d;
    if(d>=length)return length;
    return(int64_t)d;
}

FLValue fl_v11_text_get(FLValue obj,FLValue key,FLValue fallback){
    if(!t_numeric(key)||!isfinite(t_double(key))||floor(t_double(key))!=t_double(key)||t_double(key)<0)return fallback;
    TUnits u=t_units(obj);
    double ix=t_double(key);
    FLValue r=ix<u.n?t_from_units(u.p+(size_t)ix,1):fallback;
    free(u.p);
    return r;
}

static FLValue t_slice(FLValue v,FLValue start,FLValue end,int has_end){
    size_t n;
    if(v.tag==FL_VECTOR)n=((FLVector*)v.obj)->len;
    else if(v.tag==FL_STRING){
        TUnits u=t_units(v);
        int64_t a=t_index(start,u.n),b=has_end?t_index(end,u.n):u.n;
        if(b<a)b=a;
        FLValue r=t_from_units(u.p+a,b-a);
        free(u.p);
        return r;
    }
    else return fl_vec_new();
    int64_t a=t_index(start,n),b=has_end?t_index(end,n):n;
    if(b<a)b=a;
    return fl_vec_from(((FLVector*)v.obj)->data+a,b-a);
}

static int64_t t_find_units(TUnits a,TUnits b,size_t start){
    if(start>a.n)start=a.n;
    if(!b.n)return start;
    for(size_t i=start;i+b.n<=a.n;i++)if(!memcmp(a.p+i,b.p,b.n*sizeof(uint16_t)))return i;
    return-1;
}

static uint32_t t_case_simple(uint32_t cp,int upper){
    const UMap*a=upper?unicode_Uc:unicode_Lc;
    size_t lo=0,hi=upper?NEL(unicode_Uc):NEL(unicode_Lc);
    while(lo<hi){
        size_t m=(lo+hi)/2;
        if(cp<a[m].first)hi=m;
        else if(cp>a[m].last)lo=m+1;
        else return a[m].to+cp-a[m].first;
    }
    return cp;
}

static int t_property(uint32_t cp,const uint32_t*a,size_t n){
    size_t lo=0,hi=n;
    while(lo<hi){
        size_t m=(lo+hi)/2;
        if(a[m]<=cp)lo=m+1;
        else hi=m;
    }
    return lo%2;
}

static FLValue t_case(FLValue value,int upper){
    if(value.tag!=FL_STRING)return value.tag==FL_NIL?fl_str_val(""):value;
    TUnits u=t_units(value);
    uint32_t*cp=malloc((u.n+1)*sizeof(*cp));
    if(!cp)fl_throw(fl_str_val("text allocation failed"));
    size_t n=0;
    for(size_t i=0;i<u.n;i++){
        uint32_t c=u.p[i];
        if(c>=0xd800&&c<=0xdbff&&i+1<u.n&&u.p[i+1]>=0xdc00&&u.p[i+1]<=0xdfff)c=0x10000+((c-0xd800)<<10)+(u.p[++i]-0xdc00);
        cp[n++]=c;
    }
    TBuffer b={0};
    for(size_t i=0;i<n;i++){
        uint32_t c=cp[i];
        if(!upper&&c==0x3a3){
            int before=0,after=0;
            for(size_t j=i;j>0;){
                uint32_t x=cp[--j];
                if(t_property(x,unicode_CI,NEL(unicode_CI)))continue;
                before=t_property(x,unicode_Cased,NEL(unicode_Cased));
                break;
            }
            for(size_t j=i+1;j<n;j++){
                uint32_t x=cp[j];
                if(t_property(x,unicode_CI,NEL(unicode_CI)))continue;
                after=t_property(x,unicode_Cased,NEL(unicode_Cased));
                break;
            }
            t_cp(&b,before&&!after?0x3c2:0x3c3);
            continue;
        }
        const USpecial*s=NULL;
        for(size_t j=0;j<NEL(unicode_special);j++)if(unicode_special[j].cp==c){
            s=unicode_special+j;
            break;
        }
        if(s){
            unsigned k=upper?s->un:s->ln;
            for(unsigned j=0;j<k;j++)t_cp(&b,upper?s->up[j]:s->lo[j]);
        }
        else t_cp(&b,t_case_simple(c,upper));
    }
    free(cp);
    free(u.p);
    return t_finish(&b);
}

static FLValue t_clone(FLValue v){
    if(t_numeric(v)){
        double d=t_double(v);
        return !isfinite(d)?fl_nil():d==0?fl_int(0):v;
    }
    if(v.tag==FL_STRING)return t_text(v);
    if(v.tag==FL_VECTOR){
        FLVector*a=(FLVector*)v.obj;
        FLValue r=fl_vec_new();
        for(uint32_t i=0;i<a->len;i++)r=fl_vec_push(r,t_clone(a->data[i]));
        return r;
    }
    if(v.tag==FL_MAP){
        FLValue keys=t_keys(v),r=fl_map_new();
        FLVector*a=(FLVector*)keys.obj;
        for(uint32_t i=0;i<a->len;i++)r=fl_map_set(r,a->data[i],t_clone(fl_map_get(v,a->data[i])));
        return r;
    }
    return v;
}

FLValue fl_v11_map_from_pairs(FLValue *kv,uint32_t n){
    FLValue r=fl_map_new();
    for(uint32_t i=0;i<n;i++)r=fl_map_set(r,t_key(kv[i*2]),kv[i*2+1]);
    return r;
}

static int t_compare(FLValue a,FLValue b){
    if(a.tag==FL_STRING&&b.tag==FL_STRING)return t_str_compare(a,b);
    double x=t_numeric(a)?t_double(a):a.tag==FL_NIL?0:a.tag==FL_BOOL?a.b:t_double(t_parse(a,0));
    double y=t_numeric(b)?t_double(b):b.tag==FL_NIL?0:b.tag==FL_BOOL?b.b:t_double(t_parse(b,0));
    if(isnan(x)||isnan(y))return 2;
    return x<y?-1:x>y?1:0;
}

static void t_op_name(FLValue name,char out[80]){
    const char*p=name.tag==FL_STRING?((FLString*)name.obj)->data:"";
    /* v11 text/regex modules coexist with the existing kebab-case native APIs. */
    int exact=!strncmp(p,"file_",5)||!strncmp(p,"dir_",4)||!strncmp(p,"regex_",6);
    size_t i=0;
    for(;p[i]&&i<79;i++)out[i]=!exact&&p[i]=='_'?'-':p[i];
    out[i]=0;
}

FLValue fl_v11_has(FLValue name){
    char op[80];
    t_op_name(name,op);
    for(size_t i=0;v11_ops[i];i++)if(!strcmp(op,v11_ops[i]))return fl_bool(true);
    return fl_bool(false);
}

/* Capture only a scalar opcode so a callback cannot retain request-arena text. */
static FLValue v11_fn_call(FLClosure*s,int n,FLValue*a){
    return fl_v11_builtin(fl_str_val(v11_ops[s->env[0].i]),fl_vec_from(a,n));
}

FLValue fl_v11_fn(FLValue name){
    char op[80];
    t_op_name(name,op);
    for(size_t i=0;v11_ops[i];i++)if(!strcmp(op,v11_ops[i])){
        FLValue index=fl_int(i);
        return fl_fn_new(v11_fn_call,1,&index);
    }
    return fl_nil();
}

static int t_hex(uint16_t c){
    return c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:c>='a'&&c<='f'?c-'a'+10:-1;
}

/* String.replaceAll tokens, implemented without a regular-expression engine. */
static FLValue t_replace_all(FLValue input,FLValue pattern,FLValue replacement){
    TUnits s=t_units(t_js_string(input)),p=t_units(t_js_string(pattern));
    TUnits r=t_units(t_js_string(replacement)),out={0};
    size_t cursor=0,scan=0;
    while(scan<=s.n){
        int64_t found=p.n?t_find_units(s,p,scan):(int64_t)scan;
        if(found<0)break;
        size_t at=(size_t)found;
        for(size_t j=cursor;j<at;j++)t_unit(&out,s.p[j]);
        for(size_t j=0;j<r.n;j++){
            if(r.p[j]=='$'&&j+1<r.n){
                uint16_t token=r.p[j+1];
                if(token=='$'){t_unit(&out,'$');j++;continue;}
                if(token=='&'||token==96||token=='\''){
                    size_t lo=token==96?0:token=='&'?at:at+p.n;
                    size_t hi=token==96?at:token=='&'?at+p.n:s.n;
                    for(size_t k=lo;k<hi;k++)t_unit(&out,s.p[k]);
                    j++;continue;
                }
            }
            t_unit(&out,r.p[j]);
        }
        cursor=at+p.n;
        if(!p.n&&at==s.n)break;
        scan=p.n?cursor:at+1;
    }
    for(size_t j=cursor;j<s.n;j++)t_unit(&out,s.p[j]);
    FLValue value=t_from_units(out.p,out.n);
    free(s.p);free(p.p);free(r.p);free(out.p);
    return value;
}

#include "v11-time.h"
#include "v11-regex.h"
#include "v11-error.h"
#include "v11-json.h"
#include "v11-auth.h"
#include "v11-feed.h"

FLValue fl_v11_to_number(FLValue value){return t_numeric(value)?fl_float(t_double(value)):t_parse(value,0);}
FLValue fl_v11_header_value(FLValue value){
    TUnits units=t_units(t_js_string(value));TBuffer bytes={0};int invalid=0;
    for(size_t i=0;i<units.n;i++){uint16_t c=units.p[i];if(c>255||c==127||(c<32&&c!=9)){invalid=1;break;}char byte=(char)c;t_put(&bytes,&byte,1);}
    free(units.p);
    if(invalid){free(bytes.p);fl_throw(fl_make_error("TypeError","invalid HTTP response header"));return fl_nil();}
    return t_finish(&bytes);
}
FLValue fl_v11_utf8_string(FLValue value){
    TUnits units=t_units(t_js_string(value));TBuffer bytes={0};
    t_encode(&bytes,units.p,units.n,1);free(units.p);return t_finish(&bytes);
}
FLValue fl_v11_latin1_decode(const char*data,size_t length){TBuffer bytes={0};for(size_t i=0;i<length;i++)t_cp(&bytes,(unsigned char)data[i]);return t_finish(&bytes);}
FLValue fl_v11_latin1_encode(FLValue value){TUnits units=t_units(t_js_string(value));TBuffer bytes={0};for(size_t i=0;i<units.n;i++){char byte=(char)(units.p[i]&255);t_put(&bytes,&byte,1);}free(units.p);return t_finish(&bytes);}

FLValue fl_v11_builtin(FLValue name,FLValue argv){
    char op[80];
    t_op_name(name,op);
    FLVector*av=(FLVector*)argv.obj;
    size_t n=av->len;
    FLValue*a=av->data;
#define IS(s) (!strcmp(op,s))
#define ARG(i) ((size_t)(i)<n?a[i]:fl_nil())
    if(IS("identity"))return ARG(0);
    if(IS("file-sha256-result")||IS("file_sha256_result")){
        if(n!=1){fl_throw(fl_make_error("TypeError","file-sha256-result: exactly one argument required"));return fl_nil();}
        return fl_file_sha256_result(ARG(0));
    }
    if(IS("bytes-count")||IS("bytes-slice")){
        int slicing=IS("bytes-slice");
        if(n!=(slicing?3:1)||ARG(0).tag!=FL_STRING){
            fl_throw(fl_make_error("TypeError","bytes API: string and exact argument count required"));return fl_nil();
        }
        FLString*s=(FLString*)ARG(0).obj;
        if(!slicing)return fl_int(s->len);
        int64_t bounds[2];
        for(size_t i=0;i<2;i++){
            FLValue value=ARG(i+1);
            if(value.tag==FL_INT)bounds[i]=value.i;
            else if(value.tag==FL_FLOAT&&isfinite(value.f)&&trunc(value.f)==value.f&&value.f>=0&&value.f<0x1p63)bounds[i]=(int64_t)value.f;
            else {fl_throw(fl_make_error("TypeError","bytes-slice: integer bounds required"));return fl_nil();}
        }
        if(bounds[0]<0||bounds[1]<bounds[0]||(uint64_t)bounds[1]>s->len){
            fl_throw(fl_make_error("RangeError","bytes-slice: invalid bounds"));return fl_nil();
        }
        size_t length=(size_t)(bounds[1]-bounds[0]);
        if(length>SIZE_MAX-sizeof(FLString)-1){fl_throw(fl_make_error("RangeError","bytes-slice: size overflow"));return fl_nil();}
        FLString*result=fl_arena_alloc(sizeof(*result)+length+1);
        if(!result){fl_throw(fl_make_error("Error","bytes-slice: allocation failed"));return fl_nil();}
        result->base.type=FL_STRING;result->base.rc=1;result->len=length;
        memcpy(result->data,s->data+bounds[0],length);result->data[length]=0;
        return (FLValue){.tag=FL_STRING,.obj=(FLObject*)result};
    }
    if(IS("json-parse"))return vj_parse(ARG(0));
    if(IS("json-str")||IS("json-stringify"))return vj_stringify(ARG(0));
    if(IS("auth-sha256")){
        if(ARG(0).tag!=FL_STRING)fl_throw(fl_make_error("TypeError","auth_sha256: string required"));
        TUnits text=t_units(ARG(0));TBuffer bytes={0};t_encode(&bytes,text.p,text.n,1);free(text.p);
        return sha256(t_finish(&bytes));
    }
    if(!strncmp(op,"auth-",5))return va_auth(op,argv);
    if(!strncmp(op,"server-",7))return fl_v11_http(fl_str_val(op),argv);
    if(IS("http-get")||IS("http-request")||IS("http-request-timeout")||IS("http-with-timeout"))return fl_v11_http_client(fl_str_val(op),argv);
    if(IS("set-timeout")||IS("await-result")||IS("fl-yield"))return fl_v11_timer(fl_str_val(op),argv);
    if(!strncmp(op,"tcp-",4)||IS("fl-error-drain")||IS("fl-error-queue-size"))return fl_v11_io(fl_str_val(op),argv);
    if(IS("rss-feed")||IS("sitemap-xml"))return vf_feed(op,argv);
    if(!strncmp(op,"fd-",3)){
        FLValue args=argv;
        if(IS("fd-write")&&ARG(1).tag==FL_STRING){
            TUnits text=t_units(ARG(1));TBuffer bytes={0};t_encode(&bytes,text.p,text.n,1);free(text.p);
            FLValue encoded[2]={ARG(0),t_finish(&bytes)};args=fl_vec_from(encoded,2);
        }
        FLValue result=fl_v11_fd(fl_str_val(op),args);
        return IS("fd-read")?t_text(result):result;
    }
    if(IS("env-get")||IS("env-or")){
        FLValue key=t_js_string(ARG(0));
        const char *value=getenv(((FLString*)key.obj)->data);
        return value&&*value?t_text(fl_str_val(value)):IS("env-or")?ARG(1):fl_nil();
    }
    if(IS("error-message")){
        FLValue value=ARG(0);
        if(value.tag==FL_MAP){
            FLValue message=fl_map_get(value,fl_str_val("message"));
            if(fl_v11_truthy(message))return t_js_string(message);
        }
        return t_js_string(value);
    }
    if(IS("process-exit")){
        FLValue code=ARG(0);
        double number=code.tag==FL_NIL?0:t_double(code);
        if((code.tag!=FL_NIL&&!t_numeric(code))||!isfinite(number)||trunc(number)!=number||number<INT_MIN||number>INT_MAX)
            fl_throw(fl_str_val("process_exit: integer exit code required"));
        fl_schedule_shutdown();exit((int)number);
    }
    if(IS("uuid-short")){
        unsigned char bytes[4];char hex[9];
        if(RAND_bytes(bytes,sizeof(bytes))!=1)fl_throw(fl_str_val("uuid_short: secure random failed"));
        for(size_t i=0;i<sizeof(bytes);i++)snprintf(hex+i*2,3,"%02x",bytes[i]);
        return fl_str_val(hex);
    }
    if(IS("date-parse")||IS("format-date")||IS("now-ms"))return v11_time(op,argv);
    if(!strncmp(op,"regex_",6))return v11_regex(op,argv);
    if(!strncmp(op,"file_",5)||!strncmp(op,"dir_",4)){
        if((IS("file_write")||IS("file_append"))&&n>1&&a[1].tag==FL_STRING){
            TUnits text=t_units(a[1]);TBuffer bytes={0};t_encode(&bytes,text.p,text.n,1);free(text.p);
            FLValue args[2]={ARG(0),t_finish(&bytes)};
            return fl_v11_file(fl_str_val(op),fl_vec_from(args,2));
        }
        FLValue result=fl_v11_file(fl_str_val(op),argv);
        return IS("file_read")?t_text(result):result;
    }
    if(IS("length")||IS("count")||IS("str-length")){
        FLValue v=ARG(0);
        if(v.tag==FL_STRING){
            TUnits u=t_units(v);
            size_t k=u.n;
            free(u.p);
            return fl_int(k);
        }
        return length(v);
    }
    if(IS("char-at"))return fl_v11_text_get(ARG(0),t_parse(ARG(1),0),fl_str_val(""));
    if(IS("str-slice"))return t_slice(t_js_string(ARG(0)),ARG(1),ARG(2),n>2);
    if(IS("slice"))return t_slice(ARG(0),ARG(1),ARG(2),n>2);
    if(IS("substring")){
        if(ARG(0).tag!=FL_STRING)return fl_str_val("");
        TUnits text=t_units(ARG(0));
        double start=floor(t_double(t_parse(fl_v11_truthy(ARG(1))?ARG(1):fl_int(0),0)));
        double end=floor(t_double(t_parse(fl_v11_truthy(ARG(2))?ARG(2):fl_int(text.n),0)));
        size_t lo=isnan(start)||start<0?0:start>text.n?text.n:(size_t)start;
        size_t hi=isnan(end)||end<0?0:end>text.n?text.n:(size_t)end;
        if(lo>hi){size_t swap=lo;lo=hi;hi=swap;}
        FLValue value=t_from_units(text.p+lo,hi-lo);free(text.p);return value;
    }
    if(IS("str")||IS("print")||IS("println")){
        TBuffer b={0};
        for(size_t i=0;i<n;i++){
            if(!IS("str")&&i)t_ascii(&b," ");
            FLValue v=a[i];
            if(IS("str")&&v.tag==FL_STRING){
                TUnits u=t_units(v);
                t_encode(&b,u.p,u.n,0);
                free(u.p);
            }
            else t_render(&b,v,IS("str")&&(v.tag==FL_VECTOR||v.tag==FL_MAP),0);
        }
        if(IS("str")){
            FLValue raw=t_finish(&b);
            return t_text(raw);
        }
        if(b.n)fwrite(b.p,1,b.n,stdout);
        if(IS("println"))fputc('\n',stdout);
        free(b.p);
        return fl_nil();
    }
    if(IS("num-to-str"))return t_js_string(ARG(0));
    if(IS("str-to-num"))return t_parse(ARG(0),1);
    if(IS("integer?")||IS("float?")){
        FLValue v=ARG(0);
        int integer=t_numeric(v)&&isfinite(t_double(v))&&floor(t_double(v))==t_double(v);
        return fl_bool(IS("integer?")?integer:t_numeric(v)&&!integer);
    }
    if(IS("type-of"))return fl_str_val(t_numeric(ARG(0))?"number":ARG(0).tag==FL_BOOL?"boolean":fl_type_name(ARG(0)));
    if(IS("not"))return fl_bool(!fl_v11_truthy(ARG(0)));
    if(IS("=")||IS("eq")||IS("!=")||IS("neq")||IS("not=")){
        int different=IS("!=")||IS("neq")||IS("not=");
        return fl_bool(different?!t_equal(ARG(0),ARG(1),0):t_equal(ARG(0),ARG(1),1));
    }
    if(IS("<")||IS(">")||IS("<=")||IS(">=")){
        int c=t_compare(ARG(0),ARG(1));
        return fl_bool(c!=2&&(IS("<")?c<0:IS(">")?c>0:IS("<=")?c<=0:c>=0));
    }
    if(IS("+")||IS("-")||IS("*")||IS("/")||IS("%")||IS("mod")){
        if(IS("%")||IS("mod"))return fl_float(fmod(t_double(t_parse(ARG(0),0)),t_double(t_parse(ARG(1),0))));
        for(size_t i=0;i<n;i++)if(!t_numeric(a[i])){
            char message[512];
            snprintf(message,sizeof(message),"[E_TYPE_MISMATCH] %s: 인자 %zu번에 %s 전달됨 — number 필요 %s",op,i+1,a[i].tag==FL_BOOL?"boolean":fl_type_name(a[i]),a[i].tag==FL_NIL?"(nil? 확인 후 기본값 사용)":"(str-to-num 변환 확인)");
            fl_throw(fl_str_val(message));
        }
        if(!n){
            if(IS("-")||IS("/"))fl_throw(fl_str_val("Reduce of empty array with no initial value"));
            return fl_int(IS("*")?1:0);
        }
        FLValue r=IS("+")?fl_float(0):a[0];
        if(n==1&&IS("-"))return fl_float(-t_double(r));
        if(n==1&&IS("/"))return fl_float(1/t_double(r));
        for(size_t i=IS("+")?0:1;i<n;i++){
            double x=t_double(r),y=t_double(a[i]);
            r=fl_float(IS("+")?x+y:IS("-")?x-y:IS("*")?x*y:IS("/")?x/y:fmod(x,y));
        }
        return r;
    }
    if(IS("str-upper")||IS("str-to-upper")||IS("str-lower")||IS("str-to-lower"))return t_case(ARG(0),IS("str-upper")||IS("str-to-upper"));
    if(IS("str-trim")){
        FLValue v=ARG(0);
        if(v.tag!=FL_STRING)return v.tag==FL_NIL?v:fl_str_val("");
        TUnits u=t_units(v);
        size_t lo=0,hi=u.n;
        while(lo<hi&&t_space(u.p[lo]))lo++;
        while(hi>lo&&t_space(u.p[hi-1]))hi--;
        FLValue r=t_from_units(u.p+lo,hi-lo);
        free(u.p);
        return r;
    }
    if(IS("html-escape")){
        FLValue s=ARG(0).tag==FL_NIL?fl_str_val(""):t_js_string(ARG(0));
        TUnits u=t_units(s);
        TBuffer b={0};
        for(size_t i=0;i<u.n;i++){
            uint16_t c=u.p[i];
            const char*escape=c=='&'?"&amp;":c=='<'?"&lt;":c=='>'?"&gt;":c=='"'?"&quot;":c=='\''?"&#x27;":NULL;
            if(escape)t_ascii(&b,escape);
            else{
                size_t k=1;
                if(c>=0xd800&&c<=0xdbff&&i+1<u.n&&u.p[i+1]>=0xdc00&&u.p[i+1]<=0xdfff)k=2;
                t_encode(&b,u.p+i,k,0);
                i+=k-1;
            }
        }
        free(u.p);
        return t_finish(&b);
    }
    if(IS("str-find")||IS("str-index-of")||IS("str-starts")||IS("str-ends")||IS("str-starts-with")||IS("str-ends-with")||IS("str-includes")){
        if((IS("str-starts-with")||IS("str-ends-with")||IS("str-includes"))&&(ARG(0).tag!=FL_STRING||ARG(1).tag!=FL_STRING))return fl_bool(false);
        if(IS("str-index-of")&&ARG(0).tag!=FL_STRING)return fl_int(-1);
        FLValue x=t_js_string(ARG(0)),y=IS("str-index-of")&&ARG(1).tag==FL_NIL?fl_str_val(""):t_js_string(ARG(1));
        TUnits u=t_units(x),v=t_units(y);
        double offset=n>2?t_double(t_parse(ARG(2),0)):0;
        size_t start=isnan(offset)||offset<0?0:offset>=u.n?u.n:(size_t)offset;
        int64_t found=t_find_units(u,v,start);
        FLValue r;
        if(IS("str-find")||IS("str-index-of"))r=fl_int(found);
        else if(IS("str-starts")||IS("str-starts-with"))r=fl_bool(v.n<=u.n&&!memcmp(u.p,v.p,v.n*2));
        else if(IS("str-ends")||IS("str-ends-with"))r=fl_bool(v.n<=u.n&&!memcmp(u.p+u.n-v.n,v.p,v.n*2));
        else r=fl_bool(found>=0);
        free(u.p);
        free(v.p);
        return r;
    }
    if(IS("str-join")||IS("join")){
        FLValue vec=ARG(0),sep=ARG(1);
        if(vec.tag==FL_STRING&&sep.tag==FL_VECTOR){
            FLValue tmp=vec;
            vec=sep;
            sep=tmp;
        }
        if(vec.tag!=FL_VECTOR)return fl_str_val("");
        FLValue s=n>1?t_js_string(sep):fl_str_val("");
        FLString*ss=(FLString*)s.obj;
        TBuffer b={0};
        FLVector*v=(FLVector*)vec.obj;
        for(uint32_t i=0;i<v->len;i++){
            if(i)t_put(&b,ss->data,ss->len);
            if(v->data[i].tag!=FL_NIL){
                FLValue part=t_js_string(v->data[i]);
                FLString*p=(FLString*)part.obj;
                t_put(&b,p->data,p->len);
            }
        }
        FLValue r=t_finish(&b);
        return t_text(r);
    }
    if(IS("str-replace-in"))return t_replace_all(ARG(0),ARG(1),ARG(2));
    if(IS("str-lines")||IS("str-split")||IS("str-replace")){
        if(IS("str-split")&&(ARG(0).tag!=FL_STRING||ARG(1).tag!=FL_STRING))return fl_vec_new();
        if(IS("str-replace")&&ARG(0).tag!=FL_STRING)return fl_str_val("");
        FLValue s=t_js_string(ARG(0)),delim=IS("str-lines")?fl_str_val("\n"):IS("str-replace")&&ARG(1).tag==FL_NIL?fl_str_val(""):t_js_string(ARG(1));
        TUnits u=t_units(s),d=t_units(delim);
        FLValue parts=fl_vec_new();
        size_t pos=0;
        if(!d.n){
            for(size_t i=0;i<u.n;i++)parts=fl_vec_push(parts,t_from_units(u.p+i,1));
        }
        else{
            int64_t at;
            while((at=t_find_units(u,d,pos))>=0){
                parts=fl_vec_push(parts,t_from_units(u.p+pos,at-pos));
                pos=at+d.n;
            }
            parts=fl_vec_push(parts,t_from_units(u.p+pos,u.n-pos));
        }
        free(u.p);
        free(d.p);
        if(IS("str-lines"))return parts;
        if(IS("str-replace")||IS("str-replace-in")){
            FLValue j[2]={
                parts,ARG(2).tag==FL_NIL?fl_str_val(""):t_js_string(ARG(2))
            };
            return fl_v11_builtin(fl_str_val("str-join"),fl_vec_from(j,2));
        }
        double requested=n>2?t_double(t_parse(a[2],0)):0;
        if(requested>0&&requested<((FLVector*)parts.obj)->len){
            size_t limit=(size_t)requested;
            FLVector*p=(FLVector*)parts.obj;
            if(limit&&p->len>limit){
                FLValue result=fl_vec_from(p->data,limit-1),j[2]={
                    fl_vec_from(p->data+limit-1,p->len-limit+1),delim
                };
                return fl_vec_push(result,fl_v11_builtin(fl_str_val("str-join"),fl_vec_from(j,2)));
            }
        }
        return parts;
    }
    if(IS("str-encode-uri")||IS("str-decode-uri")){
        FLValue s=t_js_string(ARG(0));
        TUnits u=t_units(s);
        TBuffer b={0};
        if(IS("str-encode-uri")){
            for(size_t i=0;i<u.n;i++){
                uint32_t c=u.p[i];
                if(c>=0xd800&&c<=0xdbff&&i+1<u.n&&u.p[i+1]>=0xdc00&&u.p[i+1]<=0xdfff)c=0x10000+((c-0xd800)<<10)+(u.p[++i]-0xdc00);
                else if(c>=0xd800&&c<=0xdfff){
                    free(u.p);
                    free(b.p);
                    fl_throw(fl_str_val("URI malformed"));
                    return fl_nil();
                }
                if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||(c>0&&c<128&&strchr("-_.!~*'()",c)))t_cp(&b,c);
                else{
                    TBuffer bytes={0};
                    t_cp(&bytes,c);
                    for(size_t j=0;j<bytes.n;j++){
                        char hex[4];
                        snprintf(hex,sizeof(hex),"%%%02X",(unsigned char)bytes.p[j]);
                        t_ascii(&b,hex);
                    }
                    free(bytes.p);
                }
            }
            free(u.p);
            return t_finish(&b);
        }
        TUnits result={0};
        for(size_t i=0;i<u.n;){
            if(u.p[i]!='%'){
                t_unit(&result,u.p[i++]);
                continue;
            }
            TBuffer bytes={0};
            while(i<u.n&&u.p[i]=='%'){
                if(i+2>=u.n||t_hex(u.p[i+1])<0||t_hex(u.p[i+2])<0){
                    free(u.p);
                    free(result.p);
                    free(bytes.p);
                    fl_throw(fl_str_val("URI malformed"));
                    return fl_nil();
                }
                char c=t_hex(u.p[i+1])*16+t_hex(u.p[i+2]);
                t_put(&bytes,&c,1);
                i+=3;
            }
            FLValue raw=t_bytes(bytes.p?bytes.p:"",bytes.n,0);
            TUnits decoded=t_units(raw);
            TBuffer check={0};
            t_encode(&check,decoded.p,decoded.n,0);
            int valid=check.n==bytes.n&&(!bytes.n||!memcmp(check.p,bytes.p,bytes.n));
            free(check.p);
            free(bytes.p);
            if(!valid){
                free(u.p);
                free(result.p);
                free(decoded.p);
                fl_throw(fl_str_val("URI malformed"));
                return fl_nil();
            }
            for(size_t j=0;j<decoded.n;j++)t_unit(&result,decoded.p[j]);
            free(decoded.p);
        }
        free(u.p);
        FLValue decoded=t_from_units(result.p,result.n);
        free(result.p);
        return decoded;
    }
    if(IS("assoc")||IS("obj-merge")||IS("merge")||IS("dissoc")){
        FLValue r=ARG(0);
        if(IS("assoc")){
            for(size_t i=1;i+1<n;i+=2)r=fl_map_set(r,t_key(a[i]),a[i+1]);
        }
        else if(IS("dissoc")){
            for(size_t i=1;i<n;i++)r=dissoc(r,t_key(a[i]));
        }
        else for(size_t i=1;i<n;i++)r=fl_map_merge(r,a[i]);
        return r;
    }
    if(IS("keys"))return t_keys(ARG(0));
    if(IS("vals")){
        FLValue ks=t_keys(ARG(0)),r=fl_vec_new();
        FLVector*k=(FLVector*)ks.obj;
        for(uint32_t i=0;i<k->len;i++)r=fl_vec_push(r,fl_map_get(ARG(0),k->data[i]));
        return r;
    }
    if(IS("append")||IS("conj")){
        FLValue r=ARG(0);
        if(IS("append")&&n==2&&r.tag==FL_VECTOR&&a[1].tag==FL_VECTOR)return append(r,a[1]);
        if(IS("append")&&r.tag==FL_STRING){
            TUnits text=t_units(r);r=fl_vec_new();
            for(size_t j=0;j<text.n;j++){
                size_t width=text.p[j]>=0xd800&&text.p[j]<=0xdbff&&j+1<text.n&&text.p[j+1]>=0xdc00&&text.p[j+1]<=0xdfff?2:1;
                r=fl_vec_push(r,t_from_units(text.p+j,width));j+=width-1;
            }
            free(text.p);
        }
        else if(IS("append")&&r.tag!=FL_VECTOR&&fl_v11_truthy(r))fl_throw(fl_str_val("(args[0] || []) is not iterable"));
        for(size_t i=1;i<n;i++){
            if(r.tag==FL_MAP&&IS("conj")){
                if(a[i].tag==FL_MAP)r=fl_map_merge(r,a[i]);
                else if(a[i].tag==FL_VECTOR&&((FLVector*)a[i].obj)->len==2){
                    FLVector*p=(FLVector*)a[i].obj;
                    r=fl_map_set(r,t_key(p->data[0]),p->data[1]);
                }
            }
            else r=fl_vec_push(r.tag==FL_VECTOR?r:fl_vec_new(),a[i]);
        }
        return r;
    }
    if(IS("concat")){
        if(ARG(0).tag==FL_VECTOR){
            if(ARG(1).tag!=FL_VECTOR)return ARG(0);
            return append(a[0],a[1]);
        }
        TBuffer b={0};
        for(size_t i=0;i<n;i++){
            if(a[i].tag==FL_NIL)continue;
            FLValue s=t_js_string(a[i]);
            t_put(&b,((FLString*)s.obj)->data,((FLString*)s.obj)->len);
        }
        return t_finish(&b);
    }
    if(IS("arr-flatten")||IS("flatten-1"))return flatten(ARG(0));
    if(IS("arr-unique")){
        FLValue v=ARG(0),r=fl_vec_new(),seen=fl_vec_new();
        if(v.tag!=FL_VECTOR)return r;
        FLVector*p=(FLVector*)v.obj;
        for(uint32_t i=0;i<p->len;i++){
            TBuffer b={0};
            t_render(&b,p->data[i],1,0);
            FLValue key=t_finish(&b);
            FLVector*keys=(FLVector*)seen.obj;
            int found=0;
            for(uint32_t j=0;j<keys->len;j++)if(t_equal(key,keys->data[j],0)){
                found=1;
                break;
            }
            if(!found){
                seen=fl_vec_push(seen,key);
                r=fl_vec_push(r,t_clone(p->data[i]));
            }
        }
        return r;
    }
    if(IS("arr-sort-by")||IS("arr-sort-by-desc")){
        FLValue v=ARG(0);
        if(v.tag!=FL_VECTOR)return fl_vec_new();
        FLVector*p=(FLVector*)v.obj;
        FLValue r=fl_vec_from(p->data,p->len);
        FLVector*out=(FLVector*)r.obj;
        for(uint32_t i=1;i<out->len;i++){
            FLValue item=out->data[i],key=fl_map_get(item,t_key(ARG(1)));
            uint32_t j=i;
            while(j){
                FLValue previous=fl_map_get(out->data[j-1],t_key(ARG(1)));
                if(!fl_map_has(item,t_key(ARG(1))).b||!fl_map_has(out->data[j-1],t_key(ARG(1))).b)break;
                int c=t_compare(previous,key);
                if(c==2||(IS("arr-sort-by")?c<=0:c>=0))break;
                out->data[j]=out->data[j-1];
                j--;
            }
            out->data[j]=item;
        }
        return r;
    }
    if(IS("first")||IS("last")||IS("rest")||IS("reverse")||IS("take")||IS("drop")){
        if((IS("take")||IS("drop"))&&ARG(0).tag==FL_VECTOR&&t_numeric(ARG(1))){
            fl_throw(fl_str_val(IS("take")?"take 인자 순서 오류: (take n coll) 형식이 올바릅니다. 현재 입력: (take coll n)":"drop 인자 순서 오류: (drop n coll) 형식이 올바릅니다. 현재 입력: (drop coll n)"));
        }
        FLValue v=(IS("take")||IS("drop"))?ARG(1):ARG(0);
        if(v.tag!=FL_VECTOR)return(IS("first")||IS("last"))?fl_nil():fl_vec_new();
        FLVector*p=(FLVector*)v.obj;
        if(IS("first"))return p->len?p->data[0]:fl_nil();
        if(IS("last"))return p->len?p->data[p->len-1]:fl_nil();
        if(IS("reverse")){
            FLValue r=fl_vec_new();
            for(uint32_t i=p->len;i;i--)r=fl_vec_push(r,p->data[i-1]);
            return r;
        }
        if(IS("rest"))return fl_vec_from(p->data+(p->len?1:0),p->len?p->len-1:0);
        if(IS("drop"))return t_slice(v,ARG(0),fl_nil(),0);
        double d=t_numeric(ARG(0))?t_double(ARG(0)):0;
        size_t k=d<0?0:d>p->len?p->len:(size_t)d;
        return IS("take")?fl_vec_from(p->data,k):fl_vec_from(p->data+k,p->len-k);
    }
    if(IS("range")){
        /* Finite array ranges only; lazy sequences are outside this phase. */
        if(!n)fl_throw(fl_str_val("lazy range is not supported"));
        double start=n==1?0:t_double(t_parse(ARG(0),0));
        double end=t_double(t_parse(ARG(n==1?0:1),0));
        double step=n>2?t_double(t_parse(ARG(2),0)):1;
        FLValue r=fl_vec_new();
        for(double i=start;step>0?i<end:step<0&&i>end;i+=step)r=fl_vec_push(r,fl_float(i));
        return r;
    }
    if(IS("map"))return fl_map_fn(ARG(0),ARG(1));
    if(IS("filter")){
        FLValue r=fl_vec_new(),v=ARG(1);
        if(v.tag!=FL_VECTOR)return r;
        if(ARG(0).tag==FL_NIL)return v;
        FLVector*p=(FLVector*)v.obj;
        for(uint32_t i=0;i<p->len;i++)if(fl_v11_truthy(fl_fn_call(ARG(0),1,p->data+i)))r=fl_vec_push(r,p->data[i]);
        return r;
    }
    if(IS("reduce"))return fl_reduce_fn(ARG(0),ARG(1),ARG(2));
    if(IS("any?")||IS("every?")){
        FLValue v=ARG(1);
        if(v.tag!=FL_VECTOR)return fl_bool(IS("every?"));
        FLVector*p=(FLVector*)v.obj;
        for(uint32_t i=0;i<p->len;i++){
            FLValue result=fl_fn_call(ARG(0),1,p->data+i);
            int yes=fl_v11_truthy(result);
            if(IS("any?")&&yes)return fl_bool(true);
            if(IS("every?")&&!yes)return fl_bool(false);
        }
        return fl_bool(IS("every?"));
    }
    fl_throw(fl_str_val("unsupported v11 operation"));
    return fl_nil();
#undef IS
#undef ARG
}
