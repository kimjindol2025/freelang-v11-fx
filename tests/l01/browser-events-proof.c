/* Native CDP interactions only; no evaluated or injected JS. */
#define main editor_ui_unused_main
#include "../l08-platform/editor-ui-cdp.c"
#undef main
static const char *node_text(const char *selector){
 int id=node(selector);if(!id)return "";
 FLValue r=command("DOM.describeNode",put(put(obj(),"nodeId",fl_int(id)),"depth",fl_int(2)));
 FLValue children=field(field(r,"node"),"children");
 if(children.tag==FL_VECTOR){FLVector*v=(FLVector*)children.obj;
  for(unsigned i=0;i<v->len;i++)if(number(field(v->data[i],"nodeType"))==3)return text(field(v->data[i],"nodeValue"));
 }return "";
}
static void wait_text(const char *selector,const char *expected){
 double deadline=now()+15;
 while(now()<deadline){if(!strcmp(node_text(selector),expected))return;if(exceptions)break;poll(NULL,0,30);}
 die("expected_dom_text");
}
int main(int argc,char**argv){
 if(argc!=4)return 2;fl_init_argv(argc,argv);transport_prefix="L01_EVENTS";
 if(strncmp(argv[1],"ws://127.0.0.1:",15)||strncmp(argv[2],"http://127.0.0.1:",17))die("loopback_required");
 owned_base=argv[2];incoming=malloc(limit);if(!incoming)die("allocation");
 curl_global_init(CURL_GLOBAL_DEFAULT);ws=curl_easy_init();if(!ws)die("curl_init");
 curl_easy_setopt(ws,CURLOPT_URL,argv[1]);curl_easy_setopt(ws,CURLOPT_PROXY,"");curl_easy_setopt(ws,CURLOPT_CONNECT_ONLY,2L);curl_easy_setopt(ws,CURLOPT_TIMEOUT,15L);
 if(curl_easy_perform(ws))die("connect");
 command("Page.enable",obj());command("DOM.enable",obj());command("Runtime.enable",obj());
 command("Emulation.setDeviceMetricsOverride",put(put(put(put(obj(),"width",fl_int(768)),"height",fl_int(1024)),"deviceScaleFactor",fl_int(1)),"mobile",fl_bool(false)));
 command("Page.navigate",put(obj(),"url",fl_str_val(argv[2])));document_root=0;
 wait_text("#ready","READY");
 int focused=node("#editor:focus")!=0;
 command("Input.insertText",put(obj(),"text",fl_str_val("가😀<b>")));
 FLValue key=put(put(put(put(obj(),"key",fl_str_val("Enter")),"code",fl_str_val("Enter")),"windowsVirtualKeyCode",fl_int(13)),"nativeVirtualKeyCode",fl_int(13));
 command("Input.dispatchKeyEvent",put(put(key,"type",fl_str_val("keyDown")),"text",fl_str_val("\r")));
 command("Input.dispatchKeyEvent",put(key,"type",fl_str_val("keyUp")));
 click(wait_node("#commit"));wait_text("#ready","CLICK_AGAIN");click(wait_node("#commit"));
 const char *result=node_text("#proof");int literal_safe=node("#literal img")==0;
 if(strcmp(result,"WAIT")&&*result)fputs(result,stdout);else die("proof_missing");
 printf("J_HOST_NO_HTML=%s\n",literal_safe?"PASS":"FAIL");
 printf("J_HOST_FOCUS_TARGET=%s\n",focused?"PASS":"FAIL");
 printf("L01_BROWSER_RUNTIME_EXIT=%d\n",exceptions||!literal_safe||!focused?1:0);
 puts("L01_BROWSER_CODE=COMPILER_GENERATED");puts("L01_BROWSER_INJECTED_JS=0");
 screenshot(argv[3],768);
 send_message(put(put(obj(),"id",fl_int(++serial)),"method",fl_str_val("Browser.close")));
 curl_easy_cleanup(ws);curl_global_cleanup();free(incoming);return exceptions||!literal_safe||!focused?1:0;
}
