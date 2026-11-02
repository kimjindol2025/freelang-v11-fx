/* C-owned, prioritized backtracking VM for the blog1 RegExp subset (UTF-16).
* No POSIX translation and no external regex library. */
#include <ctype.h>
#define RX_MAX 4096
#define RX_GROUPS 32
#define RX_LOOPS 64
#define RX_BUDGET 10000000
/* AST and bytecode share kinds; SEQ/ALT/REP/GROUP/LOOK compile to instructions. */
enum {
    RX_EMPTY,RX_CHAR,RX_DOT,RX_CLASS,RX_BOL,RX_EOL,RX_BOUNDARY,RX_SEQ,RX_ALT,RX_REP,RX_GROUP,RX_LOOK,RX_MATCH,RX_SPLIT,RX_SAVE,RX_LOOP
};
typedef struct {
    int lo,hi;
}
RxRange;
typedef struct {
    int kind,a,b,min,max,lazy,neg,group,nrange;
    RxRange range[32];
    unsigned char *folded;
}
RxNode;
typedef struct {
    int kind,a,b,arg,neg;
}
RxInst;
typedef struct {
    TUnits pattern,input;
    size_t at;
    RxNode *nodes;
    RxInst *code;
    int nn,nc,groups,loops,ignore,entry,error;
    unsigned steps;
    const char*why;
}
Rx;
typedef struct {
    int pc,pos,caps[RX_GROUPS*2],last[RX_LOOPS];
}
RxState;
static void rx_bad(Rx*r,const char*why){
    if(r->error)return;
    r->error=1;
    r->why=why;
}

static int rx_node(Rx*r,int kind,int a,int b){
    if(r->nn>=RX_MAX){
        rx_bad(r,"pattern limit");
        return 0;
    }
    int n=r->nn++;
    r->nodes[n]=(RxNode){
        .kind=kind,.a=a,.b=b
    };
    return n;
}

static int rx_inst(Rx*r,int kind,int a,int b,int arg,int neg){
    if(r->nc>=RX_MAX){
        rx_bad(r,"instruction limit");
        return 0;
    }
    int n=r->nc++;
    r->code[n]=(RxInst){
        kind,a,b,arg,neg
    };
    return n;
}

static int rx_peek(Rx*r){
    return r->at<r->pattern.n?r->pattern.p[r->at]:-1;
}

static int rx_take(Rx*r){
    int c=rx_peek(r);
    if(c>=0)r->at++;
    return c;
}

static void rx_range(Rx*r,int n,int lo,int hi){
    RxNode*p=r->nodes+n;
    if(p->nrange==32||lo>hi){
        rx_bad(r,"invalid character range");
        return;
    }
    p->range[p->nrange++]=(RxRange){
        lo,hi
    };
}

static int rx_escape(Rx*r,int n){
    int c=rx_take(r);
    if(c<0){
        rx_bad(r,"trailing escape");
        return 0;
    }
    if(c=='d'||c=='w'||c=='s'||c=='D'||c=='W'||c=='S'){
        RxNode*p=r->nodes+n;
        p->kind=RX_CLASS;
        p->neg=c=='D'||c=='W'||c=='S';
        c=tolower(c);
        if(c=='d'||c=='w'){
            rx_range(r,n,'0','9');
            if(c=='w'){
                rx_range(r,n,'A','Z');
                rx_range(r,n,'a','z');
                rx_range(r,n,'_','_');
            }
        }
        else{
            rx_range(r,n,9,13);
            rx_range(r,n,32,32);
            rx_range(r,n,160,160);
            rx_range(r,n,5760,5760);
            rx_range(r,n,8192,8202);
            rx_range(r,n,8232,8233);
            rx_range(r,n,8239,8239);
            rx_range(r,n,8287,8287);
            rx_range(r,n,12288,12288);
            rx_range(r,n,65279,65279);
        }
        return -1;
    }
    if(c>='1'&&c<='9'){
        rx_bad(r,"backreference is outside the blog1 subset");
        return 0;
    }
    if(c=='x'||c=='u'){
        int count=c=='x'?2:4,value=0;
        for(int i=0;i<count;i++){
            int h=rx_take(r),v=t_hex(h);
            if(v<0){
                rx_bad(r,"invalid escape");
                return 0;
            }
            value=value*16+v;
        }
        return value;
    }
    return c=='n'?10:c=='r'?13:c=='t'?9:c=='v'?11:c=='f'?12:c=='0'?0:c=='b'?8:c;
}

static int rx_expr(Rx*,int);
static int rx_atom(Rx*r,int depth){
    if(depth>64){
        rx_bad(r,"nesting limit");
        return 0;
    }
    int c=rx_take(r),n=rx_node(r,RX_EMPTY,0,0);
    if(r->error)return n;
    if(c=='('){
        int group=0,look=0,negative=0;
        if(rx_peek(r)=='?'){
            rx_take(r);
            int modifier=rx_take(r);
            if(modifier=='=')look=1;
            else if(modifier=='!'){
                look=1;
                negative=1;
            }
            else if(modifier!=':'){
                rx_bad(r,"unsupported group");
                return n;
            }
        }
        else{
            group=++r->groups;
            if(group>=RX_GROUPS){
                rx_bad(r,"capture limit");
                return n;
            }
        }
        int child=rx_expr(r,depth+1);
        if(rx_take(r)!=')')rx_bad(r,"unterminated group");
        r->nodes[n]=(RxNode){
            .kind=look?RX_LOOK:group?RX_GROUP:RX_SEQ,.a=child,.b=0,.group=group,.neg=negative
        };
        return n;
    }
    if(c=='['){
        r->nodes[n].kind=RX_CLASS;
        if(rx_peek(r)=='^'){
            rx_take(r);
            r->nodes[n].neg=1;
        }
        while(!r->error&&rx_peek(r)>=0&&rx_peek(r)!=']'){
            int first=rx_take(r);
            if(first=='\\'){
                int temporary=rx_node(r,RX_EMPTY,0,0);
                first=rx_escape(r,temporary);
                if(first<0){
                    RxNode*q=r->nodes+temporary;
                    if(q->neg){
                        /* Complement predefined classes within a union are deliberately rejected. */rx_bad(r,"complement class union unsupported");
                        break;
                    }
                    for(int i=0;i<q->nrange;i++)rx_range(r,n,q->range[i].lo,q->range[i].hi);
                    continue;
                }
            }
            int last=first;
            if(rx_peek(r)=='-'&&r->at+1<r->pattern.n&&r->pattern.p[r->at+1]!=']'){
                rx_take(r);
                last=rx_take(r);
                if(last=='\\'){
                    int temporary=rx_node(r,RX_EMPTY,0,0);
                    last=rx_escape(r,temporary);
                    if(last<0){
                        rx_bad(r,"invalid range endpoint");
                        break;
                    }
                }
            }
            rx_range(r,n,first,last);
        }
        if(rx_take(r)!=']')rx_bad(r,"unterminated character class");
        return n;
    }
    if(c=='\\'){
        if(rx_peek(r)=='b'||rx_peek(r)=='B'){
            r->nodes[n].kind=RX_BOUNDARY;
            r->nodes[n].neg=rx_take(r)=='B';
            return n;
        }
        int value=rx_escape(r,n);
        if(value>=0){
            r->nodes[n].kind=RX_CHAR;
            r->nodes[n].a=value;
        }
        return n;
    }
    if(c=='*'||c=='+'||c=='?'||c==')'||c<0){
        rx_bad(r,"unexpected quantifier or delimiter");
        return n;
    }
    r->nodes[n].kind=c=='.'?RX_DOT:c=='^'?RX_BOL:c=='$'?RX_EOL:RX_CHAR;
    r->nodes[n].a=c;
    return n;
}

static int rx_uint(Rx*r){
    int n=0,have=0;
    while(rx_peek(r)>='0'&&rx_peek(r)<='9'){
        have=1;
        n=n*10+rx_take(r)-'0';
        if(n>1024){
            rx_bad(r,"repetition limit");
            return 0;
        }
    }
    if(!have)rx_bad(r,"missing repetition count");
    return n;
}

static int rx_seq(Rx*r,int depth){
    int result=0;
    while(!r->error&&rx_peek(r)>=0&&rx_peek(r)!=')'&&rx_peek(r)!='|'){
        int atom=rx_atom(r,depth),c=rx_peek(r),min=0,max=0,repeat=0;
        if(c=='*'||c=='+'||c=='?'){
            rx_take(r);
            repeat=1;
            min=c=='+'?1:0;
            max=c=='?'?1:-1;
        }
        else if(c=='{'){
            rx_take(r);
            min=rx_uint(r);
            max=min;
            if(rx_peek(r)==','){
                rx_take(r);
                max=rx_peek(r)=='}'?-1:rx_uint(r);
            }
            if(rx_take(r)!='}'||(max>=0&&max<min))rx_bad(r,"invalid repetition");
            repeat=1;
        }
        if(repeat){
            int n=rx_node(r,RX_REP,atom,0);
            r->nodes[n].min=min;
            r->nodes[n].max=max;
            if(rx_peek(r)=='?'){
                rx_take(r);
                r->nodes[n].lazy=1;
            }
            atom=n;
        }
        result=result?rx_node(r,RX_SEQ,result,atom):atom;
    }
    return result;
}

static int rx_expr(Rx*r,int depth){
    int n=rx_seq(r,depth);
    while(!r->error&&rx_peek(r)=='|'){
        rx_take(r);
        n=rx_node(r,RX_ALT,n,rx_seq(r,depth));
    }
    return n;
}

static int rx_compile(Rx*r,int n,int next){
    if(r->error||n==0)return next;
    RxNode p=r->nodes[n];
    switch(p.kind){
        case RX_EMPTY:return next;
        case RX_SEQ:{
            int tail=rx_compile(r,p.b,next);
            return rx_compile(r,p.a,tail);
        }
        case RX_ALT:{
            int a=rx_compile(r,p.a,next),b=rx_compile(r,p.b,next);
            return rx_inst(r,RX_SPLIT,a,b,0,0);
        }
        case RX_GROUP:{
            int tail=rx_inst(r,RX_SAVE,next,0,p.group*2+1,0);
            int body=rx_compile(r,p.a,tail);
            return rx_inst(r,RX_SAVE,body,0,p.group*2,0);
        }
        case RX_LOOK:{
            int accept=rx_inst(r,RX_MATCH,0,0,0,0);
            int sub=rx_compile(r,p.a,accept);
            return rx_inst(r,RX_LOOK,sub,next,0,p.neg);
        }
        case RX_REP:{
            int pc=next;
            if(p.max<0){
                if(r->loops==RX_LOOPS){
                    rx_bad(r,"loop limit");
                    return 0;
                }
                int loop=rx_inst(r,RX_LOOP,0,next,r->loops++,p.lazy);
                int body=rx_compile(r,p.a,loop);
                r->code[loop].a=body;
                pc=loop;
            }
            else for(int i=p.min;i<p.max;i++){
                int body=rx_compile(r,p.a,pc);
                pc=rx_inst(r,RX_SPLIT,p.lazy?pc:body,p.lazy?body:pc,0,0);
            }
            for(int i=0;i<p.min;i++)pc=rx_compile(r,p.a,pc);
            return pc;
        }
        default:return rx_inst(r,p.kind,next,0,n,0);
    }
}

static int rx_fold(int c){
    uint32_t upper=t_case_simple(c,1);
    return c>=128&&upper<128?c:(int)upper;
}

static int rx_word(int c){
    return(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_';
}

static int rx_class(Rx*r,RxNode*p,int c){
    int found=0;
    if(r->ignore&&p->folded){
        int key=rx_fold(c);
        found=(p->folded[key/8]>>(key%8))&1;
    }
    else for(int i=0;i<p->nrange;i++)if(c>=p->range[i].lo&&c<=p->range[i].hi){
        found=1;
        break;
    }
    return found!=p->neg;
}

static void rx_fold_classes(Rx*r){
    if(!r->ignore)return;
    for(int i=1;i<r->nn&&!r->error;i++){
        RxNode *p=r->nodes+i;
        if(p->kind!=RX_CLASS)continue;
        p->folded=calloc(8192,1);
        if(!p->folded){
            rx_bad(r,"class allocation failed");
            break;
        }
        for(int j=0;j<p->nrange&&!r->error;j++)for(int c=p->range[j].lo;c<=p->range[j].hi;c++){
            if(++r->steps>RX_BUDGET){
                rx_bad(r,"class construction budget exceeded");
                break;
            }
            int key=rx_fold(c);
            p->folded[key/8]|=1u<<(key%8);
        }
    }
}

static int rx_run(Rx*r,RxState state,RxState*result,int depth){
    if(depth>32){
        rx_bad(r,"assertion depth limit");
        return 0;
    }
    RxState*stack=NULL;
    size_t used=0,capacity=0;
    int matched=0;
    while(!r->error){
        if(++r->steps>RX_BUDGET){
            rx_bad(r,"backtracking budget exceeded");
            break;
        }
        RxInst in=r->code[state.pc];
        RxNode*node=r->nodes+in.arg;
        int c=state.pos<(int)r->input.n?r->input.p[state.pos]:-1;
        int good=1;
        switch(in.kind){
            case RX_MATCH:*result=state;
            matched=1;
            goto finish;
            case RX_CHAR:good=c>=0&&(r->ignore?rx_fold(c)==rx_fold(node->a):c==node->a);
            if(good){
                state.pos++;
                state.pc=in.a;
            }
            break;
            case RX_DOT:good=c>=0&&c!=10&&c!=13&&c!=0x2028&&c!=0x2029;
            if(good){
                state.pos++;
                state.pc=in.a;
            }
            break;
            case RX_CLASS:good=c>=0&&rx_class(r,node,c);
            if(good){
                state.pos++;
                state.pc=in.a;
            }
            break;
            case RX_BOL:good=state.pos==0;
            if(good)state.pc=in.a;
            break;
            case RX_EOL:good=state.pos==(int)r->input.n;
            if(good)state.pc=in.a;
            break;
            case RX_BOUNDARY:{
                int before=state.pos?rx_word(r->input.p[state.pos-1]):0;
                good=(before!=rx_word(c))!=node->neg;
                if(good)state.pc=in.a;
                break;
            }
            case RX_SAVE:state.caps[in.arg]=state.pos;
            state.pc=in.a;
            break;
            case RX_LOOK:{
                RxState sub=state,out;
                sub.pc=in.a;
                int yes=rx_run(r,sub,&out,depth+1);
                good=yes!=in.neg;
                if(good){
                    if(yes)memcpy(state.caps,out.caps,sizeof(state.caps));
                    state.pc=in.b;
                }
                break;
            }
            case RX_SPLIT:case RX_LOOP:{
                if(in.kind==RX_LOOP&&state.last[in.arg]==state.pos){
                    state.pc=in.b;
                    break;
                }
                if(used==capacity){
                    if(capacity>=65536){
                        rx_bad(r,"backtracking stack limit");
                        break;
                    }
                    capacity=capacity?capacity*2:64;
                    stack=t_resize(stack,capacity*sizeof(*stack));
                }
                RxState alternate=state;
                if(in.kind==RX_LOOP){
                    RxState body=state;
                    body.pc=in.a;
                    body.last[in.arg]=state.pos;
                    RxState exit=state;
                    exit.pc=in.b;
                    if(in.neg){
                        alternate=body;
                        state=exit;
                    }
                    else{
                        alternate=exit;
                        state=body;
                    }
                }
                else{
                    alternate.pc=in.b;
                    state.pc=in.a;
                }
                stack[used++]=alternate;
                break;
            }
            default:rx_bad(r,"invalid instruction");
            good=0;
            break;
        }
        if(!good){
            if(!used)break;
            state=stack[--used];
        }
    }
    finish:free(stack);
    return matched;
}

static int rx_search(Rx*r,int start,RxState*out){
    for(int pos=start;pos<=(int)r->input.n&&!r->error;pos++){
        RxState state={
            .pc=r->entry,.pos=pos
        };
        for(int i=0;i<RX_GROUPS*2;i++)state.caps[i]=-1;
        for(int i=0;i<RX_LOOPS;i++)state.last[i]=-1;
        state.caps[0]=pos;
        if(rx_run(r,state,out,0)){
            out->caps[1]=out->pos;
            return 1;
        }
    }
    return 0;
}

static void rx_substitution(TUnits*out,TUnits input,TUnits replacement,RxState*match,int groups){
    for(size_t i=0;i<replacement.n;i++){
        uint16_t c=replacement.p[i];
        if(c=='$'&&i+1<replacement.n){
            int token=replacement.p[i+1],lo=-1,hi=-1;
            if(token=='$'){
                t_unit(out,'$');
                i++;
                continue;
            }
            if(token=='&'){
                lo=match->caps[0];
                hi=match->caps[1];
            }
            else if(token==96){
                lo=0;
                hi=match->caps[0];
            }
            else if(token=='\''){
                lo=match->caps[1];
                hi=input.n;
            }
            else if(token>='1'&&token<='9'){
                int group=token-'0',consumed=1;
                if(i+2<replacement.n&&replacement.p[i+2]>='0'&&replacement.p[i+2]<='9'&&group*10+replacement.p[i+2]-'0'<=groups){
                    group=group*10+replacement.p[i+2]-'0';
                    consumed=2;
                }
                if(group<=groups){
                    lo=match->caps[group*2];
                    hi=match->caps[group*2+1];
                    if(lo<0)lo=hi=0;
                    i+=consumed;
                    for(int k=lo;k<hi;k++)t_unit(out,input.p[k]);
                    continue;
                }
            }
            if(lo>=0){
                for(int k=lo;k<hi;k++)t_unit(out,input.p[k]);
                i++;
                continue;
            }
        }
        t_unit(out,c);
    }
}

static FLValue v11_regex(const char*op,FLValue argv){
    FLVector*a=(FLVector*)argv.obj;
    FLValue value=a->len?a->data[0]:fl_nil(),pattern=a->len>1?a->data[1]:fl_nil(),replacement=a->len>2?a->data[2]:fl_nil();
    Rx r={
        .pattern=t_units(t_js_string(pattern)),.input=t_units(t_js_string(value)),.nn=1,.ignore=!strcmp(op,"regex_match_i")
    };
    r.nodes=calloc(RX_MAX,sizeof(*r.nodes));
    r.code=calloc(RX_MAX,sizeof(*r.code));
    if(!r.nodes||!r.code){
        free(r.nodes);
        free(r.code);
        free(r.pattern.p);
        free(r.input.p);
        fl_throw(fl_str_val("regex allocation failed"));
        return fl_nil();
    }
    if(r.pattern.n>8192)rx_bad(&r,"pattern length limit");
    int tree=r.error?0:rx_expr(&r,0);
    if(r.at!=r.pattern.n)rx_bad(&r,"unmatched delimiter");
    int accept=rx_inst(&r,RX_MATCH,0,0,0,0);
    r.entry=rx_compile(&r,tree,accept);
    rx_fold_classes(&r);
    int boolean=!strcmp(op,"regex_match")||!strcmp(op,"regex_match_i"),find=!strcmp(op,"regex_find"),split=!strcmp(op,"regex_split"),replace=!strcmp(op,"regex_replace")||!strcmp(op,"regex_replace_first");
    FLValue result=boolean?fl_bool(false):find?fl_nil():fl_vec_new();
    TUnits output={
        0
    }
    ,rep=replace?t_units(t_js_string(replacement)):(TUnits){0};
    int scan=0,cursor=0;
    RxState match;
    if(split&&r.input.n==0){
        if(!rx_search(&r,0,&match))result=fl_vec_push(result,fl_str_val(""));
    }
    else while(!r.error&&scan<=(int)r.input.n&&rx_search(&r,scan,&match)){
        int lo=match.caps[0],hi=match.caps[1];
        if(boolean){
            result=fl_bool(true);
            break;
        }
        if(find){
            result=t_from_units(r.input.p+lo,hi-lo);
            break;
        }
        if(replace){
            for(int k=cursor;k<lo;k++)t_unit(&output,r.input.p[k]);
            rx_substitution(&output,r.input,rep,&match,r.groups);
            cursor=hi;
            if(!strcmp(op,"regex_replace_first"))break;
        }
        else if(split){
            if(hi==cursor&&hi==lo){
                scan=hi+1;
                continue;
            }
            result=fl_vec_push(result,t_from_units(r.input.p+cursor,lo-cursor));
            for(int g=1;g<=r.groups;g++){
                int start=match.caps[g*2],end=match.caps[g*2+1];
                result=fl_vec_push(result,start<0?fl_nil():t_from_units(r.input.p+start,end-start));
            }
            cursor=hi;
        }
        else result=fl_vec_push(result,t_from_units(r.input.p+lo,hi-lo));
        scan=hi==lo?hi+1:hi;
        if(split&&scan==(int)r.input.n)break;
    }
    if(replace){
        for(int k=cursor;k<(int)r.input.n;k++)t_unit(&output,r.input.p[k]);
        result=t_from_units(output.p,output.n);
    }
    else if(split&&r.input.n)result=fl_vec_push(result,t_from_units(r.input.p+cursor,r.input.n-cursor));
    int error=r.error;
    const char*why=r.why;
    for(int i=1;i<r.nn;i++)free(r.nodes[i].folded);
    free(r.nodes);
    free(r.code);
    free(r.pattern.p);
    free(r.input.p);
    free(output.p);
    free(rep.p);
    if(error){
        char message[256];
        snprintf(message,sizeof(message),"%s: invalid pattern or resource limit: %s",op,why?why:"unknown");
        fl_throw(fl_str_val(message));
        return fl_nil();
    }
    return result;
}
