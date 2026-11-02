/* Read FreeLang source, skipping comments/strings; do not execute application code. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
static char *s;static size_t at;static int line;
typedef struct {size_t start,end;int line,string;} Item;
static void space(void){for(;;){while(s[at]&&isspace((unsigned char)s[at])){if(s[at++]=='\n')line++;}if(s[at]!=';')break;while(s[at]&&s[at]!='\n')at++;}}
static Item read_item(void){space();Item r={at,at,line,s[at]=='"'};char c=s[at++];if(c=='"'){while(s[at]){if(s[at]=='\\'&&s[at+1]){at+=2;continue;}if(s[at++]=='"')break;}}else if(c=='('||c=='['||c=='{'){char end=c=='('?')':c=='['?']':'}';while(s[at]){space();if(s[at]==end){at++;break;}read_item();}}else while(s[at]&&!isspace((unsigned char)s[at])&&!strchr("()[]{};",s[at]))at++;r.end=at;return r;}
static void scan(const char *file){while(s[at]){space();if(!s[at])break;if(s[at]=='"'||s[at]==';'){read_item();continue;}if(s[at]!='('){if(s[at]=='\n')line++;at++;continue;}size_t start=at++;int ln=line;space();Item op=read_item();size_t n=op.end-op.start;if(n>=6&&!strncmp(s+op.start,"regex",5)&&(s[op.start+5]=='_'||s[op.start+5]=='-')){size_t after_op=at;int after_op_line=line;Item input=read_item(),pattern=read_item();(void)input;size_t saved=at;int savedline=line;at=start;line=ln;Item call=read_item();printf("| [%s:%d](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/%s#L%d) | `%.*s` | %s | `",file,ln,file,ln,(int)n,s+op.start,pattern.string?"literal":"dynamic");for(size_t i=pattern.start;i<pattern.end;i++){char c=s[i];if(c=='\n')fputs(" ",stdout);else if(c=='|')fputs("&#124;",stdout);else putchar(c);}fputs("` |\n",stdout);at=after_op;line=after_op_line;(void)call;(void)saved;(void)savedline;}}}
int main(int argc,char**argv){for(int i=1;i<argc;i++){FILE*f=fopen(argv[i],"rb");if(!f)return 1;fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);s=calloc(n+1,1);if(!s||fread(s,1,n,f)!=(size_t)n)return 1;fclose(f);at=0;line=1;scan(argv[i]);free(s);}return 0;}
