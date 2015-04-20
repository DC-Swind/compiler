#ifndef _COMMON__H_
#define _COMMON__H_

struct Var{
    int type;//1-int,2-float,3-struct,4-intarray,5-floatarray,6-structarray,7-{
    char *tname;//type name. int float struct a ...
    int defline;
    int deadline;
    char *name;//var name
    int value; 
    int arraylen;
    struct Var *father;
    struct Var *sonlist;
    struct Var *next;
};
struct Struct{
    int type; //1-has name, 2-has no name
    char *name;
    int defline;
    
    struct Var *feild;
    struct Struct *next;
};
struct funcList{
    int type;
    int rt_type;
    int defline;
    char *name;
    
    struct funcList *next;
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
#endif
