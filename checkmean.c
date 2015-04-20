#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

struct Var* varlist;
struct Var* varlistp;
struct Struct* structlist;
struct Struct* structlistp;
//-----------------功能函数，暂时写在这里，后面移到另一个文件-----
int converttype(char* type){
    if (strcmp(type,"int") == 0) return 1;
    if (strcmp(type,"float") == 0) return 2;
    return 3;
}
int checkrepeat(char *name){
    struct Var* p = varlist;
    while(p != NULL){
        if (/*p->type == type && */strcmp(p->name,name) == 0) return 1;
        p = p->next;
    }
    return 0;
}
int checkrepeat_struct(){
    return 0;
}
//----------------------------------------------------------------
//forth level

int dfsDecList(struct treeNode* node,char* type,struct Var* rtlist){
    //node is DecList,rtlist 是将这些Dec插入到哪个Varlist
    //这个跟dfsExtDec差不多，不过注意赋值
}
int dfsDef(struct treeNode* node,struct Var* rtlist){
    //node is Def, rtlist是将这些Def插入到哪个varlist
    struct treeNode* def = node;

    //Specifier
    struct treeNode* specifier = def->sonlist;
    char* type = NULL; 
    if (strcmp(specifier->sonlist->name,"TYPE") == 0){
        type = specifier->sonlist->value;
    }else{
        if (strcmp(specifier->sonlist->sonlist->next->name,"Tag") == 0){
            //struct 定义变量
            type = specifier->sonlist->sonlist->next->sonlist->value;

        }else{
            //struct 定义,type赋值时要注意opttag可以是匿名的
            dfsStructDec(specifier->sonlist);
            //type = 
        }
    }

    //DecList 变量定义 ，这里没有函数
    struct treeNode* declist = specifier->next;
    if (strcmp(declist->name,"DecList") == 0){
        dfsDecList(declist,type,rtlist); 
    }


}
//结构体内，或函数体内 变量定义
int dfsDefList(struct treeNode* node,struct Var* rtlist){
    //node is DefList, rtlist是将这些Def插入到那个Varlist，分为varlist表和struct的feild
    struct treeNode* p = node;
    while(p != NULL && p->sonlist!=NULL){
        dfsDef(p->sonlist,rtlist);
        p = p->sonlist->next;
    }
    return 0; 
}

//third level
//全局变量定义
int dfsExtDec(struct treeNode* node,char* type){
    //node is ExtDecList
    struct treeNode* son = node->sonlist;
    if (son == NULL) return 0;
    // insert son  son is a single or array
    if (strcmp(son->sonlist->name,"ID") == 0){
        if (checkrepeat(son->sonlist->value) != 0){
            //语义错误
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",son->sonlist->lineno,son->sonlist->value);
        }else{
            varlistp->next = (struct Var*)malloc(sizeof(struct Var));
            varlistp = varlistp->next;
            varlistp->type = converttype(type);
            varlistp->tname = type;
            varlistp->defline = son->sonlist->lineno;
            varlistp->deadline = -1;
            varlistp->name = son->sonlist->value;
            varlistp->value = 0;
            varlistp->father = varlistp->next = varlistp->sonlist = NULL;
        }
    }else{
        //array
        struct treeNode* tmp = son;
        while(strcmp(tmp->sonlist->name,"ID") != 0) tmp = tmp->sonlist;
        if (checkrepeat(tmp->sonlist->value) != 0){
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",tmp->sonlist->lineno,tmp->sonlist->value);
        }else{
            varlistp->next = (struct Var*)malloc(sizeof(struct Var));
            varlistp = varlistp->next;
            varlistp->type = converttype(type)+3;
            varlistp->tname = type;
            varlistp->defline = tmp->sonlist->lineno;
            varlistp->deadline = -1;
            varlistp->name = tmp->sonlist->value;
            varlistp->value = 0;
            varlistp->arraylen = atoi(son->sonlist->next->next->value);
            varlistp->father = varlistp->next = varlistp->sonlist = NULL;
            
            struct treeNode* temp = son->sonlist->sonlist;
            struct Var* varlistq = varlistp;
            while(temp != tmp->sonlist){
                varlistq->sonlist = (struct Var*)malloc(sizeof(struct Var));
                varlistq = varlistq->sonlist;
                varlistq->type = varlistp->type;
                varlistq->tname = varlistp->tname;
                varlistq->arraylen = atoi(temp->next->next->value);
                varlistq->father = varlistq->next = varlistq->sonlist = NULL;

                temp = temp->sonlist;
            }
        }
    }

    if (son->next != NULL && strcmp(son->next->name,"COMMA") == 0){
        dfsExtDec(son->next->next,type);
    }
    return 0;
}
//函数定义
int dfsFunDec(struct treeNode* node){
    
    return 0;
}
//结构体定义
int dfsStructDec(struct treeNode* node){
    //node is StructSpecifier
    char *name = NULL;
    int type;
    if (node->sonlist->next->sonN == 0){
        //匿名结构体
        type = 2; 
    }else{
        //非匿名
        type = 1;
        name = node->sonlist->next->sonlist->value;
    } 
    if (checkrepeat_struct() == 0){
        structlistp->next = (struct Struct*)malloc(sizeof(struct Struct));
        structlistp = structlistp->next;
        structlistp->type = type;
        structlistp->name = name;
        structlistp->defline = node->lineno;
        dfsDefList(node->sonlist->next->next->next,structlistp->feild); 
    }else{

    } 
    return 0;
}


//second level，ExtDef包括函数/结构体/函数声明/全局变量定义
int dfsExtDef(struct treeNode* node){
    if (node == NULL) return 0;
    struct treeNode* son = node->sonlist;
    //Specifier
    struct treeNode* sonson = son->sonlist;
    char* type = NULL; 
    if (strcmp(sonson->name,"TYPE") == 0){
        type = sonson->value;
    }else{
        if (strcmp(sonson->sonlist->next->name,"Tag") == 0){
            //struct 定义变量
            type = sonson->sonlist->next->sonlist->value;

        }else{
            //struct 定义,type赋值时要注意opttag可以是匿名的
            dfsStructDec(sonson);
            //type = 
        }
    }

    //ExtDecList 全局变量定义 
    son = son->next;
    if (strcmp(son->name,"ExtDecList") == 0){
        dfsExtDec(son,type); 
    }
    //函数定义/声明
    if (strcmp(son->name,"FunDec") == 0){
        dfsFunDec(son); //{ insert in there to make xingcan after {
        son = son->next;
        if (strcmp(son->name,"SEMI") == 0){

        }else if (strcmp(son->name,"Compt") == 0){

        }
    }


    return 0;
}

//first level 找到ExtDef进入下一层 
int dfsbuildtable(struct treeNode* node){
    if (node == NULL) return 0;
    if (strcmp(node->name,"ExtDef") == 0){
        dfsExtDef(node);
    }else{
        struct treeNode* son = node->sonlist;
        while(son != NULL){
            dfsbuildtable(son);
            son = son->next;
        }    
    }
    return 0;
}
int checkmean(struct treeNode* root){
    //init
    varlist = (struct Var*)malloc(sizeof(struct Var));
    varlistp = varlist;
    varlist->defline = 0;
    varlist->name = malloc(sizeof("{"));
    strncpy(varlist->name,"{",sizeof("{"));
    varlist->father = varlist->sonlist = varlist->next = NULL;
     
    structlist = (struct Struct*)malloc(sizeof(struct Struct));
    structlistp = structlist;
    structlistp->defline = 0;
    structlistp->name = malloc(sizeof("s_t_a_r_t"));
    strncpy(structlistp->name,"s_t_a_r_t",sizeof("s_t_a_r_t"));
    structlistp->feild = NULL;
    structlistp->next = NULL;

    //start
    dfsbuildtable(root);
    
    return 0;
}
