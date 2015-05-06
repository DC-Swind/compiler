#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

struct Var* varlist;
struct Var* varlistp;
struct Struct* structlist;
struct Struct* structlistp;
struct Func* funclist;
struct Func* funclistp;

//匿名结构体编号
int structname = 1;
//-----------------功能函数，暂时写在这里，后面移到另一个文件-----
int checkError18(){
    struct Func* p = funclist;
    while(p!=NULL){
        if (p->type == 2){
            struct Func* q = funclist;
            while(q!= NULL){
                if (q->type == 1 && strcmp(q->name,p->name) == 0) break;
                q = q->next;
            }
            if (q == NULL){
                //error 18
                printf("Error type 18 at Line %d: Undefined function \"%s\".\n",p->defline,p->name);
            }
        }
        p = p->next;
    }
    return 0;
}

int converttype(char* type){
    if (strcmp(type,"int") == 0 || strcmp(type,"INT") == 0) return 1;
    if (strcmp(type,"float") == 0 || strcmp(type,"FLOAT") == 0) return 2;
    return 3;
}
//varlist符号表查repeat
int checkrepeat(char *name){
    struct Var* p = varlist;
    while(p != NULL){
        if (/*p->type == type && */strcmp(p->name,name) == 0) return 1;
        p = p->next;
    }
    return 0;
}
//structlist结构体表查repeat
int checkrepeat_struct(){
    return 0;
}
//structfeild符号表查repeat
int checkrepeat_inlist(char* name,struct Var* list){
    if (list == varlist){
        struct Func* func = funclist;
        while(func!=NULL){
            if (strcmp(func->name,name) == 0) return 1;
            func = func->next;
        }
        struct Struct* pp = structlist;
        while(pp != NULL){
            if (strcmp(pp->name,name) == 0) return 1;
            pp = pp->next;
        }
    }
    
    struct Var* q = list;
    struct Var* p = list;
    while (q != NULL){
        if (q->type == 7) p = q;
        q = q->next;
    }
    
    while(p != NULL){
        if (strcmp(p->name,name) == 0) return 1;
        p = p->next;
    }
    return 0;
}

int checkrepeat_func(struct Func *func){
    struct Func* p = funclist;
    while(p != NULL){
        //已有定义，本次也是定义，且名字相同
        if (strcmp(p->name,func->name) == 0 && p->type == 1 && func->type == 1) return 1;
        
        //已有定义，本次是声明  或 已有声明
        if (strcmp(p->name,func->name) == 0){
            if (p->rt_type != func->rt_type){
                printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",func->defline,func->name);
                return 1;
            }
            struct Var* list1 = func->feild;
            struct Var* list2 = p->feild;
            while(list1 != NULL && list2 != NULL){
                if (list1->type != list2->type){
                    printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",func->defline,func->name);
                    return 1;
                }
                list1 = list1->next;
                list2 = list2->next;
            }
            if (list1 != NULL || list2 != NULL){
                printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",func->defline,func->name);
                return 1;   
            }
        }
        p = p->next;
    }
    struct Var* q = varlist;
    while(q != NULL){
        if (strcmp(q->name,func->name) == 0) return 1;
        q = q->next;
    }
    return 0;
}
//----------------------------------------------------------------
//forth level

int dfsDecList(struct treeNode* node,char* type,struct Var* rtlist){
    //node is DecList,rtlist 是将这些Dec插入到哪个Varlist
    //这个跟dfsExtDec差不多，不过注意赋值
    //Dec要考虑 ASSIGNOP 的问题，Dec还没有处理完， 而且嵌套作用域问题也没有解决

    //找到list中最后一个元素
    if (rtlist == NULL) return 0;
    struct Var* rtlistp = rtlist;
    while(rtlistp->next !=NULL) rtlistp = rtlistp->next;
    
    struct treeNode* dec = node->sonlist;
    if (dec == NULL) return 0;
    // insert dec  dec is a single or array
    if (strcmp(dec->sonlist->sonlist->name,"ID") == 0){
        if (checkrepeat_inlist(dec->sonlist->sonlist->value,rtlist) != 0){
            //语义错误
            printf("Error type x at Line %d: Redefined variable \"%s\" in struct.\n",dec->sonlist->sonlist->lineno,dec->sonlist->sonlist->value);
        }else{
            rtlistp->next = (struct Var*)malloc(sizeof(struct Var));
            rtlistp = rtlistp->next;
            rtlistp->type = converttype(type);
            rtlistp->tname = type;
            rtlistp->defline = dec->sonlist->sonlist->lineno;
            rtlistp->deadline = -1;
            rtlistp->name = dec->sonlist->sonlist->value;
            rtlistp->value = 0;
            if (dec->sonlist->next != NULL && strcmp(dec->sonlist->next->name,"ASSIGNOP")==0){
                if (rtlist->type == 1) rtlistp->value = atoi(dec->sonlist->next->next->sonlist->value);
                if (rtlist->type == 2) rtlistp->value = atof(dec->sonlist->next->next->sonlist->value);
}
            rtlistp->father = rtlistp->next = rtlistp->sonlist = NULL;
        }
    }else{
        //array
        struct treeNode* tmp = dec;
        while(strcmp(tmp->sonlist->name,"ID") != 0) tmp = tmp->sonlist;
        if (checkrepeat_inlist(tmp->sonlist->value,rtlist) != 0){
            printf("Error type x at Line %d: Redefined variable \"%s\".\n",tmp->sonlist->lineno,tmp->sonlist->value);
        }else{
            rtlistp->next = (struct Var*)malloc(sizeof(struct Var));
            rtlistp = rtlistp->next;
            rtlistp->type = converttype(type)+3;
            rtlistp->tname = type;
            rtlistp->defline = tmp->sonlist->lineno;
            rtlistp->deadline = -1;
            rtlistp->name = tmp->sonlist->value;
            rtlistp->value = 0;
            rtlistp->arraylen = atoi(dec->sonlist->sonlist->next->next->value);
            rtlistp->arraydimension = 1;
            rtlistp->father = rtlistp->next = rtlistp->sonlist = NULL;
            
            struct treeNode* temp = dec->sonlist->sonlist->sonlist;
            struct Var* varlistq = rtlistp;
            while(temp != tmp->sonlist){
                varlistq->sonlist = (struct Var*)malloc(sizeof(struct Var));
                varlistq = varlistq->sonlist;
                varlistq->type = varlistp->type;
                varlistq->tname = varlistp->tname;
                varlistq->arraylen = atoi(temp->next->next->value);
                varlistq->father = varlistq->next = varlistq->sonlist = NULL;
                rtlistp->arraydimension++;
                temp = temp->sonlist;
            }
        }
    }
    if (dec->next != NULL && strcmp(dec->next->name,"COMMA") == 0){
        dfsDecList(dec->next->next,type,rtlist);
    }
    return 0;
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
            dfsStructDec(specifier->sonlist,&type);
            //type = 
        }
    }

    //DecList 变量定义 ，这里没有函数
    struct treeNode* declist = specifier->next;
    if (strcmp(declist->name,"DecList") == 0){
        dfsDecList(declist,type,rtlist); 
    }

    return 0;
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
            varlistp->arraydimension = 1;
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
                varlistp->arraydimension++;

                temp = temp->sonlist;
            }
        }
    }

    if (son->next != NULL && strcmp(son->next->name,"COMMA") == 0){
        dfsExtDec(son->next->next,type);
    }
    return 0;
}

int isAarray(struct treeNode *node){
    //node is a Exp
    if (strcmp(node->sonlist->name,"Exp") == 0)
        if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"LB") == 0){
            return 1;
        }
    return 0;
}
int checkDefine(struct treeNode* node,char *rtname,int *rttype,struct Var* list){
    //node is Exp, rewrite
    char tmp[30];
    if (rtname == NULL) rtname = tmp;
    
    if (node->sonN == 1){
        strncpy(rtname,node->sonlist->value,sizeof(node->sonlist->value));
        struct Var* p = list;
        int rt = 0;
        while(p!= NULL){
            if (strcmp(p->name,node->sonlist->value) == 0){
                rt = 1;        
                *rttype = p->type;
            }
            p = p->next;
        }
        return rt;
    }else{
        //is a array,need get dimensions and name
        struct treeNode* arr = node;
        int dimensions = 0;
        while(arr != NULL && strcmp(arr->name,"Exp") == 0){
            //check error 12,[..] not a int
            if (arr->next != NULL && strcmp(arr->next->name,"LB") == 0){
                int type = dfsExp(arr->next->next);
                if (type != _INT && type != _NUMINT){
                    printf("Error type 12 at Line %d: \"%s\" is not an integer.\n",arr->lineno,arr->next->next->name);
                }
            }
            dimensions++;
            arr = arr->sonlist;
        }
        dimensions--;
        strncpy(rtname,arr->value,sizeof(arr->value));
        struct Var* p = list;
        int rt = 0;
        while(p!=NULL){
            if (strcmp(p->name,rtname) == 0){
                if (p->type == 4 || p->type == 5 || p->type == 6){
                    if (dimensions < p->arraydimension){
                        rt = 1;
                        *rttype = p->type;
                    }else if (dimensions == p->arraydimension){
                        rt = 1;
                        *rttype = p->type - 3;
                    }
                }
            }
            p = p->next;
        }
        return rt;
    }
}

int checkFuncDefine(struct treeNode* node,int *type){
    //node is Exp,sonlist is ID,sonlist->next is LP	printf("Enter checkFuncDefine.\n");
    struct Func* p = funclist;
    if (strcmp(node->sonlist->next->next->name,"RP") == 0){
        //ID LP RP	
        while(p!=NULL){
            if (strcmp(p->name,node->sonlist->value) == 0){
                if (p->feild == NULL){
                    *type = p->rt_type;
                    return 1;
                }
            }
            p = p->next;
        }
        return 0;
    }else{
        //ID LP Args RP
        struct treeNode *args = node->sonlist->next->next;
        while(p!=NULL){
            if (strcmp(p->name,node->sonlist->value) == 0){
                struct Var* q = p->feild->next;
                while(q != NULL && args != NULL){
                    int argtype = dfsExp(args->sonlist);
                    if (argtype == q->type || (argtype == _NUMINT && q->type == _INT) || (argtype == _NUMFLOAT && q->type == _FLOAT)){
                       
                    }else{
                        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",args->lineno,node->sonlist->value);
                        return 1;
                    }
                    /*
                    if (strcmp(args->sonlist->sonlist->name,"ID") == 0 || strcmp(args->sonlist->sonlist->name,"Exp") == 0){
                        if (checkDefine(args->sonlist,NULL,&type,varlist)){
                            if (type != q->type){
                                printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",args->lineno,node->sonlist->value);
                                return 1;
                            }
                        }else{
                            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",args->lineno,args->sonlist->sonlist->value);
                            return 1;
                        }
                    }else{
                        //int or float
                        type = converttype(args->sonlist->sonlist->name);
                        if (type != q->type){
                            printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",args->lineno,node->sonlist->value);
                            return 1;
                        }
                    }
                    */
                    q = q->next;
                    if (args->sonlist->next != NULL)args = args->sonlist->next->next; else args=NULL;
                }
                if (q == NULL && args == NULL){
                    *type = p->rt_type;
                    return 1;
                }else{
                    printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",args->lineno,node->sonlist->value);
                    return 1;
                }
            }
            p = p->next;
        }
        //没有找到函数
        return 0;
    }
}
int dfsExp(struct treeNode* node){
    //node is Exp
    //printf("lineno:%d\n",node->lineno);
    
    if (node->sonN == 1 && strcmp(node->sonlist->name,"ID") == 0){
        //Exp -> ID
        int type;
        char name[30];
        if (checkDefine(node,name,&type,varlist)){
            return type;
        }else{
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->lineno,name);
        }
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"INT") == 0){
        //Exp -> INT
        return _NUMINT;
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"FLOAT") == 0){
        //Exp -> FLOAT
        return _NUMFLOAT;
    }else if (strcmp(node->sonlist->name,"ID")==0 && strcmp(node->sonlist->next->name,"LP") == 0){
        //Exp ->ID LP (Args) RP
        int type;
        if (checkFuncDefine(node,&type)){
            return type;
        }else printf("Error type 2 at Line %d: Undefined function \"%s\".\n",node->lineno,node->sonlist->value);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"ASSIGNOP") == 0){
        //Exp ASSIGNOP Exp
        int ltype;
        if (strcmp(node->sonlist->sonlist->name,"ID") == 0 && node->sonlist->sonN == 1){
            //id
            struct treeNode* id = node->sonlist->sonlist;
            int rtype = dfsExp(node->sonlist->next->next);
            if (checkDefine(node->sonlist,NULL,&ltype,varlist)){
                //check type
                if (rtype == ltype || rtype == ltype + 7){
                
                }else{
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n",id->lineno);
                }
            }else{
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",id->lineno,id->value);
            }
        }else if (strcmp(node->sonlist->sonlist->name,"Exp") == 0 && isAarray(node->sonlist)){
            //array
            char name[100];
            int rtype = dfsExp(node->sonlist->next->next);
            if (checkDefine(node->sonlist,name,&ltype,varlist)){
                //check type
                if (rtype == ltype || rtype == ltype + 7){
                    
                }else{
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n",node->lineno);
                }
            }else{
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->sonlist->lineno,name);
            }
        }else{
            //error
            printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",node->lineno);
        }
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"AND") == 0){
        //Exp AND Exp
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);

        if ((ltype == _NUMINT || ltype == _INT)&&(rtype == _NUMINT || rtype == _INT)) return _NUMINT;
        //error
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"OR") == 0){
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);
        
        if ((ltype == _NUMINT || ltype == _INT)&&(rtype == _NUMINT || rtype == _INT)) return _NUMINT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"RELOP") == 0){
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);
        
        if ((ltype == _INT || ltype == _NUMINT)&&(rtype == _INT || rtype == _NUMINT)) return _NUMINT;
        if ((ltype == _FLOAT || ltype == _NUMFLOAT)&&(rtype == _FLOAT || rtype == _NUMFLOAT)) return _NUMFLOAT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"PLUS") == 0){
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);
        
        if ((ltype == _INT || ltype == _NUMINT)&&(rtype == _INT || rtype == _NUMINT)) return _NUMINT;
        if ((ltype == _FLOAT || ltype == _NUMFLOAT)&&(rtype == _FLOAT || rtype == _NUMFLOAT)) return _NUMFLOAT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"MINUS") == 0){
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);
        if ((ltype == _INT || ltype == _NUMINT)&&(rtype == _INT || rtype == _NUMINT)) return _NUMINT;
        if ((ltype == _FLOAT || ltype == _NUMFLOAT)&&(rtype == _FLOAT || rtype == _NUMFLOAT)) return _NUMFLOAT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"STAR") == 0){
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);
        if ((ltype == _INT || ltype == _NUMINT)&&(rtype == _INT || rtype == _NUMINT)) return _NUMINT;
        if ((ltype == _FLOAT || ltype == _NUMFLOAT)&&(rtype == _FLOAT || rtype == _NUMFLOAT)) return _NUMFLOAT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"DIV") == 0){
        int ltype = dfsExp(node->sonlist);
        int rtype = dfsExp(node->sonlist->next->next);
       if ((ltype == _INT || ltype == _NUMINT)&&(rtype == _INT || rtype == _NUMINT)) return _NUMINT;
        if ((ltype == _FLOAT || ltype == _NUMFLOAT)&&(rtype == _FLOAT || rtype == _NUMFLOAT)) return _NUMFLOAT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist != NULL && strcmp(node->sonlist->name,"MINUS") == 0){
        int ltype = dfsExp(node->sonlist->next);
        if (ltype == _INT || ltype == _NUMINT) return _NUMINT;
        if (ltype == _FLOAT || ltype == _NUMFLOAT) return _NUMFLOAT;
        
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist != NULL && strcmp(node->sonlist->name,"NOT") == 0){
        int ltype = dfsExp(node->sonlist->next);
        if (ltype == _INT || ltype == _NUMINT) return _NUMINT;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist !=NULL && strcmp(node->sonlist->name,"LP") == 0){
        //LP Exp RP  (Exp)
        int ltype = dfsExp(node->sonlist->next);
        if (ltype == _INT) return _NUMINT;
        if (ltype == _FLOAT) return _NUMFLOAT;
        return ltype;
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"LB") == 0){
        //Exp LB Exp RB
        char name[30];
        int type;
        if (checkDefine(node,name,&type,varlist)){
            return type;
        }else{
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->lineno,name);
        }
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"DOT") == 0){
        //Exp DOT ID
        
    }

    return -1;
}

int dfsStmt(struct treeNode* node){
    //node is Stmt 
    struct treeNode *stmt = node;
    int rttype = -1;
    if (strcmp(stmt->sonlist->name,"Exp") == 0){
            //Stmt -> Exp SEMI
            dfsExp(stmt->sonlist);
        }else if (strcmp(stmt->sonlist->name,"CompSt") == 0){
            //Stmt -> CompSt
            dfsCompSt(stmt->sonlist,-1);
        }else if (strcmp(stmt->sonlist->name,"RETURN") == 0){
            //Stmt -> RETURN Exp SEMI
            rttype = dfsExp(stmt->sonlist->next);
        }else if (strcmp(stmt->sonlist->name,"IF") == 0){
            //Stmt -> IF LP Exp RP Stmt 
            //Stmt -> IF LP Exp RP Stmt ELSE Stmt
            dfsExp(stmt->sonlist->next->next);
            dfsStmt(stmt->sonlist->next->next->next->next);
            if (stmt->sonlist->next->next->next->next->next != NULL)dfsStmt(stmt->sonlist->next->next->next->next->next->next);
        }else{
            //Stmt -> WHILE LP Exp RP Stmt
            dfsExp(stmt->sonlist->next->next);
            dfsStmt(stmt->sonlist->next->next->next->next);
        }
    return rttype;
}

int dfsStmtList(struct treeNode* node,int rt_type){
    //node is StmtList
    struct treeNode* stmt = node->sonlist;

    //处理Stmt中各类语句错误
    while(stmt != NULL){
        int rt = dfsStmt(stmt);
        if (rt_type != -1 && rt != -1 && rt != rt_type){
            if ((rt_type == _INT && rt == _NUMINT)||(rt_type == _FLOAT && rt == _NUMFLOAT)){
                
            }else{
                printf("Error type 8 at Line %d: Type mismatched for return.\n",stmt->lineno);
            }
        }
        //next is stmtlist
        stmt = stmt->next->sonlist;
    }
    return 0;
}

int dfsCompSt(struct treeNode* node,int rt_type){
    //node is CompSt
    //insert a {
    varlistp->next = (struct Var*)malloc(sizeof(struct Var));
    varlistp = varlistp->next;
    varlistp->type = 7;
    varlistp->name = malloc(sizeof("{"));
    strncpy(varlistp->name,"{",sizeof("{"));
    varlistp->defline = node->lineno;
    varlistp->father = varlistp->sonlist = varlistp->next = NULL;
    
    dfsDefList(node->sonlist->next,varlist);
    dfsStmtList(node->sonlist->next->next,rt_type);
    //meet } , remove untill a {
    struct Var* p = varlist;
    struct Var* q = varlist;
    struct Var* qpre = varlist;
    while(p!=NULL){
        if (p->next != NULL && p->next->type == 7){
            qpre = p;
        }
        if (p->type == 7){
            q = p;
        }
        p = p->next;
    }
    qpre->next = NULL;
    while(q!=NULL){
        qpre = q;
        q = q->next;
        free(qpre);
    }
    return 0;
}


//函数定义或声明，创建一个Func，将形参提取
struct Func* dfsFunDec(struct treeNode* node){
    //node is FunDec
    struct Func* func = (struct Func*)malloc(sizeof(struct Func));
    func->defline = node->lineno;
    //init a feild
    func->feild = (struct Var*)malloc(sizeof(struct Var));
    struct Var* feildlistp = func->feild;
    feildlistp->type = 7;
    feildlistp->name = malloc(sizeof("{"));
    strncpy(feildlistp->name,"{",sizeof("{"));
        
    func->next = NULL;
    char *name = node->sonlist->value; //ID:main
    if (strcmp("RP",node->sonlist->next->next->name) == 0){
        //没有形参
    }else{
        //dfsVarList(node->sonlist->next->next);
        struct treeNode* varlist = node->sonlist->next->next;
        while(varlist != NULL){
            char* type = varlist->sonlist->sonlist->sonlist->value;
            //son is VarDec
            struct treeNode* son = varlist->sonlist->sonlist->next;
            
            // insert son  son is a single or array
            if (strcmp(son->sonlist->name,"ID") == 0){
                if (checkrepeat_inlist(son->sonlist->value,func->feild) != 0){
                    //语义错误
                    printf("Error type 3 at Line %d: Redefined variable \"%s\" in function().\n",son->sonlist->lineno,son->sonlist->value);
                }else{
                    feildlistp->next = (struct Var*)malloc(sizeof(struct Var));
                    feildlistp = feildlistp->next;
                    feildlistp->type = converttype(type);
                    feildlistp->tname = type;
                    feildlistp->defline = son->sonlist->lineno;
                    feildlistp->deadline = -1;
                    feildlistp->name = son->sonlist->value;
                    feildlistp->value = 0;
                    feildlistp->father = feildlistp->next = feildlistp->sonlist = NULL;
                }
            }else{
                //array
                struct treeNode* tmp = son;
                while(strcmp(tmp->sonlist->name,"ID") != 0) tmp = tmp->sonlist;
                if (checkrepeat_inlist(tmp->sonlist->value,func->feild) != 0){
                    printf("Error type 3 at Line %d: Redefined variable \"%s\" in function().\n",tmp->sonlist->lineno,tmp->sonlist->value);
                }else{
                    feildlistp->next = (struct Var*)malloc(sizeof(struct Var));
                    feildlistp = feildlistp->next;
                    feildlistp->type = converttype(type)+3;
                    feildlistp->tname = type;
                    feildlistp->defline = tmp->sonlist->lineno;
                    feildlistp->deadline = -1;
                    feildlistp->name = tmp->sonlist->value;
                    feildlistp->value = 0;
                    feildlistp->arraylen = atoi(son->sonlist->next->next->value);
                    feildlistp->father = feildlistp->next = feildlistp->sonlist = NULL;
            
                    struct treeNode* temp = son->sonlist->sonlist;
                    struct Var* feildlistq = feildlistp;
                    while(temp != tmp->sonlist){
                        feildlistq->sonlist = (struct Var*)malloc(sizeof(struct Var));
                        feildlistq = feildlistq->sonlist;
                        feildlistq->type = feildlistp->type;
                        feildlistq->tname = feildlistp->tname;
                        feildlistq->arraylen = atoi(temp->next->next->value);
                        feildlistq->father = feildlistq->next = feildlistq->sonlist = NULL;

                        temp = temp->sonlist;
                    }
                }
            }
            //next varlist
            if (varlist->sonlist->next == NULL) varlist = NULL;else varlist = varlist->sonlist->next->next;
        }
    }

    return func;
}
//结构体定义
int dfsStructDec(struct treeNode* node,char **typename){
    //node is StructSpecifier
    char *name = NULL;
    int type;
    if (node->sonlist->next->sonN == 0){
        //匿名结构体
        type = 2; 
        name = (char*)malloc(sizeof(char)*20);
        strncpy(name,"$struct$",sizeof("$struct$"));
        sprintf(name,"%s%d",name,structname);
        structname += 1;
    }else{
        //非匿名
        type = 1;
        name = node->sonlist->next->sonlist->value;
    }
    *typename = name;
    if (checkrepeat_struct() == 0){
        structlistp->next = (struct Struct*)malloc(sizeof(struct Struct));
        structlistp = structlistp->next;
        structlistp->type = type;
        structlistp->name = name;
        structlistp->defline = node->lineno;
        structlistp->feild = (struct Var*)malloc(sizeof(struct Var));
        //feild初始化一个开始标记为{
        structlistp->feild->type = 7;
        structlistp->feild->name = malloc(sizeof("{"));
        strncpy(structlistp->feild->name,"{",sizeof("{"));
        structlistp->feild->father = structlistp->feild->sonlist = structlistp->feild->next = NULL;

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
            dfsStructDec(sonson,&type);
        }
    }

    //ExtDecList 全局变量定义 
    son = son->next;
    if (strcmp(son->name,"ExtDecList") == 0){
        //printf("type is %s\n",type);
        dfsExtDec(son,type); 
    }
    //函数定义/声明
    if (strcmp(son->name,"FunDec") == 0){
        //dfsFunDec to create a Func with (deflist), but do not insert to funclist.
        //rt this Func , then checkrepeat
        struct Func *func = dfsFunDec(son); //{ insert in there to make xingcan after {
        if (strcmp(son->next->name,"SEMI") == 0){
            //函数声明,先插入funclistp,在最后进行check
            func->type = 2;
            func->name = son->sonlist->value;
            func->rt_type = converttype(type);
            func->rt_tname = type;
            if (checkrepeat_func(func) == 0){
                funclistp->next = func;
                funclistp = funclistp->next;
            }
        }else if (strcmp(son->next->name,"CompSt") == 0){
            func->type = 1;
            func->name = son->sonlist->value;
            func->rt_type = converttype(type);
            func->rt_tname = type;
            if (checkrepeat_func(func) == 0){

                funclistp->next = func;
                funclistp = funclistp->next;
                //将其形参加入varlist，然后进行compt的check
                struct Var* p = func->feild;
                if (p != NULL)
                    while(p->next != NULL) p = p->next;
                varlistp->next = func->feild;
                varlistp = p;
                
                //dfs CompSt
                dfsCompSt(son->next,func->rt_type);

            }else{
                printf("Error type 4 at Line %d: Redefined function \"%s\".\n",func->defline,son->sonlist->value);
            }
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

    funclist = (struct Func*)malloc(sizeof(struct Func));
    funclistp = funclist;
    funclistp->defline = 0;
    funclistp->name = malloc(sizeof("s_t_a_r_t"));
    strncpy(funclistp->name,"s_t_a_r_t",sizeof("s_t_a_r_t"));
    funclistp->type = 1;
    funclistp->feild = NULL;
    funclistp->next = NULL;
    
    //start
    dfsbuildtable(root);
    
    checkError18();
    return 0;
}
