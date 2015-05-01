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
int converttype(char* type){
    if (strcmp(type,"int") == 0) return 1;
    if (strcmp(type,"float") == 0) return 2;
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
	struct Var* p = list;
	while(p != NULL){
		if (strcmp(p->name,name) == 0) return 1;
		p = p->next;
	}
    return 0;
}

int checkrepeat_func(char* name){
	struct Func* p = funclist;
	while(p != NULL){
		if (strcmp(p->name,name) == 0 && p->type == 1) return 1;
		p = p->next;
	}
	return 0;
}
//----------------------------------------------------------------
//forth level

int dfsDecList(struct treeNode* node,char* type,struct Var* rtlist){
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

int dfsCompSt(struct treeNode* node){
	
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
        printf("type is %s\n",type);
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
			funclistp->next = func;
			funclistp = funclistp->next;
			
        }else if (strcmp(son->next->name,"CompSt") == 0){
			if (checkrepeat_func(son->sonlist->value) == 0){
				func->type = 1;
				func->name = son->sonlist->value;
				func->rt_type = converttype(type);
				func->rt_tname = type;
				funclistp->next = func;
				funclistp = funclistp->next;
				//将其形参加入varlist，然后进行compt的check
				struct Var* p = func->feild;
				if (p != NULL)
					while(p->next != NULL) p = p->next;
				varlistp->next = func->feild;
				varlistp = p;
				
				//dfs CompSt
				dfsCompSt(son->next);
				
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
    
    return 0;
}
