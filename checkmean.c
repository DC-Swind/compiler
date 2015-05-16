#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

struct Var* varlist;
struct Struct* structlist;
struct Func* funclist;
struct RTtype dfsExp(struct treeNode* node);
//匿名结构体编号
int structname = 1;
//-----------------功能函数，暂时写在这里，后面移到另一个文件-----
int checkType(char *type){
    if (strcmp(type,"int") == 0) return 1;
    else if (strcmp(type,"float") == 0) return 1;
    struct Struct* p = structlist;
    while(p!=NULL){
        if (strcmp(type,p->name) == 0) return 1;
        p = p->next;
    }
    return 0;
}
int copyVar(struct Var*insert, struct Var*p){
     insert->type = p->type;
     insert->tname = p->tname;
     insert->defline = p->defline;
     insert->deadline = p->deadline;
     insert->name = p->name;
     insert->value = p->value; 
     insert->arraylen = p->arraylen;
     insert->arraydimension = p->arraydimension;
     insert->sonlist = p->sonlist;
     return 0;
}
int printlist(struct Var* list){
    while(list != NULL){
        printf("%s(%d),",list->name,list->defline);
        list = list->next;
    }
    printf("\n");
    return 0;
}
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
    struct Struct* q = structlist;
    while(q != NULL){
        if (strcmp(q->name,name) == 0) return 1;
        q = q->next;
    }
    return 0;
}
//structlist结构体表查repeat
int checkrepeat_struct(struct Struct* q){
    struct Struct* p = structlist;
    while(p != NULL){
        if (strcmp(p->name,q->name) == 0)return 1;
        /*
        struct Var* varp = p->feild;
        struct Var* varq = q->feild;
        while(varp!=NULL && varq!=NULL){
            //printf("%s - %s\n",varp->tname,varq->tname);
            if (strcmp(varp->tname,varq->tname) != 0) break;
            varq = varq->next;
            varp = varp->next;
        }
        if (varq == NULL && varp == NULL) return 1;
        */
        p = p->next;
    }
    struct Var* pp = varlist;
    while(pp != NULL){
        if (strcmp(pp->name,q->name) == 0) return 1;
        pp = pp->next;
    }
    return 0;
}
int compare_struct(char* pname,char* qname){
    if (strcmp(pname,qname) == 0) return 1;
    struct Struct* p = NULL;
    struct Struct* q = NULL;
    struct Struct* list = structlist;
    while(list != NULL){
        if (strcmp(list->name,pname) == 0) p = list;
        if (strcmp(list->name,qname) == 0) q = list;
        list = list->next;
    }
    if (p == NULL || q == NULL){
        printf("q or p can not find.\n");
        exit(-1);
    }
    struct Var* varp = p->feild;
    struct Var* varq = q->feild;
    
    while(varp!=NULL && varq!=NULL){
        //printf("%s - %s\n",varp->tname,varq->tname);
        if (strcmp(varp->tname,varq->tname) != 0) break;
        varq = varq->next;
        varp = varp->next;
    }
    if (varq == NULL && varp == NULL) return 1;
      
    return 0;
}
//structfeild符号表查repeat
int checkrepeat_inlist(char* name,struct Var* list){
    //printlist(varlist);
    if (list == varlist){
        /*
        struct Func* func = funclist;
        while(func!=NULL){
            if (strcmp(func->name,name) == 0) return 1;
            func = func->next;
        }*/
        struct Struct* pp = structlist;
        while(pp != NULL){
            if (strcmp(pp->name,name) == 0) return 1;
            pp = pp->next;
        }
        
        struct Var* l = varlist;
        while(l!=NULL){
            if (strcmp(l->name,name) == 0) return 1;
            if (l->type == 7) break;
            l = l->next;
        }
        return 0;
    }
    
    //找到最后一个{
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
                return 2;
            }
            struct Var* list1 = func->feild;
            struct Var* list2 = p->feild;
            while(list1 != NULL && list2 != NULL){
                if (list1->type != list2->type){
                    printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",func->defline,func->name);
                    return 2;
                }
                list1 = list1->next;
                list2 = list2->next;
            }
            if (list1 != NULL || list2 != NULL){
                printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",func->defline,func->name);
                return 2;   
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

int dfsDecList(struct treeNode* node,char* type,struct Var* rtlist,int struct_flag){
    int rtlistisvarlist = 1;
    if (rtlist != varlist){
        rtlistisvarlist = 0;
        /*
        printf("dfsDecList error, varlist is stack but other is linklist.\n ");
        exit(-1);
        */
    }
    //node is DecList,rtlist 是将这些Dec插入到哪个Varlist
    //这个跟dfsExtDec差不多，不过注意赋值
    
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
            if (rtlist == varlist){
                printf("Error type 3 at Line %d: Redefined variable \"%s\" .\n",dec->sonlist->sonlist->lineno,dec->sonlist->sonlist->value);

            }else{ 
                printf("Error type 15 at Line %d: Redefined field \"%s\".\n",dec->sonlist->sonlist->lineno,dec->sonlist->sonlist->value);
                return -1;
            }
        }else{
            if (rtlist == varlist){
                rtlistp = (struct Var*)malloc(sizeof(struct Var));
                rtlistp->type = converttype(type);
                rtlistp->tname = type;
                rtlistp->defline = dec->sonlist->sonlist->lineno;
                rtlistp->deadline = -1;
                rtlistp->name = dec->sonlist->sonlist->value;
                rtlistp->value = 0;
                rtlistp->father = rtlistp->next = rtlistp->sonlist = NULL;
                rtlistp->next = varlist;
                varlist = rtlistp;
            }else{
                rtlistp->next = (struct Var*)malloc(sizeof(struct Var));
                rtlistp = rtlistp->next;
                rtlistp->type = converttype(type);
                rtlistp->tname = type;
                rtlistp->defline = dec->sonlist->sonlist->lineno;
                rtlistp->deadline = -1;
                rtlistp->name = dec->sonlist->sonlist->value;
                rtlistp->value = 0;
                rtlistp->father = rtlistp->next = rtlistp->sonlist = NULL;
            }
            
            if (dec->sonlist->next != NULL && strcmp(dec->sonlist->next->name,"ASSIGNOP")==0){
                /*
                if (rtlist->type == 1) rtlistp->value = atoi(dec->sonlist->next->next->sonlist->value);
                if (rtlist->type == 2) rtlistp->value = atof(dec->sonlist->next->next->sonlist->value);
                */
                if (struct_flag == 1){
                    printf("Error type 15 at Line %d: Assignment in struct define.\n",rtlistp->defline);
                    return -1;
                }else{
                    
                    struct RTtype rtype = dfsExp(dec->sonlist->next->next);
                    if (rtlistp->type == _INT && (rtype.type == _INT || rtype.type == _NUMINT)){
                    
                    }else if (rtlistp->type == _FLOAT && (rtype.type == _FLOAT || rtype.type == _NUMFLOAT)){
                    
                    }else if (rtlistp->type == _STRUCT && rtype.type == _STRUCT && compare_struct(rtype.tname,rtlistp->tname)){
                        
                    }else{
                        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",rtlistp->defline);
                    } 
                    
                }
            }
            
        }
    }else{
        //array
        struct treeNode* tmp = dec;
        while(strcmp(tmp->sonlist->name,"ID") != 0) tmp = tmp->sonlist;
        if (checkrepeat_inlist(tmp->sonlist->value,rtlist) != 0){
            if (rtlist == varlist)
                printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",tmp->sonlist->lineno,tmp->sonlist->value);
            else {
                printf("Error type 15 at Line %d: Redefined field \"%s\".\n",tmp->sonlist->lineno,tmp->sonlist->value);
                return -1;
            }
        }else{
            if (rtlist == varlist){
                rtlistp = (struct Var*)malloc(sizeof(struct Var));
                rtlistp->type = converttype(type)+3;
                rtlistp->tname = type;
                rtlistp->defline = tmp->sonlist->lineno;
                rtlistp->deadline = -1;
                rtlistp->name = tmp->sonlist->value;
                rtlistp->value = 0;
                rtlistp->arraylen = atoi(dec->sonlist->sonlist->next->next->value);
                rtlistp->arraydimension = 1;
                rtlistp->father = rtlistp->next = rtlistp->sonlist = NULL;
                rtlistp->next = varlist;
                varlist = rtlistp;
                //printf("rtlistp->name:%s , rtlistp->arraydimension: %d\n",rtlistp->name,rtlistp->arraydimension);
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
            }
          
            struct treeNode* temp = dec->sonlist->sonlist->sonlist;
            struct Var* varlistq = rtlistp;
            while(temp != tmp->sonlist){
                varlistq->sonlist = (struct Var*)malloc(sizeof(struct Var));
                varlistq = varlistq->sonlist;
                varlistq->type = rtlistp->type;
                varlistq->tname = rtlistp->tname;
                varlistq->arraylen = atoi(temp->next->next->value);
                varlistq->father = varlistq->next = varlistq->sonlist = NULL;
                rtlistp->arraydimension++;
                temp = temp->sonlist;
            }
            
            if (dec->sonlist->next != NULL && strcmp(dec->sonlist->next->name,"ASSIGNOP")==0){
                /*
                if (rtlist->type == 1) rtlistp->value = atoi(dec->sonlist->next->next->sonlist->value);
                if (rtlist->type == 2) rtlistp->value = atof(dec->sonlist->next->next->sonlist->value);
                */
                if (struct_flag == 1){
                    printf("Error type 15 at Line %d: Assignment in struct define.\n",rtlistp->defline);
                    return -1;
                }else{
                
                    struct RTtype rtype = dfsExp(dec->sonlist->next->next);
                    if (rtlistp->type == _INT && (rtype.type == _INT || rtype.type == _NUMINT)){
                    
                    }else if (rtlistp->type == _FLOAT && (rtype.type == _FLOAT || rtype.type == _NUMFLOAT)){
                    
                    }else if (rtlistp->type == _STRUCT && rtype.type == _STRUCT && compare_struct(rtype.tname,rtlistp->tname)){
                        
                    }else if (rtlistp->type == _INTARRAY && rtype.type == _INTARRAY && rtlistp->arraydimension == rtype.dimension){
                        
                    }else if (rtlistp->type == _FLOATARRAY && rtype.type == _FLOATARRAY && rtlistp->arraydimension == rtype.dimension){
                        
                    }else if (rtlistp->type == _STRUCTARRAY && rtype.type == _STRUCTARRAY && rtlistp->arraydimension == rtype.dimension && compare_struct(rtype.tname,rtlistp->tname)){
                        
                    }else{
                        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",rtlistp->defline);
                    }
                }
            }
            
            
        }
    }
    int rt = 0;
    if (dec->next != NULL && strcmp(dec->next->name,"COMMA") == 0){
        if (rtlistisvarlist) rt = dfsDecList(dec->next->next,type,varlist,struct_flag);
        else rt = dfsDecList(dec->next->next,type,rtlist,struct_flag);
    
        if (rt == -1) return rt;
    }
    return 0;
}
int dfsDef(struct treeNode* node,struct Var* rtlist,int struct_flag){
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
    
    if (checkType(type) == 0){
        printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",declist->lineno,type);
        return 0;
    }
    
    if (strcmp(declist->name,"DecList") == 0){
        int rt = dfsDecList(declist,type,rtlist,struct_flag); 
        if (rt == -1) return -1;
    }

    return 0;
}
//结构体内，或函数体内 变量定义
int dfsDefList(struct treeNode* node,struct Var* rtlist,int struct_flag){
    //node is DefList, rtlist是将这些Def插入到那个Varlist，分为varlist表和struct的feild
    int rt = 0;
    int rttmp = 0;
    struct treeNode* p = node;
    if (rtlist == varlist){
        while(p != NULL && p->sonlist!=NULL){
            rttmp = dfsDef(p->sonlist,varlist,struct_flag);
            if (rttmp == -1) rt = -1;
            p = p->sonlist->next;
        }
    }else{
        while(p != NULL && p->sonlist!=NULL){
            rttmp = dfsDef(p->sonlist,rtlist,struct_flag);
            if (rttmp == -1) rt = -1;
            p = p->sonlist->next;
        }
    }
    return rt; 
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
            struct Var* varlistp = (struct Var*)malloc(sizeof(struct Var));
            varlistp->type = converttype(type);
            varlistp->tname = type;
            varlistp->defline = son->sonlist->lineno;
            varlistp->deadline = -1;
            varlistp->name = son->sonlist->value;
            varlistp->value = 0;
            varlistp->father = varlistp->next = varlistp->sonlist = NULL;
            varlistp->next = varlist;
            varlist = varlistp;
        }
    }else{
        //array
        struct treeNode* tmp = son;
        while(strcmp(tmp->sonlist->name,"ID") != 0) tmp = tmp->sonlist;
        if (checkrepeat(tmp->sonlist->value) != 0){
            printf("Error type 3 at Line %d: Redefined variable \"%s\".\n",tmp->sonlist->lineno,tmp->sonlist->value);
        }else{
            struct Var* varlistp = (struct Var*)malloc(sizeof(struct Var));
            varlistp->type = converttype(type)+3;
            varlistp->tname = type;
            varlistp->defline = tmp->sonlist->lineno;
            varlistp->deadline = -1;
            varlistp->name = tmp->sonlist->value;
            varlistp->value = 0;
            varlistp->arraylen = atoi(son->sonlist->next->next->value);
            varlistp->arraydimension = 1;
            varlistp->father = varlistp->next = varlistp->sonlist = NULL;
            varlistp->next = varlist;
            varlist = varlistp;
            
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
int checkDefine(struct treeNode* node,char *rtname,struct RTtype *rttype,struct Var* list){
    //node is Exp
    char tmp[30];
    if (rtname == NULL) rtname = tmp;
    
    if (node->sonN == 1){
        strncpy(rtname,node->sonlist->value,sizeof(node->sonlist->value));
        struct Var* p = list;
        int rt = 0;
        while(p!= NULL){
            if (strcmp(p->name,node->sonlist->value) == 0){
                rt = 1;        
                rttype->type = p->type;
                rttype->tname = p->tname;
                rttype->dimension = p->arraydimension;
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
                struct RTtype type = dfsExp(arr->next->next);
                if (type.type != _INT && type.type != _NUMINT){
                    printf("Error type 12 at Line %d: array subscript must be an integer.\n",arr->lineno);
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
                        rttype->type = p->type;
                        rttype->tname = p->tname;
                    }else if (dimensions == p->arraydimension){
                        rt = 1;
                        rttype->type = p->type - 3;
                        rttype->tname = p->tname;
                    }
                }else{
                    //Error 10
                    printf("Error type 10 at Line %d: \"%s\" is not an array.\n",node->lineno,p->name);
                    rt = 1;
                }
            }
            p = p->next;
        }
        return rt;
    }
}

int checkFuncDefine(struct treeNode* node,struct RTtype *type){
    //node is Exp,sonlist is ID,sonlist->next is LP
    struct Func* p = funclist;
    if (strcmp(node->sonlist->next->next->name,"RP") == 0){
        //ID LP RP	
        while(p!=NULL){
            if (strcmp(p->name,node->sonlist->value) == 0){
                if (p->feild->next == NULL){
                    type->type = p->rt_type;
                    type->tname = p->rt_tname;
                    return 1;
                }
            }
            p = p->next;
        }
        
        struct Var* l = varlist;
        while(l!=NULL){
            if (strcmp(l->name,node->sonlist->value) == 0){
                //Error 11
                printf("Error type 11 at Line %d: \"%s\" is not a function.\n",node->lineno,l->name);
                return 1;
            }
            l = l->next;
        }
        
        return 0;
    }else{
        //ID LP Args RP
        struct treeNode *args = node->sonlist->next->next;
        while(p!=NULL){
            if (strcmp(p->name,node->sonlist->value) == 0){
                struct Var* q = p->feild->next;
                while(q != NULL && args != NULL){
                    struct RTtype argtype = dfsExp(args->sonlist);
                    if ((argtype.type == q->type && q->type != _STRUCT) || (argtype.type == q->type && q->type == _STRUCT && compare_struct(argtype.tname,q->tname) == 1 ) || (argtype.type == _NUMINT && q->type == _INT) || (argtype.type == _NUMFLOAT && q->type == _FLOAT)){
                       
                    }else{
                        printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",args->lineno,node->sonlist->value);
                        return 1;
                    }
                   
                    q = q->next;
                    if (args->sonlist->next != NULL)args = args->sonlist->next->next; else args=NULL;
                }
                if (q == NULL && args == NULL){
                    type->type = p->rt_type;
                    type->tname = p->rt_tname;
                    return 1;
                }else{
                    printf("Error type 9 at Line %d: Function \"%s\" is not applicable for arguments.\n",args->lineno,node->sonlist->value);
                    return 1;
                }
            }
            p = p->next;
        }
        
        struct Var* l = varlist;
        while(l!=NULL){
            if (strcmp(l->name,node->sonlist->value) == 0){
                //Error 11
                printf("Error type 11 at Line %d: \"%s\" is not a function.\n",node->lineno,l->name);
                return 1;
            }
            l = l->next;
        }
        
        //没有找到函数
        return 0;
    }
}
struct RTtype dfsExp(struct treeNode* node){
    //node is Exp
    //printf("lineno:%d\n",node->lineno);
    
    if (node->sonN == 1 && strcmp(node->sonlist->name,"ID") == 0){
        //Exp -> ID
        struct RTtype type;
        char name[30];
        if (checkDefine(node,name,&type,varlist)){
            return type;
        }else{
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->lineno,name);
        }
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"INT") == 0){
        //Exp -> INT
        struct RTtype type;
        type.type = _NUMINT;
        return type;
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"FLOAT") == 0){
        //Exp -> FLOAT
        struct RTtype type;
        type.type = _NUMFLOAT;
        return type;
    }else if (strcmp(node->sonlist->name,"ID")==0 && strcmp(node->sonlist->next->name,"LP") == 0){
        //Exp ->ID LP (Args) RP
        struct RTtype type;
        if (checkFuncDefine(node,&type)){
            return type;
        }else printf("Error type 2 at Line %d: Undefined function \"%s\".\n",node->lineno,node->sonlist->value);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"ASSIGNOP") == 0){
        //Exp ASSIGNOP Exp
        struct RTtype ltype = dfsExp(node->sonlist);
        //printf("%d - \n",ltype.type);
        if (ltype.type >= _INT && ltype.type <= _STRUCTARRAY){
            struct RTtype rtype = dfsExp(node->sonlist->next->next);
            int error = 0;
            if (ltype.type == _INT){
                if (rtype.type != _INT && rtype.type != _NUMINT) error = 1;
            }else if (ltype.type == _FLOAT){
                if (rtype.type != _FLOAT && rtype.type != _NUMFLOAT) error = 1;
            }else if (ltype.type == _STRUCT){
                if (rtype.type == _STRUCT && compare_struct(ltype.tname,rtype.tname)){
                    
                }else error = 1;
            }else if (ltype.type == _INTARRAY){
                if (rtype.type == _INTARRAY && ltype.dimension == rtype.dimension){
                    
                } else error = 1;
            }else if (ltype.type == _FLOATARRAY){
                if (rtype.type == _FLOATARRAY && ltype.dimension == rtype.dimension){
                    
                } else error = 1;
            }else if (ltype.type == _STRUCTARRAY){
                if (rtype.type == _STRUCTARRAY && compare_struct(ltype.tname,rtype.tname) && ltype.dimension == rtype.dimension){
                    
                }else error = 1;
            }
            if (error) printf("Error type 5 at Line %d: Type mismatched for assignment.\n",node->lineno);
        }else if (ltype.type > 0){
            printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",node->lineno);
        }
        /*
        if (strcmp(node->sonlist->sonlist->name,"ID") == 0 && node->sonlist->sonN == 1){
            //id
            struct treeNode* id = node->sonlist->sonlist;
            struct RTtype rtype = dfsExp(node->sonlist->next->next);
            if (checkDefine(node->sonlist,NULL,&ltype,varlist)){
                //check type
                if (ltype.type == rtype.type && ltype.type == _STRUCT){
                    if (compare_struct(ltype.tname,rtype.tname)){
                        return rtype;
                    }else{
                        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",id->lineno);
                    }
                }else{
                    if (rtype.type == ltype.type || rtype.type == ltype.type + 7){
                        return rtype;
                    }else{
                        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",id->lineno);
                    }
                }
            }else{
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",id->lineno,id->value);
            }
        }else if (strcmp(node->sonlist->sonlist->name,"Exp") == 0 && isAarray(node->sonlist)){
            //array
            char name[100];
            struct RTtype rtype = dfsExp(node->sonlist->next->next);
            if (checkDefine(node->sonlist,name,&ltype,varlist)){
                //check type
                if (ltype.type == rtype.type && ltype.type == _STRUCT){
                    if (compare_struct(ltype.tname,rtype.tname)){
                        return rtype;
                    }else{
                        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",node->lineno);
                    }
                }else{
                    if (rtype.type == ltype.type || rtype.type == ltype.type + 7){
                        return rtype;
                    }else{
                        printf("Error type 5 at Line %d: Type mismatched for assignment.\n",node->lineno);
                    }
                }
            }else{
                printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->sonlist->lineno,name);
            }
        }else{
            //error
            printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",node->lineno);
        }
        */
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"AND") == 0){
        //Exp AND Exp
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt; rt.type = _NUMINT;
        if ((ltype.type == _NUMINT || ltype.type == _INT)&&(rtype.type == _NUMINT || rtype.type == _INT)) return rt;
        //error
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"OR") == 0){
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt; rt.type = _NUMINT;
        if ((ltype.type == _NUMINT || ltype.type == _INT)&&(rtype.type == _NUMINT || rtype.type == _INT)) return rt;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"RELOP") == 0){
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if ((ltype.type == _INT || ltype.type == _NUMINT)&&(rtype.type == _INT || rtype.type == _NUMINT)) return rt1;
        if ((ltype.type == _FLOAT || ltype.type == _NUMFLOAT)&&(rtype.type == _FLOAT || rtype.type == _NUMFLOAT)) return rt2;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"PLUS") == 0){
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if ((ltype.type == _INT || ltype.type == _NUMINT)&&(rtype.type == _INT || rtype.type == _NUMINT)) return rt1;
        if ((ltype.type == _FLOAT || ltype.type == _NUMFLOAT)&&(rtype.type == _FLOAT || rtype.type == _NUMFLOAT)) return rt2;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"MINUS") == 0){
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if ((ltype.type == _INT || ltype.type == _NUMINT)&&(rtype.type == _INT || rtype.type == _NUMINT)) return rt1;
        if ((ltype.type == _FLOAT || ltype.type == _NUMFLOAT)&&(rtype.type == _FLOAT || rtype.type == _NUMFLOAT)) return rt2;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"STAR") == 0){
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if ((ltype.type == _INT || ltype.type == _NUMINT)&&(rtype.type == _INT || rtype.type == _NUMINT)) return rt1;
        if ((ltype.type == _FLOAT || ltype.type == _NUMFLOAT)&&(rtype.type == _FLOAT || rtype.type == _NUMFLOAT)) return rt2;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"DIV") == 0){
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rtype = dfsExp(node->sonlist->next->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if ((ltype.type == _INT || ltype.type == _NUMINT)&&(rtype.type == _INT || rtype.type == _NUMINT)) return rt1;
        if ((ltype.type == _FLOAT || ltype.type == _NUMFLOAT)&&(rtype.type == _FLOAT || rtype.type == _NUMFLOAT)) return rt2;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist != NULL && strcmp(node->sonlist->name,"MINUS") == 0){
        struct RTtype ltype = dfsExp(node->sonlist->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if (ltype.type == _INT || ltype.type == _NUMINT) return rt1;
        if (ltype.type == _FLOAT || ltype.type == _NUMFLOAT) return rt2;
        
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist != NULL && strcmp(node->sonlist->name,"NOT") == 0){
        struct RTtype ltype = dfsExp(node->sonlist->next);
        struct RTtype rt; rt.type = _NUMINT;
        if (ltype.type == _INT || ltype.type == _NUMINT) return rt;
        printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->lineno);
    }else if (node->sonlist !=NULL && strcmp(node->sonlist->name,"LP") == 0){
        //LP Exp RP  (Exp)
        struct RTtype ltype = dfsExp(node->sonlist->next);
        struct RTtype rt1; rt1.type = _NUMINT;
        struct RTtype rt2; rt2.type = _NUMFLOAT;
        if (ltype.type == _INT) return rt1;
        if (ltype.type == _FLOAT) return rt2;
        return ltype;
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"LB") == 0){
        //Exp LB Exp RB
        char name[30];
        struct RTtype type;
        type = dfsExp(node->sonlist);
        struct RTtype rtype;
        rtype = dfsExp(node->sonlist->next->next);
        if (rtype.type == _INT || rtype.type == _NUMINT){
            //printf("type.type is : %d, dimension:%d\n",type.type,type.dimension);
            if (type.type == _STRUCTARRAY || type.type == _INTARRAY || type.type == _FLOATARRAY){
                struct RTtype rt;
                if (type.dimension == 1) rt.type = type.type - 3; 
                else{
                    rt.type = type.type;
                    rt.dimension = type.dimension - 1;
                }
                return rt;
            }else{
                printf("Error type 10 at Line %d: left Exp is not an array.\n",node->lineno);
            }
        }else{
            printf("Error type 12 at Line %d: array subscript must be an integer.\n",node->lineno);
        }
        /*
        if (checkDefine(node,name,&type,varlist)){
            return type;
        }else{
            printf("Error type 1 at Line %d: Undefined variable \"%s\".\n",node->lineno,name);
        }
        */
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"DOT") == 0){
        //Exp DOT ID
        //Exp 可以是数组，函数返回值，ID，等
        struct treeNode* id = node->sonlist->next->next;
        struct RTtype ltype = dfsExp(node->sonlist);
        struct RTtype rt;
        //printf("%s-%d(3) DOT ID\n",ltype.tname,ltype.type);
        if (ltype.type == 3){
            struct Struct* l = structlist;
            while(l != NULL){
                if (strcmp(l->name,ltype.tname) == 0){
                    struct Var* ll = l->feild;
                    while(ll!=NULL){
                        if (strcmp(ll->name,id->value) == 0){
                            //printf("DDD: %d - %s\n",ll->type,ll->tname);
                            rt.type = ll->type;
                            rt.tname = ll->tname;
                            rt.dimension = ll->arraydimension;
                            return rt;
                        }
                        ll = ll->next;
                    }
                    printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",node->lineno,id->value);
                }
                l = l->next;
            }
            
        }else {
            printf("Error type 13 at Line %d: Illegal use of \".\".\n",node->lineno);
        }
    } 

    struct RTtype rtt; rtt.type = -1;
    return rtt;
}
int checkReturn(struct RTtype* rt,struct RTtype* rt_type,int lineno){
    if (rt_type->type != -1 && rt->type != -2){
        if ((rt->type != rt_type->type) &&((rt_type->type == _INT && rt->type == _NUMINT)||(rt_type->type == _FLOAT && rt->type == _NUMFLOAT))){
                
        }else if ((rt->type == rt_type->type) && (rt->type != _STRUCT)){
                
        }else if ((rt->type == rt_type->type) && (rt->type == _STRUCT) && compare_struct(rt->tname,rt_type->tname) == 1){
                
        }else{
            printf("Error type 8 at Line %d: Type mismatched for return.\n",lineno);
        }
    }
    return 0;
}

struct RTtype dfsStmt(struct treeNode* node,struct RTtype* rt_type){
    //node is Stmt 
    struct treeNode *stmt = node;
    struct RTtype rttype; rttype.type = -2;
    if (strcmp(stmt->sonlist->name,"Exp") == 0){
            //Stmt -> Exp SEMI
            dfsExp(stmt->sonlist);
        }else if (strcmp(stmt->sonlist->name,"CompSt") == 0){
            //Stmt -> CompSt
            dfsCompSt(stmt->sonlist,rt_type,0);
        }else if (strcmp(stmt->sonlist->name,"RETURN") == 0){
            //Stmt -> RETURN Exp SEMI
            rttype = dfsExp(stmt->sonlist->next);
            checkReturn(&rttype,rt_type,stmt->lineno);
        }else if (strcmp(stmt->sonlist->name,"IF") == 0){
            //Stmt -> IF LP Exp RP Stmt 
            //Stmt -> IF LP Exp RP Stmt ELSE Stmt
            dfsExp(stmt->sonlist->next->next);
            rttype = dfsStmt(stmt->sonlist->next->next->next->next,rt_type);
            //checkReturn(&rttype,rt_type,stmt->lineno);
            if (stmt->sonlist->next->next->next->next->next != NULL){
                rttype = dfsStmt(stmt->sonlist->next->next->next->next->next->next,rt_type);
                //checkReturn(&rttype,rt_type,stmt->lineno);
            }
        }else{
            //Stmt -> WHILE LP Exp RP Stmt
            dfsExp(stmt->sonlist->next->next);
            rttype = dfsStmt(stmt->sonlist->next->next->next->next,rt_type);
            //checkReturn(&rttype,rt_type,stmt->lineno);
        }
    return rttype;
}

int dfsStmtList(struct treeNode* node,struct RTtype* rt_type){
    //node is StmtList
    struct treeNode* stmt = node->sonlist;

    //处理Stmt中各类语句错误
    while(stmt != NULL){
        struct RTtype rt = dfsStmt(stmt,rt_type);
        /*
        if (rt_type->type != -1 && rt.type != -2){
            if ((rt.type != rt_type->type) &&((rt_type->type == _INT && rt.type == _NUMINT)||(rt_type->type == _FLOAT && rt.type == _NUMFLOAT))){
                
            }else if ((rt.type == rt_type->type) && (rt.type != _STRUCT)){
                
            }else if ((rt.type == rt_type->type) && (rt.type == _STRUCT) && compare_struct(rt.tname,rt_type->tname) == 1){
                
            }else{
                printf("Error type 8 at Line %d: Type mismatched for return.\n",stmt->lineno);
            }
        }
        */
        //next is stmtlist
        stmt = stmt->next->sonlist;
    }
    return 0;
}

int dfsCompSt(struct treeNode* node,struct RTtype* rt_type,int flag_func){
    //node is CompSt
    if (flag_func){
        
    }else{
        //insert a {
        struct Var* varlistp = (struct Var*)malloc(sizeof(struct Var));
        varlistp->type = 7;
        varlistp->tname = malloc(sizeof("{"));
        strncpy(varlistp->tname,"{",sizeof("{"));
        varlistp->name = malloc(sizeof("{"));
        strncpy(varlistp->name,"{",sizeof("{"));
        varlistp->defline = node->lineno;
        varlistp->father = varlistp->sonlist = varlistp->next = NULL;
        varlistp->next = varlist;
        varlist = varlistp;
    }
    dfsDefList(node->sonlist->next,varlist,0);
    dfsStmtList(node->sonlist->next->next,rt_type);

    //meet } , remove untill a {
    struct Var* p = varlist;
    struct Var* q = NULL;
    while(p != NULL){
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
    
    return 0;
}


//函数定义或声明，创建一个Func，将形参提取
struct Func* dfsFunDec(struct treeNode* node){
    //node is FunDec
    struct Func* func = (struct Func*)malloc(sizeof(struct Func));
    func->defline = node->lineno;
    //init a feild
    func->feild = (struct Var*)malloc(sizeof(struct Var));
    func->feild->next = NULL;
    struct Var* feildlistp = func->feild;
    feildlistp->type = 7;
    feildlistp->tname = malloc(sizeof("{"));
    strncpy(feildlistp->tname,"{",sizeof("{"));
    feildlistp->name = malloc(sizeof("{"));
    strncpy(feildlistp->name,"{",sizeof("{"));
        
    func->next = NULL;
    char *name = node->sonlist->value; //ID:main
    if (strcmp("RP",node->sonlist->next->next->name) == 0){
        //没有形参
    }else{
        //dfsVarList(node->sonlist->next->next);
        struct treeNode* varlist1 = node->sonlist->next->next;
        
        while(varlist1 != NULL){
            char *type = NULL;
            if (strcmp(varlist1->sonlist->sonlist->sonlist->name,"TYPE") == 0)
                type = varlist1->sonlist->sonlist->sonlist->value;
            else{ 
                //struct
                struct treeNode* structspecifier = varlist1->sonlist->sonlist->sonlist;
                if (strcmp(structspecifier->sonlist->next->name,"OptTag") == 0){
                    dfsStructDec(structspecifier,&type);
                }else{
                    type = structspecifier->sonlist->next->sonlist->value;
                }
            }
            
            //son is VarDec
            struct treeNode* son = varlist1->sonlist->sonlist->next;
            
            if (checkType(type) == 0){
                printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",son->lineno,type);
            }
            
            
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
            if (varlist1->sonlist->next == NULL) varlist1 = NULL;else varlist1 = varlist1->sonlist->next->next;
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
    //先新建一个结构体，不放入structlist，chekcrepeat后放入
    struct Struct* structlistp = (struct Struct*)malloc(sizeof(struct Struct));
    structlistp->type = type;
    structlistp->name = name;
    structlistp->defline = node->lineno;
    structlistp->feild = (struct Var*)malloc(sizeof(struct Var));
    structlistp->next = NULL;
    //feild初始化一个开始标记为{
    structlistp->feild->type = 7;
    structlistp->feild->tname = malloc(sizeof("{"));
    strncpy(structlistp->feild->tname,"{",sizeof("{"));
    structlistp->feild->name = malloc(sizeof("{"));
    strncpy(structlistp->feild->name,"{",sizeof("{"));
    structlistp->feild->father = structlistp->feild->sonlist = structlistp->feild->next = NULL;

    int rt = dfsDefList(node->sonlist->next->next->next,structlistp->feild,1); 
    if (rt == -1){
        return -1;
    }
    if (checkrepeat_struct(structlistp) == 0){
        structlistp->next = structlist;
        structlist = structlistp;
    }else{
        printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",node->lineno,name);
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
    varlist->defline = 0;
    varlist->name = malloc(sizeof("{"));
    strncpy(varlist->name,"{",sizeof("{"));
    varlist->father = varlist->sonlist = varlist->next = NULL;
     
    structlist = (struct Struct*)malloc(sizeof(struct Struct));
    structlist->defline = 0;
    structlist->name = malloc(sizeof("s_t_a_r_t"));
    strncpy(structlist->name,"s_t_a_r_t",sizeof("s_t_a_r_t"));
    structlist->feild = NULL;
    structlist->next = NULL;

    funclist = (struct Func*)malloc(sizeof(struct Func));
    funclist = funclist;
    funclist->defline = 0;
    funclist->name = malloc(sizeof("s_t_a_r_t"));
    strncpy(funclist->name,"s_t_a_r_t",sizeof("s_t_a_r_t"));
    funclist->type = 1;
    funclist->feild = NULL;
    funclist->next = NULL;
    
    //start
    dfsbuildtable(root);
    
    checkError18();
    return 0;
}
