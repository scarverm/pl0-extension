#include <stdio.h>
#include <string.h>
#include "pl0.h"
#pragma warning(disable:6031)
#pragma warning(disable:4996)

#define stacksize 500	//解释执行时使用的栈

int main() {
	bool nxtlev[symnum];

	printf("Input pl/0 file?    ");
	scanf("%s", fname);

	fin = fopen(fname, "r");	//以只读方式打开文件，文件不存在则打开失败，打开失败时返回空指针

	if (fin) {	//如果返回空指针则表示文件打开失败
		fa1 = fopen("fa1.tmp", "w");	//以只写方式打开文件，若文件存在则覆盖原内容，若不存在则创建新文件
		fprintf(fa1, "Input pl/0 file?    ");
		fprintf(fa1, "%s\n", fname);

		printf("List object code?(Y/N)");		//是否输出虚拟机代码
		scanf("%s", fname);
		listswitch = (fname[0] == 'y' || fname[0] == 'Y');

		printf("List symbol table?(Y/N)");	//是否输出名字表
		scanf("%s", fname);
		tableswitch = (fname[0] == 'y' || fname[0] == 'Y');

		init();		//初始化

		err = 0;
		cc = cx = ll = 0;
		ch = ' ';

		if (getsym() != -1) {	//getsym通过getchdo返回-1
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;

			if (block(0, 0, nxtlev, 0) == -1) {	//调用编译程序
				fclose(fa);
				fclose(fa1);
				fclose(fas);
				fclose(fin);
				printf("\n");
				return 0;
			}
			fclose(fa);
			fclose(fa1);
			fclose(fas);

			if (sym != period) {
				error(9);
			}

			if (err == 0) {
				fa2 = fopen("fa2.tmp", "w");
				interpret();	//调用解释执行程序
				fclose(fa2);
			}
			else {
				printf("Errors in pl/0 program");
			}
		}//end if (getsym() != -1)
		fclose(fin);
	}
	else {
		printf("Can't open file!\n");
	}//end if(fin)

 	printf("\n");
	return 0;
}

/*初始化*/
void init() {
	int i;	//仅作循环下标

	/*设置单字符符号*/
	/*先全部初始化为nul, 再选择需要的符号初始化为enum symbol, 保证能判定非法的单字符号*/
	for (i = 0; i <= 255; i++) {
		ssym[i] = nul;
	}
	ssym['+'] = plus;
	ssym['-'] = minus;
	ssym['*'] = times;
	ssym['/'] = slash;
	ssym['('] = lparen;
	ssym[')'] = rparen;
	ssym['='] = eql;
	ssym[','] = comma;
	ssym['.'] = period;
	ssym['#'] = neq;
	ssym[';'] = semicolon;
	ssym['$'] = comment;

	/*设置保留字名字, 按照字母顺序, 便于折半查找*/
	strcpy(&(word[0][0]), "begin");
	strcpy(&(word[1][0]), "call");
	strcpy(&(word[2][0]), "const");
	strcpy(&(word[3][0]), "do");
	strcpy(&(word[4][0]), "end");
	strcpy(&(word[5][0]), "if");
	strcpy(&(word[6][0]), "odd");
	strcpy(&(word[7][0]), "procedure");
	strcpy(&(word[8][0]), "read");
	strcpy(&(word[9][0]), "then");
	strcpy(&(word[10][0]), "var");
	strcpy(&(word[11][0]), "while");
	strcpy(&(word[12][0]), "write");

	/*设置保留字符号*/
	wsym[0] = beginsym;
	wsym[1] = callsym;
	wsym[2] = constsym;
	wsym[3] = dosym;
	wsym[4] = endsym;
	wsym[5] = ifsym;
	wsym[6] = oddsym;
	wsym[7] = procsym;
	wsym[8] = readsym;
	wsym[9] = thensym;
	wsym[10] = varsym;
	wsym[11] = whilesym;
	wsym[12] = writesym;

	/*设置指令名称*/
	strcpy(&(mnemonic[lit][0]), "lit");
	strcpy(&(mnemonic[opr][0]), "opr");
	strcpy(&(mnemonic[lod][0]), "lod");
	strcpy(&(mnemonic[vis][0]), "vis");
	strcpy(&(mnemonic[sto][0]), "sto");
	strcpy(&(mnemonic[ssto][0]), "ssto");
	strcpy(&(mnemonic[ssta][0]), "ssta");
	strcpy(&(mnemonic[chk][0]), "chk");
	strcpy(&(mnemonic[cal][0]), "cal");
	strcpy(&(mnemonic[inte][0]), "inte");
	strcpy(&(mnemonic[jmp][0]), "jmp");
	strcpy(&(mnemonic[jpc][0]), "jpc");

	/*设置符号集*/
	for (i = 0; i < symnum; i++) {
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/*设置声明开始符号集*/
	declbegsys[constsym] = true;	//声明常量
	declbegsys[varsym] = true;		//声明变量
	declbegsys[procsym] = true;		//声明函数

	/*设置语句开始符号集*/
	statbegsys[beginsym] = true;	//begin语句
	statbegsys[callsym] = true;		//函数调用语句
	statbegsys[ifsym] = true;		//if语句
	statbegsys[whilesym] = true;	//while语句

	/*设置因子开始符号集*/
	facbegsys[ident] = true;		//名字
	facbegsys[number] = true;		//数字
	facbegsys[lparen] = true;		//左括号
}

/*
* 过滤空格
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
* 被函数getsym调用
*/
int getch() {
	if (cc == ll) {	//cc==ll表示line中的字符获取完毕, 即读完一行内容
		if (feof(fin)) {	//EOF文件结束标识符，如果文件结束，返回非零值，如果文件结束，返回0
			printf("program incomplete");	//程序到了末尾应该不再调用getch，如果程序最后没加句号，就会报这个错误
			return -1;
		}
		ll = 0;
		cc = 0;
		ch = ' ';

		/*读完一行后，输出下一行的行号*/
		printf("%d ", cx);
		fprintf(fa1, "%d ", cx);

		while (ch != 10) {	//(char)10 == '\n'
			if (fscanf(fin, "%c", &ch) == EOF) {	//读到文件末尾时，为line加上空字符
				line[ll] = 0;
				break;
			}
			printf("%c", ch);
			fprintf(fa1, "%c", ch);
			line[ll] = ch;	//没有检查数组越界问题
			ll++;
		}
		line[ll] = 0;
		printf("\n");
		fprintf(fa1, "\n");
	}
	ch = line[cc];	//每次获取字符时都从line中获取
	cc++;
	return 0;
}

/*
* 词法分析，获取一个符号
*/
int getsym() {
	int i, j, k;

	while (ch == ' ' || ch == 10 || ch == 13 || ch == 9 || ch == '$') {	//忽略空格、换行、回车和TAB
		if (ch == '$') {
			cc = ll;
		}
		getchdo;
	}
	if (ch >= 'a' && ch <= 'z') {	//如果以字母开头，则表示这是一个名字或关键字/保留字
		k = 0;
		do {
			if (k < al) {	//与line相比做了越界检查,但是只取了前十个字符,事实上abcdefghijk与abcdefghijl都被认作abcdefghij
				a[k] = ch;
				k++;
			}
			getchdo;
		} while (ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9');
		a[k] = 0;	//补空字符
		strcpy(id, a);

		i = 0;
		j = norw - 1;
		do {	//折半查找当前符号是否为保留字
			k = (i + j) / 2;
			if (strcmp(id, word[k]) <= 0) {
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0) {
				i = k + 1;
			}
		} while (i <= j);	//如果搜索到结果，则j=k-1,i=k+1,i-1=j+1,即i-1>j
		if (i - 1 > j) {
			sym = wsym[k];
		}
		else {
			sym = ident;	//搜索失败，则表示这是名字或数字(以字母开头怎么会是数字)
		}
	}//end if (ch >= 'a' && ch <= 'z')
	else {
		if (ch >= '0' && ch <= '9') {	//以数字开头，则只能是数字，否则出错
			k = 0;
			num = 0;
			sym = number;	//设置符号为数字
			do {
				num = num * 10 + ch - '0';
				k++;
				getchdo;
			} while (ch >= '0' && ch <= '9');
			//k--;	//k就是num的位数，不需要这一行
			if (k > nmax) {
				error(30);
			}
		}//end if (ch >= '0' && ch <= '9')
		else {
			if (ch == ':') {	//检测是否为赋值符号
				getchdo;
				if (ch == '=') {
					sym = becomes;
					getchdo;
				}
				else {
					sym = colon;	//如果不是赋值符号，那就是冒号
				}
			}//end if (ch == ':')
			else {
				if (ch == '<') {
					getchdo;
					if (ch == '=') {
						sym = leq;
						getchdo;
					}
					else {
						sym = lss;
					}
				}
				else {
					if (ch == '>') {
						getchdo;
						if (ch == '=') {
							sym = geq;
							getchdo;
						}
						else {
							sym = gtr;
						}
					}
					else {
						sym = ssym[ch];	//不满足上述条件时，按单字符符号处理，如果是非法的单字符，则sym=ssym[ch]=nul
						if (sym != period) {	//period为句号，如果sym=period，则程序已经运行到末尾
							getchdo;			//如果程序最后没有加句号，则会报错“program incomplete”
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* 出错处理，打印出错位置和错误编码
* (其实就是输出了一堆空格，还不如直接用line代替space)
*/
void error(int n) {
	char space[81];
	memset(space, 32, 81);

	line[cc - 1] = 0;	//出错时当前符号已经读完，所以cc-1

	printf("****%s!%d\n", line, n);
	fprintf(fa1, "****%s!%d\n", line, n);

	err++;
}

int inset(int e, bool* s) {
	return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n) {
	int i;
	for (i = 0; i < n; i++) {
		sr[i] = s1[i] || s2[i];
	}
	return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		sr[i] = s1[i] && (!s2[i]);
	}
	return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		sr[i] = s1[i] && s2[i];
	}
	return 0;
}

/*
* 编译程序
* lev:		当前分程序所在层
* tx:		名字表当前尾指针
* fsys:		当前模块后跟符号集
* offset:	尾指针的偏移量
*/
int block(int lev, int tx, bool* fsys, int offset) {
	int i;

	int dx;					//名字分配到的相对地址
	int tx0;				//保留初始tx
	int cx0;				//保留初始cx
	bool nxtlev[symnum];	/* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空?
							传递给下级函数*/

	dx = 3;		//过程数据大小初始为3，每有一条声明，dx增加1
	tx0 = tx;	//记录本层名字的初始位置
	table[tx].adr = cx;
	tx = tx + offset;	//如果offset不为0，表示存在参数，而参数在上一层实现，需要使tx偏移以避免覆盖参数在名字表中占的位置
	dx = dx + offset;	//地址空间也要相应扩大

	gendo(jmp, 0, 0);	//每个过程都会生成这个指令，包括程序初始也会生成一个这个指令

	if (lev > levmax) {
		error(32);
	}

	do {	//直到没有声明符号
		if (sym == constsym) {	//收到常量声明符号
			getsymdo;
			constdeclarationdo(&tx, lev, &dx);	//dx的值会被constdeclaration改变，使用指针
			//实际上是constdeclaration调用enter，在enter中执行了(*ptx)++和(*pdx)++, 前者一定执行，后者可能执行
			while (sym == comma) {
				getsymdo;
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon) {
				getsymdo;
			}
			else {
				error(5);	//漏掉了逗号或分号, 如果漏掉了逗号, 则sym==ident
			}
		}//end if (sym == constsym)

		if (sym == varsym) {	//收到变量声明符号
			getsymdo;
			vardeclarationdo(&tx, lev, &dx);

			while (sym == comma) {
				getsymdo;
				vardeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon) {
				getsymdo;
			}
			else {
				error(5);	//vardeclaration中没有判断标识符后的=和:=，这里也没有，意味着变量不能在声明时赋值(不能初始化)
			}
		}//end if (sym == varsym)

		while (sym == procsym) {	//收到过程声明符号
			getsymdo;

			if (sym == ident) {
				enter(procedur, &tx, lev, &dx);	//记录过程名字
				getsymdo;
			}
			else {
				error(4);	//procedure后应为标识符
			}

			//带参数的过程
			int tx1 = tx;	//保留原始的tx不变
			int dx1 = 3;	//相对位置
			if (sym == lparen) {
				getsymdo;
				if (sym == varsym) {
					getsymdo;
					vardeclarationdo(&tx1, lev + 1, &dx1);
					table[tx].val++;	//用过程在名字表中的val值表示参数的个数
					while (sym == comma) {
						getsymdo;
						if (sym != varsym) {
							error(40);	//参数错误
						}
						getsymdo;
						vardeclarationdo(&tx1, lev + 1, &dx1);
						table[tx].val++;
					}
				}
				else {
					error(40);	//参数类型只能是var
				}
				if (sym == rparen) {
					getsymdo;
				}
				else {
					error(41);	//缺少右括号
				}
			}
			
			if (sym == semicolon) {	//过程名后要加分号
				getsymdo;
			}
			else {
				error(5);	//漏了分号
			}

			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
			nxtlev[semicolon] = true;

			/*可以当作 block(lev + 1, tx, nxtlev); */
			/*递归调用, 过程中包含过程, 因此过程块必须要用begin end, 否则第二个过程会被当作前一个过程的子过程*/
			if (block(lev + 1, tx, nxtlev, table[tx].val) == -1) {
				return -1;
			}

			if (sym == semicolon) {
				getsymdo;
				memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
				nxtlev[ident] = true;
				nxtlev[procsym] = true;
				testdo(nxtlev, fsys, 6);
			}
			else {
				error(5);	//漏掉了分号
			}
		}//end while (sym == procsym)
		memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));	//直到没有声明符号, 相当于 while(declbegsys[sym])

	code[table[tx0].adr].a = cx;	//开始生成当前过程代码
	table[tx0].adr = cx;			//当前过程代码地址
	table[tx0].size = dx;			//声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size
	cx0 = cx;
	gendo(inte, 0, dx);				//生成分配内存代码

	if (tableswitch) {		//输出名字表
		printf("TABLE:\n");
		if (tx0 + 1 > tx) {	//tx0+1>tx表示尾指针没有移动
			printf("	NULL\n");
		}
		for (i = tx0 + 1; i <= tx; i++) {
			switch (table[i].kind)
			{
			case constant:
				printf("	%d const %s ", i, table[i].name);
				printf("val=%d\n", table[i].val);
				fprintf(fas, "	%d const %s ", i, table[i].name);
				fprintf(fas, "val=%d\n", table[i].val);
				break;
			case variable:
				printf("	%d var %s ", i, table[i].name);
				printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
				fprintf(fas, "	%d var %s ", i, table[i].name);
				fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
				break;
			case procedur:
				printf("	%d proc %s ", i, table[i].name);
				printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				fprintf(fas, "	%d proc %s ", i, table[i].name);
				fprintf(fas, "lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
				break;
			case array:
				{
					int lowbound = table[i].lbound;
					int size = table[i].size;
					char name[11];
					strcpy(name, table[i].name);
					int ii;
					for (ii = lowbound; ii < lowbound + size; ii++, i++) {
						printf("	%d array %s[%d] ", i, name, ii);
						printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
						fprintf(fas, "	%d array %s[%d] ", i, name, ii);
						fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
					}
					i--;
					break;
				}
			}
		}
		printf("\n");
	}//end if (tableswitch)

	/*为所有作为参数的变量生成赋值代码*/
	if (offset) {
		int dx1 = 3;
		for (i = tx0; i < tx0 + offset; i++) {
			gendo(lit, 0, 0);	//不确定具体数值，先生成代码
			gendo(sto, 0, dx1);
			dx1++;
		}
	}

	/*语句后跟符号为分号或end*/
	memcpy(nxtlev, fsys, sizeof(bool)* symnum);	//每个后跟符号集合都包含上层后跟符号集合，以便补救
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);	//每个过程出口都要使用的释放数据段指令
	memset(nxtlev, 0, sizeof(bool)* symnum);	//分程序没有补救集合
	testdo(fsys, nxtlev, 8);	//检测后跟符号正确性
	listcode(cx0);	//输出代码
	return 0;
}

/*
* 生成虚拟机代码
* x: instruction.f	虚拟机代码指令
* y: instruction.l	引用层与声明层的层次差
* z: instruction.a	根据f的不同而不同
*/
int gen(enum fct x, int y, int z) {
	if (cx >= cxmax) {
		printf("Program too long");	//程序过长
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}

/*常量声明处理*/
int constdeclaration(int* ptx, int lev, int* pdx) {
	if (sym == ident) {
		getsymdo;
		if (sym == eql || sym == becomes) {	//eql → =; becomes → :=;
			if (sym == becomes) {
				error(1);	//把=写成了:=
			//本程序中，变量的声明使用:=，例如var a:=1; 而常量声明使用=, 例如const c=1
			}
			getsymdo;
			if (sym == number) {
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else {
				error(2);	//常量说明=后应是数字，这里导致常量无法被设置为负数，因为负号没有被识别为数的一部分
			}
		}//end if (sym == eql || sym == becomes)
		else {
			error(3);	//常量说明标识后应是=, 即常量用=而不用:=赋值，且常量应在声明时就赋值
		}
	}//end if (sym == ident)
	else {
		error(4);	//const后应是标识
	}
	return 0;
}

/*变量声明处理*/
int vardeclaration(int* ptx, int lev, int* pdx) {
	//比constdeclaration少了判断=和:=的一步，意味着变量不能在声明的时候赋值，与常量必须在声明时赋值相反
	if (sym == ident) {
		getsymdo;
		if (sym == lparen) {	//如果遇到左括号，则表示当前要声明的是一个数组
			int arraysize = 0;
			enter(array, ptx, lev, pdx);	//填写名字表
			getsymdo;	//获取数组下界
			if (sym == ident) {	//数组的下界只能是常量名或无符号整数
				int i = position(id, (*ptx));	//ptx？(*ptx)？
				if (table[i].kind == constant) {
					table[(*ptx)].lbound = table[i].val;
					arraysize = table[i].val;
					getsymdo;
				}
				else {
					error(42);	//数组下界格式错误，数组下界只能是常量或无符号整数
				}
			}
			else if (sym == number) {
				table[(*ptx)].lbound = num;
				arraysize = num;
				getsymdo;
			}
			else {
				error(42);	//数组下界格式错误
			}
			if (arraysize >= 0) {
				if (sym == colon) {
					getsymdo;
					if (sym == ident) {	//数组的上界只能是常量名或无符号整数
						int i = position(id, (*ptx));	//ptx？(*ptx)？
						if (table[i].kind == constant) {
							arraysize = table[i].val - arraysize;
							//得到的值实际上比数组长度小1，但已经在名字表中记录了第一个元素，所以不用再加一
							getsymdo;
						}
						else {
							error(42);	//数组下界格式错误，只能是常量或无符号整数
						}
					}
					else if (sym == number) {
						arraysize = num - arraysize;
						getsymdo;
					}
					else {
						error(42);	//数组上界格式错误
					}
					if (arraysize < 0) {
						error(43);	//数组上界的值小于下界的值
					}
					table[(*ptx)].size = arraysize + 1;
					while (arraysize--) {
						strcpy(id, "");	//为了避免使用position查找时无法找到数组的第一个元素，将id置空
						enter(array, ptx, lev, pdx);	//令每个元素都在名字表中占用一个位子
					}
					if (sym == rparen) {
						getsymdo;
					}
					else {
						error(41);	//缺少右括号
					}
				}
				else {
					error(42);	//数组上下界格式错误, 缺少分号
				}
			}
			else {
				error(42);	//数组下界格式错误
			}
		}//end if (sym == lparen)
		else {
			enter(variable, ptx, lev, pdx);	//填写名字表
		}
	}//end if (sym == ident) 
	else {
		error(4);	//var后应是标识符
	}
	return 0;
}

/*
* 在名字表中加入一项
* k:	名字种类.const var procedure
* ptx:	名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:	名字所在的层次
* pdx:	dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev, int* pdx) {
	(*ptx)++;	//先移动了指针，因此table[0]是空的
	strcpy(table[(*ptx)].name, id);
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:	//常量名字
		if (num > amax) {
			error(31);	//数越界, 准确的说是const类型的数越界，const类型最大值设置为2047
			num = 0;
		}
		table[(*ptx)].val = num;
		break;
	case variable:	//变量名字
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case procedur:	//过程名字
		table[(*ptx)].level = lev;
		break;
	case array:		//数组名字
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	}
}

/*
* 测试当前符号是否合法
* 在某一部分(如一条语句, 一个表达式)将要结束时我们希望下一个符号属于某集合
* (该部分的后跟符号), test负责这项检测, 并且负责当检测不通过时的补救措施,
* 程序在需要检测时指定当前需要的符号集合和补救用的集合(如之前未完成部分的后跟
* 符号), 以及检测不通过时的错误号
* s1:	我们需要的符号
* s2:	如果不是我们需要的, 则需要一个补救用的集合
* n:	错误号
*/
int test(bool* s1, bool* s2, int n) {
	if (!inset(sym, s1)) {
		error(n);
		/*当检测不通过时, 不停获取符号, 直到它属于需要的集合或补救的集合*/
		while ((!inset(sym, s1)) && (!inset(sym, s2))) {
			getsymdo;
		}
	}
	return 0;
}

/*语句处理*/
int statement(bool* fsys, int* ptx, int lev) {
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident) {		//准备按赋值语句处理
		i = position(id, *ptx);	//在名字表中查找id的位置
		if (i == 0) {
			error(11);	//变量未找到
		}
		else {
			if (table[i].kind == variable || table[i].kind == array) {
				if (table[i].kind == array) {
					getsymdo;
					if (sym == lparen) {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						expressiondo(nxtlev, ptx, lev);	//处理括号内表达式
						gendo(chk, table[i].lbound, table[i].size);	//检查下标是否越界
					}
					else {
						error(41);	//缺少右括号
					}
				}
				getsymdo;
				if (sym == becomes) {
					getsymdo;
				}
				else {
					error(13);	//没有检测到赋值符号
				}
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				expressiondo(nxtlev, ptx, lev);	//处理赋值符号右侧表达式
				//此时表达式的值在栈顶，而上一个expression计算出的值在次栈顶
				if (i != 0) {
					//expression将执行一系列命令，但最终结果将会保存在栈顶，执行sto命令完成赋值
					if (table[i].kind == variable) {
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					else {
						gendo(ssta, lev - table[i].level, table[i].adr - table[i].lbound);
					}
				}
			}
			else {
				error(12);	//赋值语句格式错误, 只有变量和数组能做赋值运算
				i = 0;
			}
		}//end else
	}//end if (sym == ident)
	else {
		if (sym == readsym) {	//准备按read语句处理
			getsymdo;
			if (sym != lparen) {
				error(34);	//错误，read后必须跟左括号
			}
			else {
				do {//while (sym == comma)
					getsymdo;
					if (sym == ident) {
						i = position(id, *ptx);	//查找要读的变量
					}
					else {
						i = 0;	//如果sym不是标识符，则视作未找到
					}

					if (i == 0) {
						error(35);	//read()中应是声明过的变量
					}
					else if (table[i].kind == variable) {
						gendo(opr, 0, 16);	//生成输入指令, 读取值到栈顶
						gendo(sto, lev - table[i].level, table[i].adr);	//存储到变量
					}
					else if (table[i].kind == array) {
						getsymdo;
						if (sym == lparen) {
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool) * symnum);
							expressiondo(nxtlev, ptx, lev);	//处理括号内表达式
							gendo(chk, table[i].lbound, table[i].size);	//检查下标是否越界
							gendo(opr, 0, 16);	//生成输入指令，读取值到栈顶
							gendo(ssta, lev - table[i].level, table[i].adr - table[i].lbound);	//将栈顶的值存入以次栈顶为下标的数组中
						}
						else {
							error(41);	//缺少右括号
						}
					}
					else {
						error(32);	//read()中应是变量或数组
					}
					getsymdo;
				} while (sym == comma);	//一条read语句可读多个变量
			}//end else
			if (sym != rparen) {
				error(33);	//未加右括号
				while (!inset(sym, fsys)) {	//出错补救，直到收到上层函数的后跟符号
					getsymdo;
				}
			}
			else {
				getsymdo;
			}
		}//end if (sym == readsym)
		else {
			if (sym == writesym) {	//准备按照write语句处理，与read类似
				getsymdo;
				if (sym == lparen) {
					do {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						nxtlev[rparen] = true;
						nxtlev[comma] = true;	//wirte的后跟符号为 ) 或 , 
						expressiondo(nxtlev, ptx, lev);	//调用表达式处理，此处与read不同
						gendo(opr, 0, 14);		//生成输出指令，输出栈顶的值
					} while (sym == comma);
					if (sym != rparen) {
						error(33);	//write()中应为完整表达式
					}
					else {
						getsymdo;
					}
				}//end if (sym == lparen)
				gendo(opr, 0, 15);	//输出换行
			}//end if (sym == writesym)
			else {
				if (sym == callsym) {	//准备按照call语句处理
					getsymdo;
					if (sym != ident) {
						error(14);		//call后应为标识符
					}
					else {
						i = position(id, *ptx);
						getsymdo;
						if (i == 0) {
							error(11);	//过程未找到
						}
						else {
							if (table[i].kind == procedur) {
								int para = 0;	//传入的实参个数
								if (sym == lparen) {
									do {
										getsymdo;
										para++;
										if (para <= table[i].val) {
											memcpy(nxtlev, fsys, sizeof(bool)* symnum);
											expressiondo(nxtlev, ptx, lev);	//处理实参中的表达式
											int address = table[i].adr + 1 + (para - 1) * 2;
											gendo(ssto, 0, address);	//进行回填
										}
										else {
											error(41);	//参数数量过多
										}
									} while (sym == comma);	//参数必须是标识符或数字
									if (sym == rparen) {
										getsymdo;
									}
									else {
										error(40);	//缺少右括号
									}
								}
								if (para < table[i].val) {
									error(41);	//参数数量过少
								}
								gendo(cal, lev - table[i].level, table[i].adr);	//生成call指令
							}
							else {
								error(15);	//call后标识符应为过程
							}
						}
					}//end else
				}//end if (sym == callsym)
				else {
					if (sym == ifsym) {	//准备按照if语句处理
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)* symnum);
						nxtlev[thensym] = true;
						nxtlev[dosym] = true;		//后跟符号为then或do, do 不能代替 then, 应该有其他用法
						conditiondo(nxtlev, ptx, lev);	//调用条件处理（逻辑运算）函数
						if (sym == thensym) {
							getsymdo;
						}
						else {
							error(16);	//缺少then
						}
						cx1 = cx;	//保存当前指令地址
						gendo(jpc, 0, 0);	//生成条件跳转指令，跳转地址未知，暂时写0
						statementdo(fsys, ptx, lev);	//处理then后的语句
						code[cx1].a = cx;	//经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址
						//地址回填
					}//end if (sym == ifsym)
					else {
						if (sym == beginsym) {	//准备按照复合语句处理
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool)* symnum);
							nxtlev[semicolon] = true;
							nxtlev[endsym] = true;

							/*循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end*/
							statementdo(nxtlev, ptx, lev);
							while (inset(sym, statbegsys) || sym == semicolon) {
								if (sym == semicolon) {
									getsymdo;
								}
								else {
									error(10);	//缺少分号
								}
								statementdo(nxtlev, ptx, lev);
							}
							if (sym == endsym) {
								getsymdo;
							}
							else {
								error(17);		//缺少end或分号
							}
						}//end if (sym == beginsym)
						else {
							if (sym == whilesym) {	//准备按照while语句处理
								cx1 = cx;	//保存判断条件操作的位置
								getsymdo;
								memcpy(nxtlev, fsys, sizeof(bool)* symnum);
								nxtlev[dosym] = true;	//后跟符号为do
								conditiondo(nxtlev, ptx, lev);	//调用条件处理
								cx2 = cx;	//保存循环体结束的下一个位置
								gendo(jpc, 0, 0);	//生成条件跳转，但跳出循环的地址未知
								if (sym == dosym) {
									getsymdo;
								}
								else {
									error(18);	//缺少do
								}
								statementdo(fsys, ptx, lev);	//循环体
								gendo(jmp, 0, cx1);	//回头重新判断条件
								code[cx2].a = cx;	//回填跳出循环的地址，与if类似
							}//end if (sym == whilesym)
							else {
								memset(nxtlev, 0, sizeof(bool)* symnum);	//语句结束无补救集合
								testdo(fsys, nxtlev, 19);	//检测语句结束的正确性
							}
						}//end else
					}//end else
				}//end else
			}//end else
		}//end else
	}//end else
	return 0;
}

/*
* 查找名字的位置
* 找到则返回在名字表中的位置，否则返回0
* idt:	要查找的名字
* tx:	当前名字表的尾指针
*/
int position(char* idt, int tx) {
	int i;
	strcpy(table[0].name, idt);	//将idt放在第0位，如果返回0，表示没找到
	i = tx;
	while (strcmp(table[i].name, idt) != 0) {
		i--;
	}
	return i;
}

/*
* 表达式处理
* 调用expression时，expression会先调用term，而term又会先调用factor
* 因此程序会先处理factor中的因子，即括号内的表达式
* 然后再处理term中的乘除法
* 最后再处理expression中的加减法
* 进而实现计算的优先级
*/
int expression(bool* fsys, int* ptx, int lev) {
	enum symbol addop;	//用于保存正负号
	bool nxtlev[symnum];

	if (sym == plus || sym == minus) {	//开头的正负号，此时表达式被看作一个正的或负的项
		addop = sym;	//保存开头的正负号
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);	//处理项, 在计算加减法之前先计算乘除
		if (addop == minus) {
			gendo(opr, 0, 1);	//如果开头为负号生成取负指令
		}
	}
	else {	//此时表达式被看作项的加减
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);	//处理项
	}
	while (sym == plus || sym == minus) {
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);	//处理项
		if (addop == plus) {
			gendo(opr, 0, 2);	//生成加法指令
		}
		else {
			gendo(opr, 0, 3);	//生成减法指令
		}
	}
	return 0;
}

/*条件处理*/
int condition(bool* fsys, int* ptx, int lev) {
	enum symbol relop;	//保存当前关系运算符
	bool nxtlev[symnum];

	if (sym == oddsym) {	//准备按照odd运算处理
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);	//odd运算
	}
	else {
		/*逻辑表达式处理*/
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq) {
			error(20);	//不是关系运算
		}
		else {
			relop = sym;
			getsymdo;
			expressiondo(fsys, ptx, lev);
			switch (relop)
			{
			case eql:
				gendo(opr, 0, 8);	//eql
				break;
			case neq:
				gendo(opr, 0, 9);	//neq
				break;
			case lss:
				gendo(opr, 0, 10);	//lss
				break;
			case geq:
				gendo(opr, 0, 11);	//geq
				break;
			case gtr:
				gendo(opr, 0, 12);	//gtr
				break;
			case leq:
				gendo(opr, 0, 13);	//leq
				break;
			}
		}//end else
	}
	return 0;
}

/*项处理*/
int term(bool* fsys, int* ptx, int lev) {
	enum symbol mulop;	//用于保存乘除法符号
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factordo(nxtlev, ptx, lev);	//处理因子, 再计算乘除之前先计算括号内表达式
	while (sym == times || sym == slash) {
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if (mulop == times) {
			gendo(opr, 0, 4);	//生成乘法指令
		}
		else {
			gendo(opr, 0, 5);	//生成除法指令
		}
	}
	return 0;
}

/*因子处理*/
int factor(bool* fsys, int* ptx, int lev) {
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);	//检测因子的开始符号
	if (inset(sym, facbegsys)) {	/* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
		if (sym == ident) {		//因子为常量、变量或数组
			i = position(id, *ptx);	//查找名字
			if (i == 0) {
				error(11);	//标识符未声明
			}
			else {
				switch (table[i].kind)
				{
				case constant:	//标识符为常量
					gendo(lit, 0, table[i].val);	//直接把常量的值入栈
					break;
				case variable:	//标识符为变量
					gendo(lod, lev - table[i].level, table[i].adr);	//找到变量地址并将其值入栈
					break;
				case procedur:	//标识符为过程
					error(21);	//不能为过程
					break;
				case array:		//标识符为数组
					getsymdo;
					if (sym == lparen) {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						expressiondo(nxtlev, ptx, lev);
						gendo(chk, table[i].lbound, table[i].size);	//检查下标是否越界
						gendo(vis, lev - table[i].level, table[i].adr - table[i].lbound);
						if (sym != rparen) {
							error(41);	//缺少右括号
						}
					}
					else {
						error(42);	//数组下标错误
					}
				}
			}
			getsymdo;
		}//end if (sym == ident)
		else {
			if (sym == number) {	//因子为数
				if (num > amax) {
					error(31);		//常数作为因子时不能大于amax，但计算得到的值可以大于amax
					num = 0;
				}
				gendo(lit, 0, num);
				getsymdo;
			}
			else {
				if (sym == lparen) {	//因子为表达式
					getsymdo;
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;
					expressiondo(nxtlev, ptx, lev);
					if (sym == rparen) {
						getsymdo;
					}
					else {
						error(22);	//缺少右括号
					}
				}
				testdo(fsys, facbegsys, 23);	//因子后有非法符号
			}//end else
		}//end else
	}//end if (inset(sym, facbegsys))
	return 0;
}

/*输出目标代码清单*/
void listcode(int cx0) {
	int i;
	if (listswitch) {
		for (i = cx0; i < cx; i++) {
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fa, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*解释程序*/
void interpret() {
	int p, b, t;	//指令指针，指令基址，栈顶指针
	struct instruction i;	//存放当前指令
	int s[stacksize];	//栈

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {//while (p != 0)
		i = code[p];	//读当前指令
		p++;
		switch (i.f)
		{
		case lit:	//将a的值取到栈顶
			s[t] = i.a;
			t++;
			break;
		case opr:	//数学、逻辑运算
			switch (i.a)
			{
			case 0:
				t = b;
				p = s[t + 2];
				b = s[t + 1];
				break;
			case 1:		//未实现
				s[t - 1] = -s[t - 1];
				break;
			case 2:		//加法指令
				t--;
				s[t - 1] = s[t - 1] + s[t];
				break;
			case 3:		//减法指令
				t--;
				s[t - 1] = s[t - 1] - s[t];
				break;
			case 4:		//乘法指令
				t--;
				s[t - 1] = s[t - 1] * s[t];
				break;
			case 5:		//除法指令
				t--;
				s[t - 1] = s[t - 1] / s[t];
				break;
			case 6:		//odd指令
				s[t - 1] = s[t - 1] % 2;
				break;
			case 8:		//等于
				t--;
				s[t - 1] = (s[t - 1] == s[t]);
				break;
			case 9:		//不等
				t--;
				s[t - 1] = (s[t - 1] != s[t]);
				break;
			case 10:	//小于
				t--;
				s[t - 1] = (s[t - 1] < s[t]);
				break;
			case 11:	//大于等于
				t--;
				s[t - 1] = (s[t - 1] >= s[t]);
				break;
			case 12:	//大于
				t--;
				s[t - 1] = (s[t - 1] > s[t]);
				break;
			case 13:	//小于等于
				t--;
				s[t - 1] = (s[t - 1] <= s[t]);
				break;
			case 14:	//输出栈顶的值, 用于输出结果
				printf("%d", s[t - 1]);
				fprintf(fa2, "%d", s[t - 1]);
				t--;
				break;
			case 15:	//输出换行
				printf("\n");
				fprintf(fa2, "\n");
				break;
			case 16:	//读取值到栈顶
				printf("?");
				fprintf(fa2, "?");
				scanf("%d", &(s[t]));
				fprintf(fa2, "%d\n", s[t]);
				t++;
				break;
			}//end switch (i.a)
			break;
		case lod:	//取相对当前过程的数据基地址为a的内存的值到栈顶
			s[t] = s[base(i.l, s, b) + i.a];
			t++;
			break;
		case vis:	//将相对当前过程的数据基地址为a+s[t-1]的内存的值更换栈顶值，原栈顶值是计算得到的下标
			s[t - 1] = s[base(i.l, s, b) + i.a + s[t - 1]];	//i.a的值为数组名所在名字表中的adr-lbound
			break;
		case sto:	//栈顶的值存到相对当前过程的数据基地址为a的内存
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case ssto:	//将栈顶的值存入lit指令
			code[i.a].a = s[t - 1];
			break;
		case ssta:	//将栈顶的值存入以次栈顶为下标的数组中
			t--;
			s[base(i.l, s, b) + i.a + s[t - 1]] = s[t];
			break;
		case chk:	//检查栈顶的值是否在数组的下标范围中
			if (s[t - 1] < i.l) {
				error(43);	//数组下标越下界
			}
			if (s[t - 1] > i.l + i.a - 1) {
				error(43);	//数组下标越上界
			}
			break;
		case cal:	//调用子过程
			s[t] = base(i.l, s, b);	//将父过程基地址入栈
			s[t + 1] = b;	//将本过程基地址入栈，此两项用于base函数
			s[t + 2] = p;	//将当前指令指针入栈
			b = t;			//改变基地址指针值为新过程的基地址
			p = i.a;		//跳转
			break;
		case inte:	//分配内存
			t += i.a;
			break;
		case jmp:	//直接跳转
			p = i.a;
			break;
		case jpc:	//条件跳转
			t--;
			if (s[t] == 0) {	//不满足条件则跳转
				p = i.a;
			}
			break;
		}//end switch (i.f)
	} while (p != 0);
}

/*通过过程基址求上1层过程的基址*/
int base(int l, int* s, int b) {
	int b1;
	b1 = b;
	while (l > 0) {
		b1 = s[b1];
		l--;
	}
	return b1;
}