#pragma once

typedef enum {
	false,
	true
} bool;

#define norw 16		//关键字个数
#define al 10		//符号最大长度	用于文件名
#define nmax 14		//number的最大位数
#define txmax 100	//名字表容量
#define cxmax 500	//最多虚拟机代码数
#define levmax 3	//最大允许过程嵌套声明层次
#define amax 2047	//地址上界

/* 符号 */
enum symbol {
	nul,		ident,		number,		plus,		minus,
	times,		slash,		oddsym,		eql,		neq,
	lss,		leq,		gtr,		geq,		lparen,
	rparen,		comma,		semicolon,	period,		becomes,
	beginsym,	endsym,		ifsym,		thensym,	whilesym,
	writesym,	readsym,	dosym,		callsym,	constsym,
	varsym,		procsym,	comment,	colon,		repeatsym,
	untilsym,	elsesym
};
#define symnum 37

/*虚拟机代码*/
enum fct {
	lit,	opr,	lod,
	sto,	cal,	inte,
	jmp,	jpc,	ssto,
	vis,	ssta,	chk
};
#define fctnum 12

/*虚拟机代码结构*/
struct instruction {
	enum fct f;	//虚拟机代码指令
	int l;		//引用层与声明层的层次差
	int a;		//根据f的不同而不同
};
struct instruction code[cxmax];	//存放虚拟机代码的数组

/*名字表中的类型*/
enum object {
	constant,	//常量
	variable,	//变量
	procedur,	//函数
	array		//add
};

/*名字表结构*/
struct tablestruct{
	char name[al];		//名字
	enum object kind;	//类型：const, var, array or procedure
	int val;			//数值，仅const使用（procedure使用时表示参数数量）
	int level;			//所处层，仅const不使用
	int adr;			//地址，仅const不使用
	int size;			//仅procedure和array使用，分别表示需要分配的数据区空间和数组大小
	int lbound;			//下界，仅array使用
};
struct tablestruct table[txmax];	//名字表

FILE* fin;
char fname[al];	//文件名

FILE* fas;	//输出名字表
FILE* fa;	//输出虚拟机代码
FILE* fa1;	//输出源文件及其各行对应的首地址
FILE* fa2;	//输出结果
int err;	//错误计数器

int cc, ll;	//getch使用的计数器，cc表示当前字符的位置，ll表示line中有效字符的长度
int cx;		//虚拟机代码指针，取值范围[0, cxmax-1]
char ch;	//获取字符的缓冲区
char line[81];	//行缓冲区, 每行最多80个字符, 多出一个字符存放空字符
char a[al + 1];	//临时符号，多出一个字符存放空字符
char id[al + 1];//当前ident，多出一个字符存放空字符

bool listswitch;	//是否显示虚拟机代码
bool tableswitch;	//是否显示名字表

char word[norw][al];		//保留字，关键字
char mnemonic[fctnum][5];	//虚拟机代码指令名称
enum symbol wsym[norw];		//保留字对应的符号值
enum symbol ssym[256];		//单字符的符号值，256对应ASCII码
enum symbol sym;			//当前的符号
int num;					//当前数字

bool declbegsys[symnum];	//表示声明开始的符号集合
bool statbegsys[symnum];	//表示语句开始的符号集合
bool facbegsys[symnum];		//表示因子开始的符号集合

/*当函数中会发生fatal error时，返回-1告知调用它的函数，最终退出程序*/
#define getchdo						if (-1 == getch()) return -1
#define gendo(a, b, c)				if (-1 == gen(a, b, c)) return -1
#define getsymdo					if (-1 == getsym()) return -1
#define constdeclarationdo(a, b, c)	if (-1 == constdeclaration(a, b, c)) return -1
#define vardeclarationdo(a, b, c)	if (-1 == vardeclaration(a, b, c)) return -1
#define testdo(a, b, c)				if (-1 == test(a, b, c)) return -1
#define expressiondo(a, b, c)		if (-1 == expression(a, b, c)) return -1
#define conditiondo(a, b, c)		if (-1 == condition(a, b, c)) return -1
#define statementdo(a, b, c)		if (-1 == statement(a, b, c)) return -1
#define termdo(a, b, c)				if (-1 == term(a, b, c)) return -1
#define factordo(a, b, c)			if (-1 == factor(a, b, c)) return -1

void init();
int getch();
int getsym();
void error(int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
int gen(enum fct x, int y, int z);
int block(int lev, int tx, bool* fsys, int offset);
int constdeclaration(int* ptx, int lev, int* pdx);
int vardeclaration(int* ptx, int lev, int* pdx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int test(bool* s1, bool* s2, int n);
int statement(bool* fsys, int* ptx, int lev);
int position(char* idt, int tx);
int expression(bool* fsys, int* ptx, int lev);
int condition(bool* fsys, int* ptx, int lev);
int term(bool* fsys, int* ptx, int lev);
int factor(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
void interpret();
int base(int l, int* s, int b);