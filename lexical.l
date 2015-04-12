%option yylineno
%{
#include <stdio.h>
#include "syntax.tab.h"

struct treeNode{
    int lineno;
    int type; //1-stop and no value, 2 - stop and have value, 3 - int ,4 - float, 5-no stop
    int tokentype;
    char * name;
    char * value;
    int sonN;
    struct treeNode *sonlist;
    struct treeNode *next;
};
void setyylval(int type,int tokentype,char* name);
int yycolumn = 1;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;
%}
letter [_a-zA-Z]
digit [0-9]
STAR [*]
DIV [/]
MINUS [-]
PLUS [+]
RELOP >|<|>=|<=|==|!=
INT16 0[xX][1-9a-fA-F][0-9a-fA-F]*
INT10 ([0]|([1-9]{digit}*))
INT8 0[1-7][0-7]*
INT {INT8}|{INT10}|{INT16}
ERRORINT8 0(([089a-fA-F][0-9a-fA-F]*)|([1-7][0-7]*[89a-fA-F][0-7]*))
ERRORINT16 0[xX](([0g-zG-Z][0-9a-zA-Z]*)|([1-9a-fA-F][0-9a-fA-F]*[g-zG-Z][0-9a-fA-F]*))
ERRORINT {ERRORINT8}|{ERRORINT16}
ID {letter}({letter}|{digit})*
FLOATA {digit}*\.{digit}+
FLOATB (([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))[Ee][+-]?[0-9]+
FLOAT {FLOATA}|{FLOATB}
TYPE int|float
ASSIGNOP [=]
AND &&
OR \|\|
DOT \.
SEMI ;
COMMA ,
NOT !
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
space [ ]
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
ANYTHING [ _0-9a-zA-Z+!=<>|.;,&?]|{LP}|{RP}|{LB}|{RB}|{LC}|{RC}|{MINUS}
LINECOMMENT \/\/({ANYTHING}|[*/])*
LCOMMENT \/\*(\n|({ANYTHING}[*/]{ANYTHING})|{ANYTHING})*
RCOMMENT \*\/
MCOMMENT {LCOMMENT}{RCOMMENT}
%%


{STRUCT} {
    printf("Line %d:(STRUCT , %s)\n",yylineno,yytext);
    setyylval(1,1,"STRUCT");
    return STRUCT;
}
{RETURN} {
    printf("Line %d:(RETURN , %s)\n",yylineno,yytext);
    setyylval(1,1,"RETURN");
    return RETURN;
}
{IF} {
    printf("Line %d:(IF , %s)\n",yylineno,yytext);
    setyylval(1,1,"IF");
    return IF;
}
{ELSE} {
    printf("Line %d:(ELSE , %s)\n",yylineno,yytext);
    setyylval(1,1,"ELSE");
    return ELSE;
}
{WHILE} {
    printf("Line %d:(WHILE , %s)\n",yylineno,yytext); 
    setyylval(1,1,"WHILE");
    return WHILE;
}
{TYPE} {
    printf("Line %d:(TYPE , %s)\n",yylineno,yytext);
    setyylval(2,2,"TYPE");
    return TYPE;
}

{STAR} {
    printf("Line %d:(STAR , %s)\n",yylineno,yytext);
    setyylval(1,1,"STAR");
    return STAR;
}
{DIV} {
    printf("Line %d:(DIV , %s)\n",yylineno,yytext);
    setyylval(1,1,"DIV");
    return DIV;
}
{PLUS} {
    printf("Line %d:(PLUS , %s)\n",yylineno,yytext);
    setyylval(1,1,"PLUS");
    return PLUS;
}
{MINUS} {
    printf("Line %d:(MINUS , %s)\n",yylineno,yytext);
    setyylval(1,1,"MINUS");
    return MINUS;
}
{ASSIGNOP} {
    printf("Line %d:(ASSIGNOP , %s)\n",yylineno,yytext);
    setyylval(1,1,"ASSIGNOP");
    return ASSIGNOP;
}

{ID} {
    printf("Line %d:(ID , %s)\n",yylineno,yytext);
    setyylval(2,2,"ID");
    return ID;
}
{INT} { 
    printf("Line %d:(INT , %s)\n",yylineno,yytext);
    setyylval(3,3,"INT");
    return INT;
}
{FLOAT} {
    printf("Line %d:(FLOAT , %s)\n",yylineno,yytext); 
    setyylval(4,4,"FLOAT");
    return FLOAT;
}
{SEMI} {
    printf("Line %d:(SEMI , %s)\n",yylineno,yytext);
    setyylval(1,1,"SEMI");
    return SEMI;
}
{space}+ {}
\n { yycolumn = 1;}
{COMMA} {
    printf("Line %d:(, , %s)\n",yylineno,yytext);
    setyylval(1,1,"COMMA");
    return COMMA;
}
{AND} {
    printf("Line %d:(AND , %s)\n",yylineno,yytext);
    setyylval(1,1,"AND");
    return AND;
}
{OR} {
    printf("Line %d:(OR , %s)\n",yylineno,yytext);
    setyylval(1,1,"OR");
    return OR;
}

"(" {
    printf("Line %d:(LP , %s)\n",yylineno,yytext);
    setyylval(1,1,"LP");
    return LP;
}

")" {
    printf("Line %d:(RP , %s)\n",yylineno,yytext);
    setyylval(1,1,"RP");
    return RP;
}

"{" {
    printf("Line %d:(LC , %s)\n",yylineno,yytext);
    setyylval(1,1,"LC");
    return LC;
}

"}" {
    printf("Line %d:(RC , %s)\n",yylineno,yytext);
    setyylval(1,1,"RC");
    return RC;
}
{NOT}  {
    printf("Line %d:(NOT , %s)\n",yylineno,yytext);
    setyylval(1,1,"NOT");
    return NOT;
}
{RELOP} {
    printf("Line %d:(RELOP , %s)\n",yylineno,yytext);
    setyylval(1,1,"RELOP");
    return RELOP;
}

{DOT} {
    printf("Line %d:(DOT , %s)\n",yylineno,yytext);
    setyylval(1,1,"DOT");
    return DOT;
}
"[" {
    printf("Line %d:(LB , %s)\n",yylineno,yytext);
    setyylval(1,1,"LB");
    return LB;
}
"]" {
    printf("Line %d:(RB , %s)\n",yylineno,yytext);
    setyylval(1,1,"RB");
    return RB;
}

{LINECOMMENT} {
    printf("Line %d:(LINECOMMENT , %s)\n",yylineno,yytext);
}
{MCOMMENT} {
    printf("Line %d:(MULCOMMET , %s)\n",yylineno,yytext);
}
{LCOMMENT} {
    printf("Line %d:(LCOMMENT , %s)\n",yylineno,yytext);
    setyylval(1,1,"LCOMMENT");
    return LCOMMENT;
}
{RCOMMENT} {
    printf("Line %d:(RCOMMENT , %s)\n",yylineno,yytext);
    setyylval(1,1,"RCOMMENT");
    return RCOMMENT;
}

{ERRORINT} {
    printf("Error type A at Line %d: Illegal number \'%s\'.\n",yylineno,yytext);
    return INT;
}
. {
    printf("Error type A at Line %d: Mysterious character \"%s\".\n",yylineno,yytext);
}
%%
/*
int main(int argc, char** argv){
    if (argc > 1){
        if(!(yyin = fopen(argv[1],"r"))){
            perror(argv[1]);
            return 1;
        }
    }
    yylex();
    return 0;
}
*/
int int16Toint(char *s){
    int ans = 0,i;
    for(i = 2; i< strlen(s); i++){
        ans *= 16;
        if (s[i] == 'a' || s[i] == 'A') ans += 10;
        else if (s[i] == 'b' || s[i] == 'B') ans += 11;
        else if (s[i] == 'c' || s[i] == 'C') ans += 12;
        else if (s[i] == 'd' || s[i] == 'D') ans += 13;
        else if (s[i] == 'e' || s[i] == 'E') ans += 14;
        else if (s[i] == 'f' || s[i] == 'F') ans += 15;
        else ans += s[i] - '0';
    }
    return ans;
}
int int8Toint(char *s){
    int ans = 0,i;
    for(i = 1; i<strlen(s); i++){
        ans *= 8;
        ans += s[i] - '0';
    }
    return ans;
}
float floatTofloat(char *s){
    int e = 0;
    if (strstr(s,"e") != NULL) e = strstr(s,"e") - s; 
    else e = strstr(s,"E") - s;
    char a[100],b[100];
    strncpy(a,s,e);
    float aa = atof(a);
    int bb,i;
    if (s[e+1] == '+'){
        strncpy(b,&s[e+2],strlen(s)-e-1);
        bb = atoi(b);
        for (i=0; i<bb; i++) aa *= 10;
    }else if (s[e+1] == '-'){
        strncpy(b,&s[e+2],strlen(s)-e-1);
        bb = atoi(b);
        for (i=0; i<bb; i++) aa /= 10;
    }else{
        strncpy(b,&s[e+1],strlen(s)-e);
        bb = atoi(b);
        for (i=0; i<bb; i++) aa *= 10;
    }
    return aa;
}
void setyylval(int type,int tokentype,char* name){
    yylval = (struct treeNode*)malloc(sizeof(struct treeNode));
    yylval->lineno = yylineno;
    yylval->type = type;
    yylval->tokentype = tokentype;
    yylval->name = malloc(strlen(name)+1);
    strcpy(yylval->name,name);
    if (type == 3){
        yylval->value = malloc(32);
        if (strlen(yytext) == 1 || yytext[0] !='0')sprintf(yylval->value,"%d",atoi(yytext));
        else if (yytext[0] == '0' && yytext[1] == 'x') sprintf(yylval->value,"%d",int16Toint(yytext));
        else sprintf(yylval->value,"%d",int8Toint(yytext));
    }else if (type == 4){
        yylval->value = malloc(32);
        if (strstr(yytext,"e") == NULL && strstr(yytext,"E") == NULL) sprintf(yylval->value,"%f",atof(yytext));
        else sprintf(yylval->value,"%f",floatTofloat(yytext));
    }else{
        yylval->value = malloc(strlen(yytext)+1);
        strcpy(yylval->value,yytext);
    }
    yylval->sonN = 0;
    yylval->sonlist = NULL;
    yylval->next = NULL;
}

