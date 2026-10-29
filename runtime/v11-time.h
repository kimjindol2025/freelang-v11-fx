/* ISO date contract used by v11's time module. No process-global TZ mutation. */
#include <time.h>
static int vt_leap(int y){
    return y%4==0&&(y%100!=0||y%400==0);
}

static int64_t vt_days(int y,unsigned m,unsigned d){
    y-=m<=2;
    int era=(y>=0?y:y-399)/400;
    unsigned yy=y-era*400;
    unsigned doy=(153*(m+(m>2?-3:9))+2)/5+d-1;
    return (int64_t)era*146097+(yy*365+yy/4-yy/100+doy)-719468;
}

static int vt_digits(const char **p,int n,int *v){
    *v=0;
    for(int i=0;i<n;i++){
        if(**p<'0'||**p>'9')return 0;
        *v=*v*10+*(*p)++-'0';
    }
    return 1;
}

static int vt_same(struct tm *a,struct tm *b){
    return a->tm_year==b->tm_year&&a->tm_mon==b->tm_mon&&a->tm_mday==b->tm_mday&&a->tm_hour==b->tm_hour&&a->tm_min==b->tm_min&&a->tm_sec==b->tm_sec;
}

static time_t vt_local(struct tm value){
    struct tm a=value,b=value,ra,rb;
    a.tm_isdst=0;
    b.tm_isdst=1;
    time_t x=mktime(&a),y=mktime(&b);
    localtime_r(&x,&ra);
    localtime_r(&y,&rb);
    int ax=vt_same(&ra,&value),by=vt_same(&rb,&value);
    if(ax&&by)return x<y?x:y;
    if(ax)return x;
    if(by)return y;
    value.tm_isdst=-1;
    return mktime(&value);
}

static FLValue vt_parse(FLValue input){
    FLValue text=t_js_string(input);
    FLString*s=(FLString*)text.obj;
    const char *p=s->data;
    int y,m=1,d=1,h=0,mi=0,se=0,ms=0,sign=1,zone=0,has_time=0,explicit_zone=0;
    if(*p=='+'||*p=='-'){
        sign=*p++=='-'?-1:1;
        if(!vt_digits(&p,6,&y))goto bad;
        y*=sign;
    }
    else if(!vt_digits(&p,4,&y))goto bad;
    if(*p=='-'){
        p++;
        if(!vt_digits(&p,2,&m))goto bad;
        if(*p=='-'){
            p++;
            if(!vt_digits(&p,2,&d))goto bad;
        }
    }
    if(m<1||m>12||d<1||d>31)goto bad;
    if(*p=='T'||*p==' '){
        has_time=1;
        p++;
        if(!vt_digits(&p,2,&h)||*p++!=':'||!vt_digits(&p,2,&mi))goto bad;
        if(*p==':'){
            p++;
            if(!vt_digits(&p,2,&se))goto bad;
        }
        if(*p=='.'){
            p++;
            int n=0;
            if(*p<'0'||*p>'9')goto bad;
            while(*p>='0'&&*p<='9'){
                if(n<3)ms=ms*10+*p-'0';
                n++;
                p++;
            }
            while(n++<3)ms*=10;
        }
        if(h>24||mi>59||se>59||(h==24&&(mi||se||ms)))goto bad;
        if(*p=='Z'){
            p++;
            explicit_zone=1;
        }
        else if(*p=='+'||*p=='-'){
            int zh,zm;
            int zs=*p++=='-'?-1:1;
            if(!vt_digits(&p,2,&zh))goto bad;
            if(*p==':')p++;
            if(!vt_digits(&p,2,&zm)||zh>23||zm>59)goto bad;
            zone=zs*(zh*60+zm);
            explicit_zone=1;
        }
    }
    if(*p||(size_t)(p-s->data)!=s->len)goto bad;
    int64_t seconds;
    if(!has_time||explicit_zone)seconds=vt_days(y,m,d)*86400+(int64_t)h*3600+mi*60+se-zone*60;
    else {
        struct tm date={
            .tm_year=y-1900,.tm_mon=m-1,.tm_mday=d,.tm_hour=h,.tm_min=mi,.tm_sec=se
        };
        seconds=vt_local(date);
    }
    double result=(double)seconds*1000+ms;
    if(fabs(result)>8640000000000000.0)goto bad;
    return fl_float(result);
    bad:;
    TBuffer b={0};
    t_ascii(&b,"date_parse: invalid date string \"");
    t_put(&b,s->data,s->len);
    t_ascii(&b,"\"");
    fl_throw(t_finish(&b));
    return fl_nil();
}

static FLValue vt_format(FLValue stamp,FLValue format){
    double n=t_double(t_parse(stamp,0));
    int valid=isfinite(n)&&fabs(n)<=8640000000000000.0;
    n=trunc(n);
    time_t seconds=valid?(time_t)floor(n/1000):0;
    struct tm date={0};
    if(valid&&!localtime_r(&seconds,&date))valid=0;
    const char *tokens[]={
        "YYYY","MM","DD","HH","mm","ss","SSS"
    };
    int values[]={
        date.tm_year+1900,date.tm_mon+1,date.tm_mday,date.tm_hour,date.tm_min,date.tm_sec,valid?(int)(n-(double)seconds*1000):0
    };
    FLValue result=t_js_string(format);
    for(int i=0;i<7;i++){
        FLString*s=(FLString*)result.obj;
        const char*found=strstr(s->data,tokens[i]);
        if(!found)continue;
        char value[32];
        if(!valid)strcpy(value,"NaN");
        else if(i==0)snprintf(value,sizeof(value),"%d",values[i]);
        else snprintf(value,sizeof(value),i==6?"%03d":"%02d",values[i]);
        TBuffer b={0};
        size_t at=found-s->data;
        t_put(&b,s->data,at);
        t_ascii(&b,value);
        size_t end=at+strlen(tokens[i]);
        t_put(&b,s->data+end,s->len-end);
        result=t_finish(&b);
    }
    return result;
}

static FLValue v11_time(const char *op,FLValue argv){
    FLVector*a=(FLVector*)argv.obj;
    FLValue x=a->len?a->data[0]:fl_nil(),y=a->len>1?a->data[1]:fl_nil();
    if(!strcmp(op,"date-parse"))return vt_parse(x);
    if(!strcmp(op,"format-date"))return vt_format(x,y);
    struct timespec now;
    if(clock_gettime(CLOCK_REALTIME,&now))fl_throw(fl_str_val("now_ms: clock failure"));
    return fl_int((int64_t)now.tv_sec*1000+now.tv_nsec/1000000);
}
