#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

struct Instr *instrlist;

int printInstrList(){
    
    return 0;
}
struct Instr* generate_instr(char* place,int op,char* arg1,char* arg2){
    struct Instr* instr = (struct Instr*)malloc(sizeof(struct Instr));
    instr->target = (char*)malloc(sizeof(20));
    strcpy(instr->target,place);
    instr->op = op;
    instr->arg1 = (char*)malloc(sizeof(20));
    strcpy(instr->arg1,arg1);
    instr->arg2 = (char*)malloc(sizeof(20));
    strcpy(instr->arg2,arg2);
    instr->prev = instr->next = NULL;
    return instr;
}

int get_value(struct treeNode* node){
    //node is INT
    return atoi(node->value);
}

struct Instr* translate_Exp(struct treeNode* node,struct Varm* list,char* place){
    //node is Exp
    if (node->sonN == 1 && strcmp(node->sonlist->name,"INT") == 0){
        //Exp -> INT
        int value = get_value(node->sonlist);
        char tmp[20];
        sprintf(tmp,"#%d",value);
        return generate_instr(place,_ASSIGNOP,tmp,NULL);
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"ID") == 0){
        //Exp -> ID
        char* var = lookup(list,node->sonlist->value);
        return generate_instr(place,_ASSIGNOP,var,NULL);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"ASSIGNOP") == 0){
        //Exp -> Exp ASSIGNOP Exp
        
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"PLUS") == 0){
        //Exp -> Exp PLUS Exp
        
    }else if (strcmp(node->sonlist->name,"MINUS") == 0){
        //MINUS Exp
        
    }else if (strcmp(node->sonlist->name,"NOT") == 0 || (node->sonlist->next != NULL && (strcmp(node->sonlist->next->name,"RELOP") == 0) || strcmp(node->sonlist->next->name,"AND") == 0) || strcmp(node->sonlist->next->name,"OR") == 0))){
        //Exp -> Exp RELOP Exp | NOT Exp | Exp AND Exp | Exp OR Exp 
        
    }
    
}

int translate_ExtDef(struct treeNode* node){
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
            dfsStructDec(sonson,&type);
        }
    }
    
    //ExtDecList 全局变量定义 
    son = son->next;
    
    

    
    if (strcmp(son->name,"ExtDecList") == 0){
        //printf("type is %s\n",type);
        if (checkType(type) == 0){
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",son->lineno,type);
            return 0;
        }
        dfsExtDec(son,type); 
    }
    //函数定义/声明
    if (strcmp(son->name,"FunDec") == 0){
        //dfsFunDec to create a Func with (deflist), but do not insert to funclist.
        //rt this Func , then checkrepeat
        if (checkType(type) == 0){
            printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",son->lineno,type);
            //return 0;
        }
        struct Func *func = dfsFunDec(son); //{ insert in there to make xingcan after {
        if (strcmp(son->next->name,"SEMI") == 0){
            //函数声明
            func->type = 2;
            func->name = son->sonlist->value;
            func->rt_type = converttype(type);
            func->rt_tname = type;
            if (checkrepeat_func(func) == 0){
                func->next = funclist;
                funclist = func;
            }
        }else if (strcmp(son->next->name,"CompSt") == 0){
            func->type = 1;
            func->name = son->sonlist->value;
            func->rt_type = converttype(type);
            func->rt_tname = type;
            int rtr = checkrepeat_func(func);
            
            if (rtr == 0){
                func->next = funclist;
                funclist = func;
                //将其形参加入varlist，然后进行compt的check
                struct Var* p = func->feild;
                while(p != NULL){
                    struct Var* insert = (struct Var*)malloc(sizeof(struct Var));
                    copyVar(insert,p);
                    insert->father = NULL;
                    insert->next = varlist;
                    varlist = insert;
                    p = p->next;
                }
                //dfs CompSt
                struct RTtype rtt;
                rtt.type = func->rt_type;
                rtt.tname = func->rt_tname;
                dfsCompSt(son->next,&rtt,1);
                
                //delete 形参, 在dfsCompSt中删除
                /*p = varlist;
                struct Var* q;
                while(p!=NULL){
                    if (p->type == 7){
                        q = p->next;
                        free(p);
                        varlist = q;
                        break;
                    }
                    q = p->next;
                    free(p);
                    p = q;
                }
                */

            }else{
                if (rtr == 1){
                    printf("Error type 4 at Line %d: Redefined function \"%s\".\n",func->defline,son->sonlist->value);
                }
                //free(func->feild);
                free(func);
            }
        }
        
    }
    return 0;
}

//first level 找到ExtDef进入下一层 
int translate(struct treeNode* node){
    if (node == NULL) return 0;
    if (strcmp(node->name,"ExtDef") == 0){
        translate_ExtDef(node);
    }else{
        struct treeNode* son = node->sonlist;
        while(son != NULL){
            translate(son);
            son = son->next;
        }    
    }
    return 0;
}

int middle(struct treeNode* root){
    translate(root);
    return 0;
}