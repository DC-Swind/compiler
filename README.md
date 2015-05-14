# Compiler <br>
完成的功能：<br>
    首先我实现了基本要求中的Error1-17的检测，除此之外，我实现了选做部分的要求2.1-2.3. 实现的程序可以检测出C--测试代码中所有语义错误。满足实验的要求，Error的输出格式与要求相符，为<br>
    Error type 错误类型 at Line 行号： 说明文字 <br>
    但是其中的说明文字与实验讲义中样例略有不同，但保证错误类型完全一致。<br>

编译步骤：<br>
    Makefile中定义了一些伪对象 <br>
    编译  make <br>
    清理历史编译  make clean <br>
    运行  make play <br>

实现方法：<br>
    在Lab1中已经建立了语法树，在Lab2中，沿用了建好的语法树，从根结点开始DFS，我为每一个非终结符号创建了一个DFS函数，就可以将程序很好的封装、分割。
    其中符号表我使用了变量符号表（struct Var）、函数符号表（struct Struct）、结构体符号表（struct Func）。三个表的组织方式都采取了栈结构，主要考虑到变量的作用域问题。在遇到{时，将{压栈（放入链表头部），在遇到}时，弹栈直到遇到一个{。
    其中结构体（struct Struct）中有一个（struct Var）链表用来记录结构体的属性。函数（struct Func）中有一个（struct Var）链表用来记录函数的形参列表。

结点的数据结构表示：<br>
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

