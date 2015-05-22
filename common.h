#ifndef _COMMON__H_
#define _COMMON__H_

#define _INT 1
#define _FLOAT 2
#define _STRUCT 3
#define _INTARRAY 4
#define _FLOATARRAY 5
#define _STRUCTARRAY 6
#define _NUMINT 8
#define _NUMFLOAT 9

#define _LABEL 1
#define _FUNCTION 2
#define _ASSIGNOP 3
#define _ADD 4
#define _MINUS 5
#define _STAR 6
#define _DIV 7
#define _GETADDR 8
#define _GETVALUE 9
#define _ASSIGNOP_GETVALUE 10
#define _GOTO 11
#define _IF 12
#define _RETURN 13
#define _DEC 14
#define _ARG 15
#define _CALL 16
#define _PARAM 17
#define _READ 18
#define _WRITE 19

struct Instr{
    int type; // 
    char* target;
    char* op;
    char* arg1;
    char* arg2;
    struct Instr* prev;
    struct Instr* next;
};
struct Varm{
    int type;//1-int,2-float,3-struct,4-intarray,5-floatarray,6-structarray,7-{
    char *name; //int a;  a is name , int is tname, generate a v1 is vname
    char *tname;
    char *vname;
    int arraylen;
    int arraydimension;
    int size;
    struct Varm* next;
    struct Varm* sonlist;
};
struct Structm{
    int type;
    char* name;
    struct Varm* feild;
    struct Structm* next;
};

struct Var{
    int type;//1-int,2-float,3-struct,4-intarray,5-floatarray,6-structarray,7-{
    char *tname;//type name. int float struct a ...
    int defline;
    int deadline;
    char *name;//var name
    float value; 
    int arraylen;
	int arraydimension;
    struct Var *father;
    struct Var *sonlist;
    struct Var *next;
};
struct RTtype{
    int type;
    char* tname;
    int dimension;
};
struct Struct{
    int type; //1-has name, 2-has no name
    char *name;
    int defline;
    
    struct Var *feild;
    struct Struct *next;
};
struct Func{
    int type;//0 - declaration, 1 - define
    int rt_type; //Var-type
    char* rt_tname;
    int defline;
    char *name;
	struct Var *feild;
    struct Func *next;
};
struct treeNode{
    int lineno;
    int type; //1-stop and no value, 2-stop and have value, 3-int ,4-float, 5-no stop
    int tokentype;
    char * name;
    char * value;
    int sonN;
    struct treeNode *sonlist;
    struct treeNode *next;
};

int middle(struct treeNode* root);
int checkmean(struct treeNode* root);
#endif
