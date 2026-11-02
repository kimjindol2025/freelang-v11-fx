/* Read the final DOM independently of the FreeLang accessor implementation. */
static int dom_checks(void);
#define L01_PROOF_EXTRA() dom_checks()
#include "browser-proof.c"
static int dom_checks(void){
 int tree=node("#literal > #paragraph:first-child")&&node("#literal > #figure:last-child > #photo");
 int photo=node("#photo");const char *alt=photo?attribute(photo,"alt"):NULL;
 tree=tree&&alt&&!strcmp(alt,"한😀")&&attribute(node("#paragraph"),"hidden")==NULL;
 int literal=node("#paragraph img")==0&&node("#literal script")==0&&node("#literal iframe")==0;
 printf("J_DOM_CDP_TREE=%s\n",tree?"PASS":"FAIL");
 printf("J_DOM_CDP_LITERAL=%s\n",literal?"PASS":"FAIL");return tree&&literal;
}
