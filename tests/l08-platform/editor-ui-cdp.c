/* Real browser interaction through CDP. No Runtime.evaluate or injected JS. */
#include "runtime.h"
#include <curl/curl.h>
#include <curl/websockets.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>
static CURL *ws;
static int serial, exceptions, passed, failed, document_retries, document_root, draft_dialogs;
static const char *owned_base;
static const char *transport_prefix="EDITOR_UI";
static FLValue last_browser_exception;
static char *incoming;
static const size_t limit=16u*1024u*1024u;
static FLValue obj(void){return fl_map_new();}
static FLValue put(FLValue o,const char*k,FLValue v){return fl_map_set(o,fl_str_val(k),v);}
static FLValue field(FLValue o,const char*k){return fl_map_get(o,fl_str_val(k));}
static const char *text(FLValue v){return v.tag==FL_STRING?((FLString*)v.obj)->data:"";}
static double number(FLValue v){return v.tag==FL_INT?(double)v.i:v.tag==FL_FLOAT?v.f:0;}
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void die(const char *stage){fprintf(stderr,"%s_CDP_FAILURE=%s\n",transport_prefix,stage);exit(1);}
static void send_message(FLValue message){
 FLValue encoded=fl_json_stringify(message);FLString*s=(FLString*)encoded.obj;size_t sent=0;
 CURLcode rc=curl_ws_send(ws,s->data,s->len,&sent,0,CURLWS_TEXT);
 if(rc||sent!=s->len)die("send");
}
static FLValue command(const char *method,FLValue params){
 int id=++serial;
 send_message(put(put(put(obj(),"id",fl_int(id)),"method",fl_str_val(method)),"params",params));
 size_t used=0;double deadline=now()+15;
 while(now()<deadline){
  size_t count=0;const struct curl_ws_frame *frame=NULL;
  CURLcode rc=curl_ws_recv(ws,incoming+used,limit-used-1,&count,&frame);
  if(rc==CURLE_AGAIN){curl_socket_t fd;curl_easy_getinfo(ws,CURLINFO_ACTIVESOCKET,&fd);struct pollfd p={fd,POLLIN,0};poll(&p,1,20);continue;}
  if(rc||!frame||(frame->flags&CURLWS_CLOSE))die("receive");
  used+=count;if(used>=limit-1)die("message_limit");
  if(frame->bytesleft||(frame->flags&CURLWS_CONT))continue;
  incoming[used]=0;FLValue msg=fl_json_parse(fl_str_val(incoming));used=0;
  if(!strcmp(text(field(msg,"method")),"Runtime.exceptionThrown")){exceptions++;last_browser_exception=field(field(msg,"params"),"exceptionDetails");}
  if(!strcmp(text(field(msg,"method")),"DOM.documentUpdated"))document_root=0;
  if(!strcmp(text(field(msg,"method")),"Page.javascriptDialogOpening")){
   FLValue dialog=field(msg,"params");
   if(strcmp(text(field(dialog,"type")),"beforeunload")||strncmp(text(field(dialog,"url")),owned_base,strlen(owned_base)))die("unexpected_dialog");
   FLValue reply=put(put(put(obj(),"id",fl_int(++serial)),"method",fl_str_val("Page.handleJavaScriptDialog")),"params",put(obj(),"accept",fl_bool(true)));
   send_message(reply);draft_dialogs++;
  }
  if((int)number(field(msg,"id"))!=id)continue;
  if(field(msg,"error").tag!=FL_NIL){
   FLValue error=field(msg,"error");
   /* Navigation can invalidate the root between getDocument and querySelector. */
   if(!strcmp(method,"DOM.querySelector")&&number(field(error,"code"))==-32000){document_root=0;document_retries++;return fl_nil();}
   fprintf(stderr,"EDITOR_UI_CDP_METHOD=%s CODE=%.0f\n",method,number(field(error,"code")));
   if(!strncmp(method,"DOM.",4)||!strncmp(method,"CSS.",4))fprintf(stderr,"EDITOR_UI_CDP_MESSAGE=%s\n",text(field(error,"message")));
   die("protocol_error");
  }
  return field(msg,"result");
 }
 fprintf(stderr,"EDITOR_UI_CDP_METHOD=%s\n",method);die("response_timeout");return fl_nil();
}
static int node(const char *selector){
 if(!document_root){FLValue r=command("DOM.getDocument",obj());document_root=(int)number(field(field(r,"root"),"nodeId"));}
 FLValue r=command("DOM.querySelector",put(put(obj(),"nodeId",fl_int(document_root)),"selector",fl_str_val(selector)));
 return (int)number(field(r,"nodeId"));
}
static int wait_node(const char *selector){
 double deadline=now()+15;int found;
 do{found=node(selector);if(found)return found;poll(NULL,0,30);}while(now()<deadline);
 return 0;
}
static const char *style(int id,const char *property){
 FLValue r=command("CSS.getComputedStyleForNode",put(obj(),"nodeId",fl_int(id)));
 FLValue list=field(r,"computedStyle");if(list.tag!=FL_VECTOR)die("computed_style");
 FLVector*v=(FLVector*)list.obj;
 for(unsigned i=0;i<v->len;i++)if(!strcmp(text(field(v->data[i],"name")),property))return text(field(v->data[i],"value"));
 return "";
}
static const char *attribute(int id,const char *name){
 FLValue r=command("DOM.getAttributes",put(obj(),"nodeId",fl_int(id)));FLValue list=field(r,"attributes");
 if(list.tag!=FL_VECTOR)die("attributes");FLVector*v=(FLVector*)list.obj;
 for(unsigned i=0;i+1<v->len;i+=2)if(!strcmp(text(v->data[i]),name))return text(v->data[i+1]);
 return NULL;
}
static void click(int id){
 if(!id)die("missing_click_target");
 command("DOM.scrollIntoViewIfNeeded",put(obj(),"nodeId",fl_int(id)));
 FLValue r=command("DOM.getBoxModel",put(obj(),"nodeId",fl_int(id)));FLValue quad=field(field(r,"model"),"content");
 if(quad.tag!=FL_VECTOR||((FLVector*)quad.obj)->len!=8)die("box_model");FLVector*q=(FLVector*)quad.obj;
 double x=(number(q->data[0])+number(q->data[4]))/2,y=(number(q->data[1])+number(q->data[5]))/2;
 FLValue p=put(put(obj(),"x",fl_float(x)),"y",fl_float(y));
 command("Input.dispatchMouseEvent",put(p,"type",fl_str_val("mouseMoved")));
 p=put(put(p,"button",fl_str_val("left")),"clickCount",fl_int(1));
 command("Input.dispatchMouseEvent",put(p,"type",fl_str_val("mousePressed")));
 command("Input.dispatchMouseEvent",put(p,"type",fl_str_val("mouseReleased")));
}
static void check(int width,const char *label,int ok){printf("EDITOR_UI_%d_%s=%s\n",width,label,ok?"PASS":"FAIL");if(ok)passed++;else failed++;}
static void screenshot(const char *out,int width){
 FLValue p=put(put(obj(),"format",fl_str_val("png")),"captureBeyondViewport",fl_bool(false));
 FLValue r=command("Page.captureScreenshot",p);FLValue value=field(r,"data");if(value.tag!=FL_STRING)die("screenshot_data");
 FLString*s=(FLString*)value.obj;size_t cap=(s->len/4+1)*3;unsigned char*data=malloc(cap);if(!data)die("screenshot_allocation");
 int n=EVP_DecodeBlock(data,(const unsigned char*)s->data,s->len);if(n<0)die("screenshot_base64");
 if(s->len&&s->data[s->len-1]=='=')n--;if(s->len>1&&s->data[s->len-2]=='=')n--;
 char path[4096];if(snprintf(path,sizeof path,"%s/editor-image-%d.png",out,width)>=(int)sizeof path)die("screenshot_path");
 FILE*f=fopen(path,"wb");if(!f||fwrite(data,1,n,f)!=(size_t)n||fclose(f))die("screenshot_write");free(data);
 printf("EDITOR_UI_SCREENSHOT=%d\n",width);
}
int main(int argc,char**argv){
 if(argc!=4)return 2;fl_init_argv(argc,argv);
 if(strncmp(argv[1],"ws://127.0.0.1:",15)||strncmp(argv[2],"http://127.0.0.1:",17))die("loopback_required");
 owned_base=argv[2];
 const char *token=getenv("BLOG_TEST_SESSION_TOKEN");if(!token||strlen(token)!=64)die("synthetic_session_required");
 incoming=malloc(limit);if(!incoming)die("allocation");curl_global_init(CURL_GLOBAL_DEFAULT);ws=curl_easy_init();if(!ws)die("curl_init");
 curl_easy_setopt(ws,CURLOPT_URL,argv[1]);curl_easy_setopt(ws,CURLOPT_PROXY,"");curl_easy_setopt(ws,CURLOPT_CONNECT_ONLY,2L);curl_easy_setopt(ws,CURLOPT_TIMEOUT,15L);
 if(curl_easy_perform(ws))die("connect");
 command("Page.enable",obj());command("DOM.enable",obj());command("CSS.enable",obj());command("Runtime.enable",obj());command("Network.enable",obj());
 FLValue p=put(put(put(put(obj(),"name",fl_str_val("blog_session")),"value",fl_str_val(token)),"url",fl_str_val(argv[2])),"httpOnly",fl_bool(true));
 if(!fl_truthy(field(command("Network.setCookie",p),"success")))die("synthetic_cookie");
 int widths[]={375,768,1440},heights[]={812,1024,900};
 for(int i=0;i<3;i++){
  int w=widths[i];exceptions=0;
  p=put(put(put(put(obj(),"width",fl_int(w)),"height",fl_int(heights[i])),"deviceScaleFactor",fl_int(1)),"mobile",fl_bool(false));command("Emulation.setDeviceMetricsOverride",p);
  char url[4096];snprintf(url,sizeof url,"%s/admin/posts/new",argv[2]);command("Page.navigate",put(obj(),"url",fl_str_val(url)));document_root=0;
  int visual=wait_node("#editor-visual");if(!visual)die("editor_navigation");
  double ready=now()+10;
  while(!strcmp(style(visual,"display"),"none")&&now()<ready)poll(NULL,0,30);
  check(w,"VISUAL_READY",strcmp(style(visual,"display"),"none")!=0);
  int preview=wait_node("#editor-preview");check(w,"INITIAL_PREVIEW_HIDDEN",!strcmp(style(preview,"display"),"none"));
  click(wait_node(".editor-image-menu>summary"));check(w,"IMAGE_MENU_OPEN",attribute(wait_node(".editor-image-menu"),"open")!=NULL);
  click(wait_node("button[data-src='/images/66666666666666666666666666666666.png']"));
  int image=wait_node("#editor-visual img");check(w,"IMAGE_INSERTED",image!=0);
  check(w,"IMAGE_ALT",image&&attribute(image,"alt")&&!strcmp(attribute(image,"alt"),"사진 시험"));
  check(w,"INSERT_PREVIEW_HIDDEN",!strcmp(style(preview,"display"),"none"));
  click(wait_node("#editor-preview-toggle"));int rendered=wait_node("#editor-preview img");
  check(w,"PREVIEW_IMAGE_VISIBLE",rendered&&strcmp(style(preview,"display"),"none"));
  screenshot(argv[3],w);
  click(wait_node("#editor-preview-toggle"));
  check(w,"RETURN_IMAGE_VISIBLE",node("#editor-visual img")&&strcmp(style(visual,"display"),"none")&&!strcmp(style(preview,"display"),"none"));
  check(w,"BROWSER_EXCEPTION_FREE",exceptions==0);
 }
 printf("EDITOR_UI_PASS=%d FAIL=%d\n",passed,failed);
 printf("EDITOR_UI_DOCUMENT_RETRIES=%d\n",document_retries);
 printf("EDITOR_UI_OWN_DRAFT_DIALOGS=%d\n",draft_dialogs);
 puts("EDITOR_UI_AUTHENTICATION=SYNTHETIC_SESSION");puts("EDITOR_UI_PASSWORD_LOGIN=UNEXECUTED");puts("EDITOR_UI_EXISTING_JS=EXECUTED_UNCHANGED");puts("EDITOR_UI_INJECTED_JS=0");
 /* Request normal shutdown without depending on a reply after the socket closes. */
 send_message(put(put(obj(),"id",fl_int(++serial)),"method",fl_str_val("Browser.close")));
 curl_easy_cleanup(ws);curl_global_cleanup();free(incoming);return failed?1:0;
}
