/* Real Chrome GC + snapshot proof. No injected JavaScript. */
#include "runtime.h"
#include <ctype.h>
static void e10_event(FLValue message);
static int e10_memory(const char *out);
#define L01_CDP_EVENT(message) e10_event(message)
#define L01_PROOF_EXTRA() e10_memory(argv[3])
#include "browser-proof.c"
static FILE *e10_snapshot;
static void e10_event(FLValue message){
 if(e10_snapshot&&!strcmp(text(field(message,"method")),"HeapProfiler.addHeapSnapshotChunk")){
  FLValue chunk=field(field(message,"params"),"chunk");
  if(chunk.tag!=FL_STRING)die("snapshot_chunk_type");
  FLString*s=(FLString*)chunk.obj;
  if(fwrite(s->data,1,s->len,e10_snapshot)!=s->len)die("snapshot_write");
 }
}
static FLValue e10_at(FLValue list,unsigned i){
 if(list.tag!=FL_VECTOR||i>=((FLVector*)list.obj)->len)die("snapshot_array_bounds");
 return ((FLVector*)list.obj)->data[i];
}
static unsigned e10_len(FLValue list){if(list.tag!=FL_VECTOR)die("snapshot_array_type");return ((FLVector*)list.obj)->len;}
static int e10_index(FLValue list,const char *wanted){
 for(unsigned i=0;i<e10_len(list);i++)if(!strcmp(text(e10_at(list,i)),wanted))return (int)i;
 die("snapshot_metadata_missing");return -1;
}
static const char *e10_ws(const char *p){while(isspace((unsigned char)*p))p++;return p;}
static const char *e10_string_end(const char *p){
 if(*p++!='"')die("snapshot_string");
 while(*p){if(*p=='\\'){p++;if(!*p)die("snapshot_escape");p++;}else if(*p++=='"')return p;}
 die("snapshot_string_end");return p;
}
static const char *e10_value_end(const char *p){
 p=e10_ws(p);if(*p=='"')return e10_string_end(p);
 if(*p=='['||*p=='{'){
  int depth=0;
  do{if(*p=='"'){p=e10_string_end(p);continue;}if(*p=='['||*p=='{')depth++;if(*p==']'||*p=='}')depth--;p++;}while(*p&&depth);
  if(depth)die("snapshot_value_end");return p;
 }
 const char *start=p;while(*p&&*p!=','&&*p!=']'&&*p!='}'&&!isspace((unsigned char)*p))p++;
 if(p==start)die("snapshot_value");return p;
}
static int e10_string_is(const char *p,const char *end,const char *s){
 size_t n=strlen(s);return end-p==(long)n+2&&!memcmp(p+1,s,n);
}
static const char *e10_member(const char *raw,const char *key,const char **end){
 const char *p=e10_ws(raw);if(*p++!='{')die("snapshot_object");
 while(*(p=e10_ws(p))!='}'){
  const char *name=p,*name_end=e10_string_end(p);p=e10_ws(name_end);if(*p++!=':')die("snapshot_colon");
  const char *value=e10_ws(p),*value_end=e10_value_end(value);
  if(e10_string_is(name,name_end,key)){*end=value_end;return value;}
  p=e10_ws(value_end);if(*p==',')p++;else if(*p!='}')die("snapshot_separator");
 }
 die("snapshot_member_missing");return NULL;
}
static int e10_memory(const char *out){
 fflush(stdout);
 FLValue before=command("Runtime.getHeapUsage",obj());
 command("HeapProfiler.enable",obj());
 command("HeapProfiler.collectGarbage",obj());
 FLValue after=command("Runtime.getHeapUsage",obj());
 char path[4096];if(snprintf(path,sizeof path,"%s/cancel.heapsnapshot",out)>=(int)sizeof path)die("snapshot_path_length");
 e10_snapshot=fopen(path,"wb");if(!e10_snapshot)die("snapshot_open");
 command("HeapProfiler.takeHeapSnapshot",obj());
 if(fclose(e10_snapshot))die("snapshot_close");e10_snapshot=NULL;
 FILE *input=fopen(path,"rb");if(!input)die("snapshot_read_open");
 if(fseek(input,0,SEEK_END))die("snapshot_seek");long size=ftell(input);if(size<=0)die("snapshot_size");
 rewind(input);char *raw=malloc((size_t)size+1);if(!raw)die("snapshot_allocate");
 if(fread(raw,1,(size_t)size,input)!=(size_t)size)die("snapshot_read");raw[size]=0;fclose(input);
 const char *end,*value=e10_member(raw,"snapshot",&end);size_t n=(size_t)(end-value);
 char *small=malloc(n+1);if(!small)die("snapshot_metadata_allocate");memcpy(small,value,n);small[n]=0;
 FLValue meta=field(fl_json_parse(fl_str_val(small)),"meta");free(small);
 FLValue fields=field(meta,"edge_fields");
 unsigned stride=e10_len(fields);if(!stride)die("snapshot_edge_stride");
 int type=e10_index(fields,"type"),name=e10_index(fields,"name_or_index");
 int property=e10_index(e10_at(field(meta,"edge_types"),(unsigned)type),"property");
 const char *cursor=e10_member(raw,"strings",&end);if(*cursor++!='[')die("snapshot_strings");
 long marker=-1,index=0;
 while(*(cursor=e10_ws(cursor))!=']'){
  const char *next=e10_string_end(cursor);
  if(e10_string_is(cursor,next,"E10_CANCEL_PROBE")){if(marker>=0)die("snapshot_marker_duplicate");marker=index;}
  index++;cursor=e10_ws(next);if(*cursor==',')cursor++;else if(*cursor!=']')die("snapshot_strings_separator");
 }
 if(marker<0)die("snapshot_marker_missing");
 cursor=e10_member(raw,"edges",&end);if(*cursor++!='[')die("snapshot_edges");
 unsigned slot=0;unsigned long edge_type=0,edge_name=0;long probes=0;
 while(*(cursor=e10_ws(cursor))!=']'){
  if(!isdigit((unsigned char)*cursor))die("snapshot_edge_number");
  char *next;unsigned long item=strtoul(cursor,&next,10);if(next==cursor)die("snapshot_edge_number");
  if(slot==(unsigned)type)edge_type=item;if(slot==(unsigned)name)edge_name=item;
  if(++slot==stride){if(edge_type==(unsigned long)property&&edge_name==(unsigned long)marker)probes++;slot=0;}
  cursor=e10_ws(next);if(*cursor==',')cursor++;else if(*cursor!=']')die("snapshot_edges_separator");
 }
 if(slot)die("snapshot_edge_stride");free(raw);
 printf("E10_HEAP_USED_BEFORE_GC=%.0f\nE10_HEAP_USED_AFTER_GC=%.0f\n",number(field(before,"usedSize")),number(field(after,"usedSize")));
 printf("E10_CAPTURE_PROPERTY_EDGES=%ld\nE10_LIVE_CONTROL_EXPECTED=1\nE10_CANCEL_RETAINED_CAPTURES=%ld\n",probes,probes?probes-1:-1);
 printf("E10_CANCEL_MEMORY=%s\n",probes==1?"PASS":"FAIL");
 return probes==1;
}
