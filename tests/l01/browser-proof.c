/* Reuse the native CDP transport. No injected JS or script evaluation. */
#define main editor_ui_unused_main
#include "../l08-platform/editor-ui-cdp.c"
#undef main
int main(int argc,char**argv){
 if(argc!=3)return 2;fl_init_argv(argc,argv);transport_prefix="L01";
 if(strncmp(argv[1],"ws://127.0.0.1:",15)||strncmp(argv[2],"http://127.0.0.1:",17))die("loopback_required");
 owned_base=argv[2];incoming=malloc(limit);if(!incoming)die("allocation");
 curl_global_init(CURL_GLOBAL_DEFAULT);ws=curl_easy_init();if(!ws)die("curl_init");
 curl_easy_setopt(ws,CURLOPT_URL,argv[1]);curl_easy_setopt(ws,CURLOPT_PROXY,"");curl_easy_setopt(ws,CURLOPT_CONNECT_ONLY,2L);curl_easy_setopt(ws,CURLOPT_TIMEOUT,15L);
 if(curl_easy_perform(ws))die("connect");
 command("Page.enable",obj());command("DOM.enable",obj());command("Runtime.enable",obj());
 command("Page.navigate",put(obj(),"url",fl_str_val(argv[2])));document_root=0;
 const char *result="WAIT";double deadline=now()+20;
 do{
  int target=node("#proof");
  if(target){
   FLValue response=command("DOM.describeNode",put(put(obj(),"nodeId",fl_int(target)),"depth",fl_int(2)));
   FLValue children=field(field(response,"node"),"children");
   if(children.tag==FL_VECTOR){FLVector*v=(FLVector*)children.obj;
    for(unsigned i=0;i<v->len;i++)if(number(field(v->data[i],"nodeType"))==3)result=text(field(v->data[i],"nodeValue"));
   }
  }
  if(exceptions||strcmp(result,"WAIT"))break;
  poll(NULL,0,30);
 }while(now()<deadline);
 if(exceptions){
  fprintf(stderr,"L01_BROWSER_EXCEPTIONS=%d\n",exceptions);
  fprintf(stderr,"L01_BROWSER_EXCEPTION_TEXT=%s\n",text(field(last_browser_exception,"text")));
  FLValue exception=field(last_browser_exception,"exception");
  fprintf(stderr,"L01_BROWSER_EXCEPTION_DESCRIPTION=%s\n",text(field(exception,"description")));
  fprintf(stderr,"L01_BROWSER_EXCEPTION_VALUE=%s\n",text(field(exception,"value")));
 }
 if(!exceptions&&strcmp(result,"WAIT")){fputs(result,stdout);if(*result&&result[strlen(result)-1]!='\n')putchar('\n');}
 int ok=!exceptions&&strcmp(result,"WAIT")!=0;
 printf("L01_BROWSER_RUNTIME_EXIT=%d\n",ok?0:1);
 puts("L01_BROWSER_CODE=COMPILER_GENERATED");puts("L01_BROWSER_INJECTED_JS=0");
 send_message(put(put(obj(),"id",fl_int(++serial)),"method",fl_str_val("Browser.close")));
 curl_easy_cleanup(ws);curl_global_cleanup();free(incoming);return ok?0:1;
}
