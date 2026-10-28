/* Native compiler binding pass. Keep source names separate from C symbols. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct FLBinding {
    struct FLBinding *next;
    FLValue source, target;
    int cell;
} FLBinding;
typedef struct { unsigned serial; FLBinding *globals; } FLResolver;
static FLValue rb_get(FLValue v, const char *key) { return fl_map_get(v, fl_str_val(key)); }
static FLValue rb_set(FLValue v, const char *key, FLValue x) { return fl_map_set(v, fl_str_val(key), x); }
static int rb_is(FLValue v, const char *s) {
    return v.tag == FL_STRING && !strcmp(((FLString*)v.obj)->data,s);
}
static FLValue rb_lookup(FLBinding *env, FLValue name) {
    for (;env;env=env->next) if (fl_truthy(fl_eq(env->source,name))) return env->target;
    return fl_nil();
}
static FLValue rb_fresh(FLResolver *r) {
    char s[64]; snprintf(s,sizeof(s),"__fl_binding_%u",r->serial++); return fl_str_val(s);
}
static FLValue rb_bind(FLResolver *r, FLBinding **env, FLValue name) {
    FLBinding *b=malloc(sizeof(*b));
    if (!b) fl_throw(fl_str_val("compiler: binding allocation failed"));
    b->source=name; b->target=rb_fresh(r); b->cell=0; b->next=*env; *env=b; return b->target;
}
static void rb_pop(FLBinding **env, FLBinding *stop) {
    while (*env!=stop) { FLBinding *p=*env; *env=p->next; free(p); }
}
static FLValue rb_name(FLValue n) {
    return rb_get(n,rb_is(rb_get(n,"kind"),"variable")?"name":"value");
}
static FLValue rb_var(FLValue name) {
    return rb_set(rb_set(fl_map_new(),"kind",fl_str_val("variable")),"name",name);
}
static FLValue rb_call(const char *op, FLValue args) {
    return rb_set(rb_set(rb_set(fl_map_new(),"kind",fl_str_val("sexpr")),"op",fl_str_val(op)),"args",args);
}
static FLValue rb_vec(int n, FLValue *a) { return fl_vec_from(a,n); }
static FLValue rb_items(FLValue n) { return rb_get(rb_get(n,"fields"),"items"); }
static FLValue rb_with_items(FLValue n, FLValue items) {
    return rb_set(n,"fields",rb_set(rb_get(n,"fields"),"items",items));
}
static FLValue rb_registration(FLValue name,FLValue binding) {
    FLValue s=rb_set(rb_set(fl_map_new(),"kind",fl_str_val("literal")),"type",fl_str_val("string"));
    s=rb_set(s,"value",name);FLValue a[2]={s,rb_var(binding)};
    return rb_call("fl_register_named_fn",rb_vec(2,a));
}
static FLValue rb_node(FLResolver*,FLBinding**,FLValue);
static FLValue rb_read(FLBinding *env, FLValue name) {
    for(;env;env=env->next) if(fl_truthy(fl_eq(env->source,name))) {
        FLValue variable=rb_var(env->target);
        return env->cell?rb_call("deref",rb_vec(1,&variable)):variable;
    }
    return fl_nil();
}
static FLValue rb_sequence(FLResolver *r,FLBinding **env,FLValue args,uint32_t start) {
    FLBinding *save=*env;
    FLValue binds=fl_vec_new(),body=fl_vec_new();FLVector *v=(FLVector*)args.obj;
    for(uint32_t i=start;i<v->len;i++) {
        FLValue n=v->data[i],op=rb_get(n,"op");
        if(rb_is(op,"defn")||rb_is(op,"define")) {
            FLValue name=rb_name(fl_vec_get(rb_get(n,"args"),fl_int(0)));
            FLValue target=rb_bind(r,env,name);(*env)->cell=1;
            binds=fl_vec_push(binds,rb_var(target));
            FLValue nil=rb_set(rb_set(fl_map_new(),"kind",fl_str_val("literal")),"type",fl_str_val("nil"));
            binds=fl_vec_push(binds,rb_call("atom",rb_vec(1,&nil)));
        }
    }
    for(uint32_t i=start;i<v->len;i++) {
        FLValue n=v->data[i],op=rb_get(n,"op");
        if(rb_is(op,"defn")||rb_is(op,"define")) {
            FLValue a=rb_get(n,"args"),value;
            FLValue name=rb_name(fl_vec_get(a,fl_int(0)));
            if(rb_is(op,"defn")) {
                FLVector *av=(FLVector*)a.obj;FLValue fargs=fl_vec_new();
                for(uint32_t j=1;j<av->len;j++)fargs=fl_vec_push(fargs,av->data[j]);
                value=rb_node(r,env,rb_call("fn",fargs));
            } else value=rb_node(r,env,fl_vec_get(a,fl_int(1)));
            FLValue pair[2]={rb_var(rb_lookup(*env,name)),value};
            body=fl_vec_push(body,rb_call("reset!",rb_vec(2,pair)));
        } else body=fl_vec_push(body,rb_node(r,env,n));
    }
    FLValue result=rb_call("do",body);
    if(((FLVector*)binds.obj)->len) {
        FLValue block=rb_set(rb_set(fl_map_new(),"kind",fl_str_val("block")),"type",fl_str_val("Array"));
        FLValue pair[2]={rb_with_items(block,binds),result};result=rb_call("let",rb_vec(2,pair));
    }
    rb_pop(env,save);return result;
}
static FLValue rb_list(FLResolver *r,FLBinding **env,FLValue list) {
    if (list.tag!=FL_VECTOR) return list;
    FLValue out=fl_vec_new(); FLVector *v=(FLVector*)list.obj;
    for (uint32_t i=0;i<v->len;i++) out=fl_vec_push(out,rb_node(r,env,v->data[i]));
    return out;
}
static FLValue rb_params(FLResolver *r,FLBinding **env,FLValue block) {
    FLValue out=fl_vec_new(); FLVector *v=(FLVector*)rb_items(block).obj;
    if (!v) return block;
    for (uint32_t i=0;i<v->len;i++) out=fl_vec_push(out,rb_var(rb_bind(r,env,rb_name(v->data[i]))));
    return rb_with_items(block,out);
}
static FLValue rb_bindings(FLResolver *r,FLBinding **env,FLValue block) {
    FLValue out=fl_vec_new(),items=rb_items(block);
    if (items.tag!=FL_VECTOR) return block;
    FLVector *v=(FLVector*)items.obj;
    int nested=v->len && rb_is(rb_get(v->data[0],"kind"),"block");
    for (uint32_t i=0;i<v->len;i+=nested?1:2) {
        FLValue pair=nested?rb_items(v->data[i]):items;
        uint32_t j=nested?0:i;
        FLValue init=rb_node(r,env,fl_vec_get(pair,fl_int(j+1)));
        FLValue name=rb_bind(r,env,rb_name(fl_vec_get(pair,fl_int(j))));
        FLValue a[2]={rb_var(name),init};
        if (nested) out=fl_vec_push(out,rb_with_items(v->data[i],rb_vec(2,a)));
        else {out=fl_vec_push(out,a[0]);out=fl_vec_push(out,a[1]);}
    }
    return rb_with_items(block,out);
}
static FLValue rb_node(FLResolver *r,FLBinding **env,FLValue n) {
    if (n.tag!=FL_MAP) return n;
    FLValue kind=rb_get(n,"kind");
    if (rb_is(kind,"variable") || (rb_is(kind,"literal")&&rb_is(rb_get(n,"type"),"symbol"))) {
        FLValue read=rb_read(*env,rb_name(n));
        return read.tag==FL_NIL?n:rb_set(read,"line",rb_get(n,"line"));
    }
    if (rb_is(kind,"block")) return rb_with_items(n,rb_list(r,env,rb_items(n)));
    if (rb_is(kind,"sexpr")) {
        FLValue op=rb_get(n,"op"),args=rb_get(n,"args"),out=fl_vec_new();
        FLBinding *save=*env;
        if (rb_is(op,"fn")||rb_is(op,"defn")) {
            int offset=rb_is(op,"defn")?1:0;
            if(offset) out=fl_vec_push(out,fl_vec_get(args,fl_int(0)));
            out=fl_vec_push(out,rb_params(r,env,fl_vec_get(args,fl_int(offset))));
            out=fl_vec_push(out,rb_sequence(r,env,args,offset+1));
            rb_pop(env,save); return rb_set(n,"args",out);
        }
        if(rb_is(op,"do")) return rb_sequence(r,env,args,0);
        if(rb_is(op,"let")||rb_is(op,"loop")||rb_is(op,"doseq")||rb_is(op,"if-let")||rb_is(op,"when-let")) {
            out=fl_vec_push(out,rb_bindings(r,env,fl_vec_get(args,fl_int(0))));
            FLVector *v=(FLVector*)args.obj;
            for(uint32_t i=1;i<v->len;i++) out=fl_vec_push(out,rb_node(r,env,v->data[i]));
            rb_pop(env,save); return rb_set(n,"args",out);
        }
        if(rb_is(op,"catch")) {
            FLValue param=rb_bind(r,env,rb_name(fl_vec_get(args,fl_int(0))));
            out=fl_vec_push(out,rb_var(param)); FLVector *v=(FLVector*)args.obj;
            for(uint32_t i=1;i<v->len;i++) out=fl_vec_push(out,rb_node(r,env,v->data[i]));
            rb_pop(env,save); return rb_set(n,"args",out);
        }
        if(rb_is(op,"define")) {
            FLValue original=rb_name(fl_vec_get(args,fl_int(0)));
            FLValue mapped=rb_lookup(r->globals,original);
            if(mapped.tag==FL_NIL) mapped=rb_bind(r,env,original);
            FLValue a[2]={rb_var(mapped),rb_node(r,env,fl_vec_get(args,fl_int(1)))};
            return rb_set(n,"args",rb_vec(2,a));
        }
        FLValue key=op;
        if(key.tag==FL_STRING && ((FLString*)key.obj)->data[0]=='$') key=fl_str_val(((FLString*)key.obj)->data+1);
        FLValue read=rb_read(*env,key);
        out=rb_list(r,env,args);
        if(read.tag!=FL_NIL) {
            if(rb_is(rb_get(read,"kind"),"variable")) op=fl_str_n(2,fl_str_val("$"),rb_get(read,"name"));
            else {
                FLValue all=fl_vec_push(fl_vec_new(),read);FLVector *v=(FLVector*)out.obj;
                for(uint32_t i=0;i<v->len;i++)all=fl_vec_push(all,v->data[i]);
                op=fl_str_val("__apply__");out=all;
            }
        }
        return rb_set(rb_set(n,"op",op),"args",out);
    }
    /* try/throw/and/or and other parser records contain child nodes/lists. */
    FLMap *m=(FLMap*)n.obj; FLValue out=n;
    for(uint32_t i=0;i<m->len;i++) {
        FLValue value=m->entries[i].val;
        if(value.tag==FL_MAP) value=rb_node(r,env,value);
        else if(value.tag==FL_VECTOR) value=rb_list(r,env,value);
        out=fl_map_set(out,m->entries[i].key,value);
    }
    return out;
}
FLValue fl_compiler_bindings(FLValue nodes) {
    FLResolver r={0}; FLVector *v=(FLVector*)nodes.obj;
    for(uint32_t i=0;i<v->len;i++) {
        FLValue n=v->data[i],op=rb_get(n,"op");
        if(rb_is(op,"defn")||rb_is(op,"define")) {
            FLValue name=rb_name(fl_vec_get(rb_get(n,"args"),fl_int(0)));
            if(rb_lookup(r.globals,name).tag==FL_NIL) rb_bind(&r,&r.globals,name);
        }
    }
    FLBinding *env=r.globals; FLValue out=fl_vec_new();
    for(uint32_t i=0;i<v->len;i++) {
        FLValue n=v->data[i];
        if(rb_is(rb_get(n,"op"),"defn")) {
            FLValue args=rb_get(n,"args"),name=rb_name(fl_vec_get(args,fl_int(0)));
            FLValue implementation=rb_fresh(&r),a=fl_vec_new(); FLVector *av=(FLVector*)args.obj;
            a=fl_vec_push(a,rb_var(implementation));
            for(uint32_t j=1;j<av->len;j++) a=fl_vec_push(a,av->data[j]);
            out=fl_vec_push(out,rb_node(&r,&env,rb_set(n,"args",a)));
            FLValue binding=rb_lookup(r.globals,name),d[2]={rb_var(binding),rb_var(implementation)};
            out=fl_vec_push(out,rb_call("define",rb_vec(2,d)));
            out=fl_vec_push(out,rb_registration(name,binding));
        } else {
            out=fl_vec_push(out,rb_node(&r,&env,n));
            if(rb_is(rb_get(n,"op"),"define")) {
                FLValue name=rb_name(fl_vec_get(rb_get(n,"args"),fl_int(0)));
                out=fl_vec_push(out,rb_registration(name,rb_lookup(r.globals,name)));
            }
        }
    }
    rb_pop(&env,NULL); return out;
}
FLValue fl_compiler_realpath(FLValue path) {
    if(path.tag!=FL_STRING) return fl_nil();
    char *p=realpath(((FLString*)path.obj)->data,NULL);
    if(!p) return fl_nil(); FLValue result=fl_str_val(p); free(p); return result;
}
