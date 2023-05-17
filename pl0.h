#pragma once

typedef enum {
	false,
	true
} bool;

#define norw 16		//�ؼ��ָ���
#define al 10		//������󳤶�	�����ļ���
#define nmax 14		//number�����λ��
#define txmax 100	//���ֱ�����
#define cxmax 500	//��������������
#define levmax 3	//����������Ƕ���������
#define amax 2047	//��ַ�Ͻ�

/* ���� */
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

/*���������*/
enum fct {
	lit,	opr,	lod,
	sto,	cal,	inte,
	jmp,	jpc,	ssto,
	vis,	ssta,	chk
};
#define fctnum 12

/*���������ṹ*/
struct instruction {
	enum fct f;	//���������ָ��
	int l;		//���ò���������Ĳ�β�
	int a;		//����f�Ĳ�ͬ����ͬ
};
struct instruction code[cxmax];	//�����������������

/*���ֱ��е�����*/
enum object {
	constant,	//����
	variable,	//����
	procedur,	//����
	array		//add
};

/*���ֱ�ṹ*/
struct tablestruct{
	char name[al];		//����
	enum object kind;	//���ͣ�const, var, array or procedure
	int val;			//��ֵ����constʹ�ã�procedureʹ��ʱ��ʾ����������
	int level;			//�����㣬��const��ʹ��
	int adr;			//��ַ����const��ʹ��
	int size;			//��procedure��arrayʹ�ã��ֱ��ʾ��Ҫ������������ռ�������С
	int lbound;			//�½磬��arrayʹ��
};
struct tablestruct table[txmax];	//���ֱ�

FILE* fin;
char fname[al];	//�ļ���

FILE* fas;	//������ֱ�
FILE* fa;	//������������
FILE* fa1;	//���Դ�ļ�������ж�Ӧ���׵�ַ
FILE* fa2;	//������
int err;	//���������

int cc, ll;	//getchʹ�õļ�������cc��ʾ��ǰ�ַ���λ�ã�ll��ʾline����Ч�ַ��ĳ���
int cx;		//���������ָ�룬ȡֵ��Χ[0, cxmax-1]
char ch;	//��ȡ�ַ��Ļ�����
char line[81];	//�л�����, ÿ�����80���ַ�, ���һ���ַ���ſ��ַ�
char a[al + 1];	//��ʱ���ţ����һ���ַ���ſ��ַ�
char id[al + 1];//��ǰident�����һ���ַ���ſ��ַ�

bool listswitch;	//�Ƿ���ʾ���������
bool tableswitch;	//�Ƿ���ʾ���ֱ�

char word[norw][al];		//�����֣��ؼ���
char mnemonic[fctnum][5];	//���������ָ������
enum symbol wsym[norw];		//�����ֶ�Ӧ�ķ���ֵ
enum symbol ssym[256];		//���ַ��ķ���ֵ��256��ӦASCII��
enum symbol sym;			//��ǰ�ķ���
int num;					//��ǰ����

bool declbegsys[symnum];	//��ʾ������ʼ�ķ��ż���
bool statbegsys[symnum];	//��ʾ��俪ʼ�ķ��ż���
bool facbegsys[symnum];		//��ʾ���ӿ�ʼ�ķ��ż���

/*�������лᷢ��fatal errorʱ������-1��֪�������ĺ����������˳�����*/
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