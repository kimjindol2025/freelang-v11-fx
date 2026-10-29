/* Source-compatible RSS/sitemap serializers, with safe CDATA boundaries. */
static FLValue vf_get(FLValue map,const char*key){return fl_map_get(map,fl_str_val(key));}
static FLValue vf_default(FLValue value,const char*fallback){return fl_v11_truthy(value)?value:fl_str_val(fallback);}
static void vf_char(TBuffer*out,TUnits*text,size_t*index){
 size_t at=*index,n=1;
 if(text->p[at]>=0xd800&&text->p[at]<=0xdbff&&at+1<text->n&&text->p[at+1]>=0xdc00&&text->p[at+1]<=0xdfff)n=2;
 t_encode(out,text->p+at,n,1);*index+=n-1;
}
static void vf_escape(TBuffer*out,FLValue value){
 FLValue text=value.tag==FL_NIL?fl_str_val(""):t_js_string(value);
 TUnits s=t_units(text);
 for(size_t i=0;i<s.n;i++){
  switch(s.p[i]){
   case '&':t_ascii(out,"&amp;");break;case '<':t_ascii(out,"&lt;");break;case '>':t_ascii(out,"&gt;");break;
   case '"':t_ascii(out,"&quot;");break;case '\'':t_ascii(out,"&apos;");break;default:vf_char(out,&s,&i);
  }
 }
 free(s.p);
}
static void vf_tag(TBuffer*out,const char*tag,FLValue value){t_ascii(out,"<");t_ascii(out,tag);t_ascii(out,">");vf_escape(out,value);t_ascii(out,"</");t_ascii(out,tag);t_ascii(out,">\n");}
static FLValue vf_utc(FLValue value,int now){
 double milliseconds=0;
 if(now){struct timespec stamp;if(clock_gettime(CLOCK_REALTIME,&stamp))fl_throw(fl_str_val("rss_feed: clock failure"));milliseconds=(double)stamp.tv_sec*1000+stamp.tv_nsec/1e6;}
 else if(t_numeric(value))milliseconds=t_double(value);
 else{
  if(fl_try_top>=FL_TRY_MAX)return fl_str_val("Invalid Date");
  FLTryFrame*frame=&fl_try_stack[fl_try_top++];
  if(setjmp(frame->buf)==0){FLValue stamp=vt_parse(value);fl_try_top--;milliseconds=t_double(stamp);}
  else{fl_try_top--;return fl_str_val("Invalid Date");}
 }
 if(!isfinite(milliseconds)||fabs(milliseconds)>8640000000000000.0)return fl_str_val("Invalid Date");
 time_t seconds=(time_t)floor(trunc(milliseconds)/1000);struct tm date;
 if(!gmtime_r(&seconds,&date))return fl_str_val("Invalid Date");
 static const char*week[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
 static const char*month[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
 char text[96],year[32];int y=date.tm_year+1900;
 if(y<0)snprintf(year,sizeof(year),"-%04d",-y);else snprintf(year,sizeof(year),"%04d",y);
 snprintf(text,sizeof(text),"%s, %02d %s %s %02d:%02d:%02d GMT",week[date.tm_wday],date.tm_mday,month[date.tm_mon],year,date.tm_hour,date.tm_min,date.tm_sec);
 return fl_str_val(text);
}
static void vf_cdata(TBuffer*out,FLValue value){
 TUnits text=t_units(t_js_string(value));
 for(size_t i=0;i<text.n;i++){
  if(i+2<text.n&&text.p[i]==']'&&text.p[i+1]==']'&&text.p[i+2]=='>'){t_ascii(out,"]]]]><![CDATA[>");i+=2;}
  else vf_char(out,&text,&i);
 }
 free(text.p);
}
static FLValue vf_feed(const char*op,FLValue argv){
 FLVector*args=(FLVector*)argv.obj;FLValue first=args->len?args->data[0]:fl_nil(),items=args->len>1?args->data[1]:fl_nil();
 if(items.tag!=FL_NIL&&items.tag!=FL_VECTOR){fl_throw(fl_make_error("TypeError","feed items must be an array"));return fl_nil();}
 FLVector*list=items.tag==FL_VECTOR?(FLVector*)items.obj:NULL;TBuffer out={0};
 t_ascii(&out,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 if(!strcmp(op,"rss-feed")){
  FLValue url=vf_get(first,"url");if(url.tag!=FL_STRING){free(out.p);fl_throw(fl_make_error("TypeError","rss_feed: URL string required"));return fl_nil();}
  t_ascii(&out,"<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\" xmlns:content=\"http://purl.org/rss/1.0/modules/content/\">\n<channel>\n");
  vf_tag(&out,"title",vf_get(first,"title"));vf_tag(&out,"link",url);vf_tag(&out,"description",vf_default(vf_get(first,"description"),""));vf_tag(&out,"language",vf_default(vf_get(first,"language"),"en"));
  FLValue updated=vf_get(first,"updated");vf_tag(&out,"lastBuildDate",fl_v11_truthy(updated)?updated:vf_utc(fl_nil(),1));
  t_ascii(&out,"<atom:link href=\"");vf_escape(&out,url);FLString*u=(FLString*)url.obj;if(!u->len||u->data[u->len-1]!='/')t_ascii(&out,"/");t_ascii(&out,"feed.xml\" rel=\"self\" type=\"application/rss+xml\"/>\n");
  for(uint32_t i=0;list&&i<list->len;i++){
   FLValue item=list->data[i];t_ascii(&out,"<item>\n");vf_tag(&out,"title",vf_get(item,"title"));vf_tag(&out,"link",vf_get(item,"url"));t_ascii(&out,"<guid isPermaLink=\"true\">");vf_escape(&out,vf_get(item,"url"));t_ascii(&out,"</guid>\n");
   FLValue date=vf_get(item,"date");if(fl_v11_truthy(date))vf_tag(&out,"pubDate",vf_utc(date,0));
   FLValue author=vf_get(item,"author");if(fl_v11_truthy(author))vf_tag(&out,"author",author);
   FLValue description=vf_get(item,"description");if(fl_v11_truthy(description))vf_tag(&out,"description",description);
   FLValue content=vf_get(item,"content");if(fl_v11_truthy(content)){t_ascii(&out,"<content:encoded><![CDATA[");vf_cdata(&out,content);t_ascii(&out,"]]></content:encoded>\n");}
   t_ascii(&out,"</item>\n");
  }
  t_ascii(&out,"</channel>\n</rss>");
 }else{
  if(first.tag!=FL_STRING){free(out.p);fl_throw(fl_make_error("TypeError","sitemap_xml: base URL string required"));return fl_nil();}
  FLString*base=(FLString*)first.obj;size_t length=base->len;if(length&&base->data[length-1]=='/')length--;
  t_ascii(&out,"<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n");
  for(uint32_t i=0;list&&i<list->len;i++){
   FLValue item=list->data[i],path=item.tag==FL_STRING?item:vf_get(item,"loc");
   if(path.tag!=FL_STRING){free(out.p);fl_throw(fl_make_error("TypeError","sitemap_xml: route string required"));return fl_nil();}
   TBuffer loc={0};t_put(&loc,base->data,length);FLString*p=(FLString*)path.obj;if(!p->len||p->data[0]!='/')t_ascii(&loc,"/");t_put(&loc,p->data,p->len);
   t_ascii(&out,"<url>\n");vf_tag(&out,"loc",t_finish(&loc));
   if(item.tag!=FL_STRING){
    FLValue value=vf_get(item,"lastmod");if(fl_v11_truthy(value))vf_tag(&out,"lastmod",value);
    value=vf_get(item,"changefreq");if(fl_v11_truthy(value))vf_tag(&out,"changefreq",value);
    value=vf_get(item,"priority");if(t_numeric(value))vf_tag(&out,"priority",value);
   }
   t_ascii(&out,"</url>\n");
  }
  t_ascii(&out,"</urlset>");
 }
 return t_finish(&out);
}
