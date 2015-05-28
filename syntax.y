%{
#include "common.h"
struct treeNode *createNode(int line,char* name,int n,...);
void printTree(struct treeNode *node,int deep);
#define YYSTYPE struct treeNode *

#include <stdio.h>
#include "lex.yy.c"
#include <stdarg.h>
#define max(a,b) (a>b?a:b)
int occurError = 0;
void yyyerror(char *msg,int lineno);
int checkmean(struct treeNode* root);
%}


%token INT FLOAT ID/*1 2 3*/
%token PLUS MINUS STAR DIV/*4 5 6 7*/
%token AND OR NOT ASSIGNOP RELOP /*8 9 10 11 12*/
%token SEMI COMMA DOT/*13 14 15*/
%token LP RP LB RB LC RC/*16 17 18 19 20 21*/
%token IF ELSE WHILE STRUCT RETURN TYPE/*22 23 24 25 26 27*/


%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT


%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE 
%%

Program : ExtDefList {$$ = createNode(@$.first_line,"Program",1,$1); if (occurError == 0) {/*printTree($$,0);*/ /*checkmean($$);*/ middle($$);}}
  /*| LCOMMENT RCOMMENT ExtDefList {$$ = createNode(@$.first_line,"Program",1,$3); if (occurError == 0)printTree($$,0);}*/
  | error ExtDefList {}
;
ExtDefList : ExtDef ExtDefList {$$ = createNode(@$.first_line,"ExtDefList",2,$1,$2);}
  /*| ExtDef LCOMMENT RCOMMENT ExtDefList {$$ = createNode(@$.first_line,"ExtDefList",2,$1,$4);}*/
  | {$$ = createNode(@$.first_line,"ExtDefList",0);}
  | ExtDef error ExtDefList {}
;
ExtDef : Specifier ExtDecList SEMI {$$ = createNode(@$.first_line,"ExtDef",3,$1,$2,$3);}
  | Specifier SEMI {$$ = createNode(@$.first_line,"ExtDef",2,$1,$2);}  /*But this can not in a Func*/
  | Specifier FunDec CompSt {$$ = createNode(@$.first_line,"ExtDef",3,$1,$2,$3);}
  | error FunDec CompSt
  | Specifier FunDec SEMI {$$ = createNode(@$.first_line,"ExtDef",3,$1,$2,$3);}
;
ExtDecList : VarDec {$$ = createNode(@$.first_line,"ExtDecList",1,$1);}
  | VarDec COMMA ExtDecList {$$ = createNode(@$.first_line,"ExtDecList",3,$1,$2,$3);}
;


Specifier : TYPE {$$ = createNode(@$.first_line,"Specifier",1,$1);}
  | StructSpecifier {$$ = createNode(@$.first_line,"Specifier",1,$1);}
;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = createNode(@$.first_line,"StructSpecifier",5,$1,$2,$3,$4,$5);}
  | STRUCT OptTag LC error DefList RC
  | STRUCT Tag {$$ = createNode(@$.first_line,"StructSpecifier",2,$1,$2);}
;
OptTag : ID {$$ = createNode(@$.first_line,"OptTag",1,$1);}
  | {$$ = createNode(@$.first_line,"OptTag",0);}
;
Tag : ID {$$ = createNode(@$.first_line,"Tag",1,$1);}
;


VarDec : ID {$$ = createNode(@$.first_line,"VarDec",1,$1);}
  | VarDec LB INT RB {$$ = createNode(@$.first_line,"VarDec",4,$1,$2,$3,$4);}
;
FunDec : ID LP VarList RP {$$ = createNode(@$.first_line,"FunDec",4,$1,$2,$3,$4);}
  | ID LP RP {$$ = createNode(@$.first_line,"FunDec",3,$1,$2,$3);}
  | ID LP error RP
  /*| ID LP VarList {yyyerror("Missing )",@1.first_line);}*/
  | ID LP {yyyerror("Missing )",@1.first_line);}
  /*| ID VarList RP {yyyerror("Missing (",@1.first_line);}*/
  | ID RP {yyyerror("Missing (",@1.first_line);}

;
VarList : ParamDec COMMA VarList {$$ = createNode(@$.first_line,"VarList",3,$1,$2,$3);}
  | ParamDec {$$ = createNode(@$.first_line,"VarList",1,$1);}
;
ParamDec : Specifier VarDec {$$ = createNode(@$.first_line,"ParamDec",2,$1,$2);}
;


CompSt : LC DefList StmtList RC {$$ = createNode(@$.first_line,"CompSt",4,$1,$2,$3,$4);}
  /*| LC LCOMMENT RCOMMENT DefList StmtList RC {$$ = createNode(@$.first_line,"CompSt",4,$1,$4,$5,$6);}*/
  | LC error DefList StmtList RC {}
;
StmtList : Stmt StmtList {$$ = createNode(@$.first_line,"StmtList",2,$1,$2);}
  | {$$ = createNode(@$.first_line,"StmtList",0);}
  | Stmt error StmtList {}
;
Stmt : Exp SEMI {$$ = createNode(@$.first_line,"Stmt",2,$1,$2);}
  | CompSt {$$ = createNode(@$.first_line,"Stmt",1,$1);}
  | RETURN Exp SEMI {$$ = createNode(@$.first_line,"Stmt",3,$1,$2,$3);}
  | IF LP Exp RP Stmt  %prec LOWER_THAN_ELSE {$$ = createNode(@$.first_line,"Stmt",5,$1,$2,$3,$4,$5);}
  | IF LP Exp RP Stmt ELSE Stmt {$$ = createNode(@$.first_line,"Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
  | WHILE LP Exp RP Stmt {$$ = createNode(@$.first_line,"Stmt",5,$1,$2,$3,$4,$5);}
  | Exp error {}  /*ppp*/
;


DefList : Def DefList {$$ = createNode(@$.first_line,"DefList",2,$1,$2);}
  /*| Def LCOMMENT RCOMMENT DefList {$$ = createNode(@$.first_line,"DefList",2,$1,$4);}*/
  | {$$ = createNode(@$.first_line,"DefList",0);}
  | Def error DefList {}
;
Def : Specifier DecList SEMI {$$ = createNode(@$.first_line,"Def",3,$1,$2,$3);}
;
DecList : Dec {$$ = createNode(@$.first_line,"DecList",1,$1);}
  | Dec COMMA DecList {$$ = createNode(@$.first_line,"DecList",3,$1,$2,$3);}
;
Dec : VarDec {$$ = createNode(@$.first_line,"Dec",1,$1);} 
  | VarDec ASSIGNOP Exp {$$ = createNode(@$.first_line,"Dec",3,$1,$2,$3);}
;


Exp : Exp ASSIGNOP Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp AND Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp OR Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp RELOP Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp PLUS Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp MINUS Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp STAR Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp DIV Exp {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | LP Exp RP {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | MINUS Exp {$$ = createNode(@$.first_line,"Exp",2,$1,$2);}
  | NOT Exp {$$ = createNode(@$.first_line,"Exp",2,$1,$2);}
  | ID LP Args RP {$$ = createNode(@$.first_line,"Exp",4,$1,$2,$3,$4);}
  | ID LP RP {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | Exp LB Exp RB {$$ = createNode(@$.first_line,"Exp",4,$1,$2,$3,$4);}
  | Exp DOT ID {$$ = createNode(@$.first_line,"Exp",3,$1,$2,$3);}
  | ID {$$ = createNode(@$.first_line,"Exp",1,$1);}
  | INT {$$ = createNode(@$.first_line,"Exp",1,$1);}
  | FLOAT {$$ = createNode(@$.first_line,"Exp",1,$1);}
  /*| Exp LB Exp error RB {printf("Exp LB Exp error RB.\n");}*/
;
Args : Exp COMMA Args {$$ = createNode(@$.first_line,"Args",3,$1,$2,$3);}
  | Exp {$$ = createNode(@$.first_line,"Args",1,$1);}
;

%%

int main(int argc, char** argv){
    if (argc > 1){
        if(!(yyin = fopen(argv[1],"r"))){
            perror(argv[1]);
            return 1;
        }
    }else return 1;
    //yyrestart(f);
    //yydebug = 1;
    yyparse();
    return 0;
}


yyerror(char *msg){
    occurError++;
    //if (strncmp(msg,"syntax error",max(strlen(msg),12)) == 0)
        fprintf(stderr,"Error type B at Line %d: %s\n",yylineno,msg);
    
}
void yyyerror(char *msg,int lineno){
    occurError++;
    fprintf(stderr,"Error type B at Line %d: %s\n",lineno,msg);
}
struct treeNode *createNode(int line,char* name,int n,...){
    struct treeNode *node = (struct treeNode *)malloc(sizeof(struct treeNode));
    node->lineno = line;
    node->name = malloc(strlen(name)+1);
    node->type = 5;
    node->tokentype = 0;
    strcpy(node->name,name);
    node->sonlist = NULL;
    node->next = NULL;
    node->sonN = n;

    struct treeNode *tnode = NULL;
    va_list base;
    va_start(base,n); 
    int i;
    for (i=0; i<n; i++){
        struct treeNode *temp = va_arg(base,struct treeNode*);
        if (node->sonlist == NULL){ 
            node->sonlist = temp;
            tnode = node->sonlist;
        }else{
            tnode->next = temp;
            tnode = tnode->next;
        } 
    }
    va_end(base);
    return node;
}

void printTree(struct treeNode *node,int deep){
    int i;
    if (node->type == 5 && node->sonN == 0) return;
    if (node->type >5 || node->type < 1) return;
    for(i=0; i<deep; i++) printf("  ");
    if (node->type == 5)printf("%s (%d)\n",node->name,node->lineno);
    else if (node->type == 1)printf("%s\n",node->name);
    else if (node->type == 2 || node->type == 3 || node->type == 4)printf("%s: %s\n",node->name,node->value);
    struct treeNode *sonlist = node->sonlist;
    for(i=0; i<node->sonN; i++){
        printTree(sonlist,deep+1);
        sonlist = sonlist->next;
    }
}
