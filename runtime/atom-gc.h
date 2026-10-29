/* Arena boundaries collect shared atom cycles; active owners remain RC roots. */
typedef struct AtomGCNode {
    FLValue value;
    size_t incoming;
    int live;
    struct AtomGCNode *next;
} AtomGCNode;
typedef struct {
    AtomGCNode *buckets[4096],**nodes;
    size_t count,capacity;
    int failed;
} AtomGCGraph;
static size_t atom_gc_bucket(FLObject *object){return ((uintptr_t)object>>4)&4095;}
static AtomGCNode *atom_gc_find(AtomGCGraph *graph,FLValue value){
    if(value.tag<FL_STRING||!value.obj)return NULL;
    for(AtomGCNode *n=graph->buckets[atom_gc_bucket(value.obj)];n;n=n->next)
        if(n->value.obj==value.obj)return n;
    return NULL;
}
static int atom_gc_owned(FLValue value){
    if(value.tag<FL_STRING||!value.obj)return 0;
    if(value.obj->type==FL_MANAGED_ATOM||value.obj->type==FL_OWNED_FN)return 1;
    /* Promise/future managers retain their results as external owners. */
    if(value.obj->type==FL_NATIVE_PROMISE)return 0;
    return (__atomic_load_n(&value.obj->rc,__ATOMIC_RELAXED)&FL_HEAP_RC)!=0;
}
static AtomGCNode *atom_gc_add(AtomGCGraph *graph,FLValue value){
    if(graph->failed||!atom_gc_owned(value))return NULL;
    AtomGCNode *node=atom_gc_find(graph,value);if(node)return node;
    if(graph->count==graph->capacity){
        size_t capacity=graph->capacity?graph->capacity*2:64;
        AtomGCNode **nodes=realloc(graph->nodes,capacity*sizeof(*nodes));
        if(!nodes){graph->failed=1;return NULL;}
        graph->nodes=nodes;graph->capacity=capacity;
    }
    node=calloc(1,sizeof(*node));if(!node){graph->failed=1;return NULL;}
    node->value=value;size_t bucket=atom_gc_bucket(value.obj);
    node->next=graph->buckets[bucket];graph->buckets[bucket]=node;
    graph->nodes[graph->count++]=node;return node;
}
typedef void (*AtomGCVisit)(FLValue,void*);
static void atom_gc_children(FLValue value,AtomGCVisit visit,void *context){
    if(value.tag==FL_FN&&value.obj->type==FL_OWNED_FN){
        FLClosure *fn=(FLClosure*)value.obj;
        for(uint32_t i=0;i<fn->nenv;i++)visit(fn->env[i],context);
    }else if(value.tag==FL_VECTOR){
        FLVector *v=(FLVector*)value.obj;
        for(uint32_t i=0;i<v->len;i++)visit(v->data[i],context);
    }else if(value.tag==FL_MAP){
        FLMap *map=(FLMap*)value.obj;
        for(uint32_t i=0;i<map->len;i++){visit(map->entries[i].key,context);visit(map->entries[i].val,context);}
    }
}
static void atom_gc_edge(FLValue child,void *context){
    AtomGCNode *node=atom_gc_add(context,child);if(node)node->incoming++;
}
typedef struct {AtomGCGraph *graph;AtomGCNode **queue;size_t count;} AtomGCMark;
static void atom_gc_mark(FLValue value,void *context){
    AtomGCMark *mark=context;AtomGCNode *node=atom_gc_find(mark->graph,value);
    if(node&&!node->live){node->live=1;mark->queue[mark->count++]=node;}
}
static void atom_gc_drop_edge(FLValue value,void *context){
    AtomGCNode *node=atom_gc_find(context,value);
    if(!node||node->live)fl_heap_release(value);
}
static void atom_gc_free_storage(FLValue value){
    if(value.obj->type==FL_MANAGED_ATOM){
        FLAtom *atom=(FLAtom*)value.obj;
        if(pthread_mutex_destroy(&atom->mutex)){fputs("atom GC encountered a locked object\n",stderr);abort();}
        free(atom->vector.data);free(atom);
    }else if(value.tag==FL_VECTOR){free(((FLVector*)value.obj)->data);free(value.obj);}
    else if(value.tag==FL_MAP){free(((FLMap*)value.obj)->entries);free(value.obj);}
    else free(value.obj);
}
static void atom_gc_free_graph(AtomGCGraph *graph){
    for(size_t i=0;i<graph->count;i++)free(graph->nodes[i]);
    free(graph->nodes);free(graph);
}
void fl_gc_collect_atoms(void){
    fl_gc_lock();
    if(!gc_pending){fl_gc_unlock();return;}
    AtomGCGraph *graph=calloc(1,sizeof(*graph));
    if(!graph){fl_gc_unlock();return;}
    for(FLAtom *atom=gc_atoms;atom;atom=atom->gc_next)
        if(atom->gc_candidate)atom_gc_add(graph,(FLValue){.tag=FL_VECTOR,.obj=(FLObject*)atom});
    for(size_t i=0;i<graph->count&&!graph->failed;i++)atom_gc_children(graph->nodes[i]->value,atom_gc_edge,graph);
    AtomGCNode **queue=graph->count?malloc(graph->count*sizeof(*queue)):NULL;
    if(graph->failed||(graph->count&&!queue)){free(queue);atom_gc_free_graph(graph);fl_gc_unlock();return;}
    AtomGCMark mark={graph,queue,0};int imbalance=0;
    for(size_t i=0;i<graph->count;i++){
        AtomGCNode *node=graph->nodes[i];FLObject *object=node->value.obj;
        uint32_t refs=__atomic_load_n(&object->rc,__ATOMIC_RELAXED);
        if(object->type!=FL_MANAGED_ATOM&&object->type!=FL_OWNED_FN)refs&=~FL_HEAP_RC;
        if(refs<node->incoming)imbalance=1;
        if(refs>node->incoming)atom_gc_mark(node->value,&mark);
    }
    if(imbalance){fputs("atom GC reference imbalance\n",stderr);free(queue);atom_gc_free_graph(graph);fl_gc_unlock();return;}
    for(size_t i=0;i<mark.count;i++)atom_gc_children(mark.queue[i]->value,atom_gc_mark,&mark);
    gc_pending=0;
    for(size_t i=0;i<graph->count;i++){
        AtomGCNode *node=graph->nodes[i];
        if(node->value.obj->type==FL_MANAGED_ATOM){
            FLAtom *atom=(FLAtom*)node->value.obj;atom->gc_candidate=0;
            if(!node->live)gc_untrack_atom(atom);
        }
        if(!node->live)__atomic_store_n(&node->value.obj->rc,0,__ATOMIC_RELAXED);
    }
    for(size_t i=0;i<graph->count;i++)if(!graph->nodes[i]->live)atom_gc_children(graph->nodes[i]->value,atom_gc_drop_edge,graph);
    for(size_t i=0;i<graph->count;i++)if(!graph->nodes[i]->live)atom_gc_free_storage(graph->nodes[i]->value);
    free(queue);atom_gc_free_graph(graph);fl_gc_unlock();
}
