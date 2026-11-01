/* Real Chrome GC + snapshot proof. No injected JavaScript. */
#include "runtime.h"
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
static int e10_memory(const char *out){
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
 FLValue snapshot=fl_json_parse(fl_str_val(raw));free(raw);
 FLValue meta=field(field(snapshot,"snapshot"),"meta");
 FLValue fields=field(meta,"edge_fields"),edges=field(snapshot,"edges"),strings=field(snapshot,"strings");
 unsigned stride=e10_len(fields);if(!stride||e10_len(edges)%stride)die("snapshot_edge_stride");
 int type=e10_index(fields,"type"),name=e10_index(fields,"name_or_index");
 int property=e10_index(e10_at(field(meta,"edge_types"),(unsigned)type),"property");
 int marker=e10_index(strings,"E10_CANCEL_PROBE");long probes=0;
 for(unsigned i=0;i<e10_len(edges);i+=stride)
  if((int)number(e10_at(edges,i+(unsigned)type))==property&&(int)number(e10_at(edges,i+(unsigned)name))==marker)probes++;
 printf("E10_HEAP_USED_BEFORE_GC=%.0f\nE10_HEAP_USED_AFTER_GC=%.0f\n",number(field(before,"usedSize")),number(field(after,"usedSize")));
 printf("E10_CAPTURE_PROPERTY_EDGES=%ld\nE10_LIVE_CONTROL_EXPECTED=1\nE10_CANCEL_RETAINED_CAPTURES=%ld\n",probes,probes?probes-1:-1);
 printf("E10_CANCEL_MEMORY=%s\n",probes==1?"PASS":"FAIL");
 return probes==1;
}
