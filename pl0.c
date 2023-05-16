#include <stdio.h>
#include <string.h>
#include "pl0.h"
#pragma warning(disable:6031)
#pragma warning(disable:4996)

#define stacksize 500	//����ִ��ʱʹ�õ�ջ

int main() {
	bool nxtlev[symnum];

	printf("Input pl/0 file?    ");
	scanf("%s", fname);

	fin = fopen(fname, "r");	//��ֻ����ʽ���ļ����ļ����������ʧ�ܣ���ʧ��ʱ���ؿ�ָ��

	if (fin) {	//������ؿ�ָ�����ʾ�ļ���ʧ��
		fa1 = fopen("fa1.tmp", "w");	//��ֻд��ʽ���ļ������ļ������򸲸�ԭ���ݣ����������򴴽����ļ�
		fprintf(fa1, "Input pl/0 file?    ");
		fprintf(fa1, "%s\n", fname);

		printf("List object code?(Y/N)");		//�Ƿ�������������
		scanf("%s", fname);
		listswitch = (fname[0] == 'y' || fname[0] == 'Y');

		printf("List symbol table?(Y/N)");	//�Ƿ�������ֱ�
		scanf("%s", fname);
		tableswitch = (fname[0] == 'y' || fname[0] == 'Y');

		init();		//��ʼ��

		err = 0;
		cc = cx = ll = 0;
		ch = ' ';

		if (getsym() != -1) {	//getsymͨ��getchdo����-1
			fa = fopen("fa.tmp", "w");
			fas = fopen("fas.tmp", "w");
			addset(nxtlev, declbegsys, statbegsys, symnum);
			nxtlev[period] = true;

			if (block(0, 0, nxtlev, 0) == -1) {	//���ñ������
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
				interpret();	//���ý���ִ�г���
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

/*��ʼ��*/
void init() {
	int i;	//����ѭ���±�

	/*���õ��ַ�����*/
	/*��ȫ����ʼ��Ϊnul, ��ѡ����Ҫ�ķ��ų�ʼ��Ϊenum symbol, ��֤���ж��Ƿ��ĵ��ַ���*/
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

	/*���ñ���������, ������ĸ˳��, �����۰����*/
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

	/*���ñ����ַ���*/
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

	/*����ָ������*/
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

	/*���÷��ż�*/
	for (i = 0; i < symnum; i++) {
		declbegsys[i] = false;
		statbegsys[i] = false;
		facbegsys[i] = false;
	}

	/*����������ʼ���ż�*/
	declbegsys[constsym] = true;	//��������
	declbegsys[varsym] = true;		//��������
	declbegsys[procsym] = true;		//��������

	/*������俪ʼ���ż�*/
	statbegsys[beginsym] = true;	//begin���
	statbegsys[callsym] = true;		//�����������
	statbegsys[ifsym] = true;		//if���
	statbegsys[whilesym] = true;	//while���

	/*�������ӿ�ʼ���ż�*/
	facbegsys[ident] = true;		//����
	facbegsys[number] = true;		//����
	facbegsys[lparen] = true;		//������
}

/*
* ���˿ո�
* ÿ�ζ�һ�У�����line��������line��getsymȡ�պ��ٶ�һ��
* ������getsym����
*/
int getch() {
	if (cc == ll) {	//cc==ll��ʾline�е��ַ���ȡ���, ������һ������
		if (feof(fin)) {	//EOF�ļ�������ʶ��������ļ����������ط���ֵ������ļ�����������0
			printf("program incomplete");	//������ĩβӦ�ò��ٵ���getch������������û�Ӿ�ţ��ͻᱨ�������
			return -1;
		}
		ll = 0;
		cc = 0;
		ch = ' ';

		/*����һ�к������һ�е��к�*/
		printf("%d ", cx);
		fprintf(fa1, "%d ", cx);

		while (ch != 10) {	//(char)10 == '\n'
			if (fscanf(fin, "%c", &ch) == EOF) {	//�����ļ�ĩβʱ��Ϊline���Ͽ��ַ�
				line[ll] = 0;
				break;
			}
			printf("%c", ch);
			fprintf(fa1, "%c", ch);
			line[ll] = ch;	//û�м������Խ������
			ll++;
		}
		line[ll] = 0;
		printf("\n");
		fprintf(fa1, "\n");
	}
	ch = line[cc];	//ÿ�λ�ȡ�ַ�ʱ����line�л�ȡ
	cc++;
	return 0;
}

/*
* �ʷ���������ȡһ������
*/
int getsym() {
	int i, j, k;

	while (ch == ' ' || ch == 10 || ch == 13 || ch == 9 || ch == '$') {	//���Կո񡢻��С��س���TAB
		if (ch == '$') {
			cc = ll;
		}
		getchdo;
	}
	if (ch >= 'a' && ch <= 'z') {	//�������ĸ��ͷ�����ʾ����һ�����ֻ�ؼ���/������
		k = 0;
		do {
			if (k < al) {	//��line�������Խ����,����ֻȡ��ǰʮ���ַ�,��ʵ��abcdefghijk��abcdefghijl��������abcdefghij
				a[k] = ch;
				k++;
			}
			getchdo;
		} while (ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9');
		a[k] = 0;	//�����ַ�
		strcpy(id, a);

		i = 0;
		j = norw - 1;
		do {	//�۰���ҵ�ǰ�����Ƿ�Ϊ������
			k = (i + j) / 2;
			if (strcmp(id, word[k]) <= 0) {
				j = k - 1;
			}
			if (strcmp(id, word[k]) >= 0) {
				i = k + 1;
			}
		} while (i <= j);	//����������������j=k-1,i=k+1,i-1=j+1,��i-1>j
		if (i - 1 > j) {
			sym = wsym[k];
		}
		else {
			sym = ident;	//����ʧ�ܣ����ʾ�������ֻ�����(����ĸ��ͷ��ô��������)
		}
	}//end if (ch >= 'a' && ch <= 'z')
	else {
		if (ch >= '0' && ch <= '9') {	//�����ֿ�ͷ����ֻ�������֣��������
			k = 0;
			num = 0;
			sym = number;	//���÷���Ϊ����
			do {
				num = num * 10 + ch - '0';
				k++;
				getchdo;
			} while (ch >= '0' && ch <= '9');
			//k--;	//k����num��λ��������Ҫ��һ��
			if (k > nmax) {
				error(30);
			}
		}//end if (ch >= '0' && ch <= '9')
		else {
			if (ch == ':') {	//����Ƿ�Ϊ��ֵ����
				getchdo;
				if (ch == '=') {
					sym = becomes;
					getchdo;
				}
				else {
					sym = colon;	//������Ǹ�ֵ���ţ��Ǿ���ð��
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
						sym = ssym[ch];	//��������������ʱ�������ַ����Ŵ�������ǷǷ��ĵ��ַ�����sym=ssym[ch]=nul
						if (sym != period) {	//periodΪ��ţ����sym=period��������Ѿ����е�ĩβ
							getchdo;			//����������û�мӾ�ţ���ᱨ��program incomplete��
						}
					}
				}
			}
		}
	}
	return 0;
}

/*
* ��������ӡ����λ�úʹ������
* (��ʵ���������һ�ѿո񣬻�����ֱ����line����space)
*/
void error(int n) {
	char space[81];
	memset(space, 32, 81);

	line[cc - 1] = 0;	//����ʱ��ǰ�����Ѿ����꣬����cc-1

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
* �������
* lev:		��ǰ�ֳ������ڲ�
* tx:		���ֱ�ǰβָ��
* fsys:		��ǰģ�������ż�
* offset:	βָ���ƫ����
*/
int block(int lev, int tx, bool* fsys, int offset) {
	int i;

	int dx;					//���ַ��䵽����Ե�ַ
	int tx0;				//������ʼtx
	int cx0;				//������ʼcx
	bool nxtlev[symnum];	/* ���¼������Ĳ����У����ż��Ͼ�Ϊֵ�Σ�������ʹ������ʵ�֣�
							���ݽ�������ָ�룬Ϊ��ֹ�¼������ı��ϼ������ļ��ϣ������µĿ�?
							���ݸ��¼�����*/

	dx = 3;		//�������ݴ�С��ʼΪ3��ÿ��һ��������dx����1
	tx0 = tx;	//��¼�������ֵĳ�ʼλ��
	table[tx].adr = cx;
	tx = tx + offset;	//���offset��Ϊ0����ʾ���ڲ���������������һ��ʵ�֣���Ҫʹtxƫ���Ա��⸲�ǲ��������ֱ���ռ��λ��
	dx = dx + offset;	//��ַ�ռ�ҲҪ��Ӧ����

	gendo(jmp, 0, 0);	//ÿ�����̶����������ָ����������ʼҲ������һ�����ָ��

	if (lev > levmax) {
		error(32);
	}

	do {	//ֱ��û����������
		if (sym == constsym) {	//�յ�������������
			getsymdo;
			constdeclarationdo(&tx, lev, &dx);	//dx��ֵ�ᱻconstdeclaration�ı䣬ʹ��ָ��
			//ʵ������constdeclaration����enter����enter��ִ����(*ptx)++��(*pdx)++, ǰ��һ��ִ�У����߿���ִ��
			while (sym == comma) {
				getsymdo;
				constdeclarationdo(&tx, lev, &dx);
			}
			if (sym == semicolon) {
				getsymdo;
			}
			else {
				error(5);	//©���˶��Ż�ֺ�, ���©���˶���, ��sym==ident
			}
		}//end if (sym == constsym)

		if (sym == varsym) {	//�յ�������������
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
				error(5);	//vardeclaration��û���жϱ�ʶ�����=��:=������Ҳû�У���ζ�ű�������������ʱ��ֵ(���ܳ�ʼ��)
			}
		}//end if (sym == varsym)

		while (sym == procsym) {	//�յ�������������
			getsymdo;

			if (sym == ident) {
				enter(procedur, &tx, lev, &dx);	//��¼��������
				getsymdo;
			}
			else {
				error(4);	//procedure��ӦΪ��ʶ��
			}

			//�������Ĺ���
			int tx1 = tx;	//����ԭʼ��tx����
			int dx1 = 3;	//���λ��
			if (sym == lparen) {
				getsymdo;
				if (sym == varsym) {
					getsymdo;
					vardeclarationdo(&tx1, lev + 1, &dx1);
					table[tx].val++;	//�ù��������ֱ��е�valֵ��ʾ�����ĸ���
					while (sym == comma) {
						getsymdo;
						if (sym != varsym) {
							error(40);	//��������
						}
						getsymdo;
						vardeclarationdo(&tx1, lev + 1, &dx1);
						table[tx].val++;
					}
				}
				else {
					error(40);	//��������ֻ����var
				}
				if (sym == rparen) {
					getsymdo;
				}
				else {
					error(41);	//ȱ��������
				}
			}
			
			if (sym == semicolon) {	//��������Ҫ�ӷֺ�
				getsymdo;
			}
			else {
				error(5);	//©�˷ֺ�
			}

			memcpy(nxtlev, fsys, sizeof(bool) * symnum);
			nxtlev[semicolon] = true;

			/*���Ե��� block(lev + 1, tx, nxtlev); */
			/*�ݹ����, �����а�������, ��˹��̿����Ҫ��begin end, ����ڶ������̻ᱻ����ǰһ�����̵��ӹ���*/
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
				error(5);	//©���˷ֺ�
			}
		}//end while (sym == procsym)
		memcpy(nxtlev, statbegsys, sizeof(bool) * symnum);
		nxtlev[ident] = true;
		testdo(nxtlev, declbegsys, 7);
	} while (inset(sym, declbegsys));	//ֱ��û����������, �൱�� while(declbegsys[sym])

	code[table[tx0].adr].a = cx;	//��ʼ���ɵ�ǰ���̴���
	table[tx0].adr = cx;			//��ǰ���̴����ַ
	table[tx0].size = dx;			//����������ÿ����һ�����������dx����1�����������Ѿ�������dx���ǵ�ǰ�������ݵ�size
	cx0 = cx;
	gendo(inte, 0, dx);				//���ɷ����ڴ����

	if (tableswitch) {		//������ֱ�
		printf("TABLE:\n");
		if (tx0 + 1 > tx) {	//tx0+1>tx��ʾβָ��û���ƶ�
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

	/*Ϊ������Ϊ�����ı������ɸ�ֵ����*/
	if (offset) {
		int dx1 = 3;
		for (i = tx0; i < tx0 + offset; i++) {
			gendo(lit, 0, 0);	//��ȷ��������ֵ�������ɴ���
			gendo(sto, 0, dx1);
			dx1++;
		}
	}

	/*���������Ϊ�ֺŻ�end*/
	memcpy(nxtlev, fsys, sizeof(bool)* symnum);	//ÿ��������ż��϶������ϲ������ż��ϣ��Ա㲹��
	nxtlev[semicolon] = true;
	nxtlev[endsym] = true;
	statementdo(nxtlev, &tx, lev);
	gendo(opr, 0, 0);	//ÿ�����̳��ڶ�Ҫʹ�õ��ͷ����ݶ�ָ��
	memset(nxtlev, 0, sizeof(bool)* symnum);	//�ֳ���û�в��ȼ���
	testdo(fsys, nxtlev, 8);	//�����������ȷ��
	listcode(cx0);	//�������
	return 0;
}

/*
* �������������
* x: instruction.f	���������ָ��
* y: instruction.l	���ò���������Ĳ�β�
* z: instruction.a	����f�Ĳ�ͬ����ͬ
*/
int gen(enum fct x, int y, int z) {
	if (cx >= cxmax) {
		printf("Program too long");	//�������
		return -1;
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx].a = z;
	cx++;
	return 0;
}

/*������������*/
int constdeclaration(int* ptx, int lev, int* pdx) {
	if (sym == ident) {
		getsymdo;
		if (sym == eql || sym == becomes) {	//eql �� =; becomes �� :=;
			if (sym == becomes) {
				error(1);	//��=д����:=
			//�������У�����������ʹ��:=������var a:=1; ����������ʹ��=, ����const c=1
			}
			getsymdo;
			if (sym == number) {
				enter(constant, ptx, lev, pdx);
				getsymdo;
			}
			else {
				error(2);	//����˵��=��Ӧ�����֣����ﵼ�³����޷�������Ϊ��������Ϊ����û�б�ʶ��Ϊ����һ����
			}
		}//end if (sym == eql || sym == becomes)
		else {
			error(3);	//����˵����ʶ��Ӧ��=, ��������=������:=��ֵ���ҳ���Ӧ������ʱ�͸�ֵ
		}
	}//end if (sym == ident)
	else {
		error(4);	//const��Ӧ�Ǳ�ʶ
	}
	return 0;
}

/*������������*/
int vardeclaration(int* ptx, int lev, int* pdx) {
	//��constdeclaration�����ж�=��:=��һ������ζ�ű���������������ʱ��ֵ���볣������������ʱ��ֵ�෴
	if (sym == ident) {
		getsymdo;
		if (sym == lparen) {	//������������ţ����ʾ��ǰҪ��������һ������
			int arraysize = 0;
			enter(array, ptx, lev, pdx);	//��д���ֱ�
			getsymdo;	//��ȡ�����½�
			if (sym == ident) {	//������½�ֻ���ǳ��������޷�������
				int i = position(id, (*ptx));	//ptx��(*ptx)��
				if (table[i].kind == constant) {
					table[(*ptx)].lbound = table[i].val;
					arraysize = table[i].val;
					getsymdo;
				}
				else {
					error(42);	//�����½��ʽ���������½�ֻ���ǳ������޷�������
				}
			}
			else if (sym == number) {
				table[(*ptx)].lbound = num;
				arraysize = num;
				getsymdo;
			}
			else {
				error(42);	//�����½��ʽ����
			}
			if (arraysize >= 0) {
				if (sym == colon) {
					getsymdo;
					if (sym == ident) {	//������Ͻ�ֻ���ǳ��������޷�������
						int i = position(id, (*ptx));	//ptx��(*ptx)��
						if (table[i].kind == constant) {
							arraysize = table[i].val - arraysize;
							//�õ���ֵʵ���ϱ����鳤��С1�����Ѿ������ֱ��м�¼�˵�һ��Ԫ�أ����Բ����ټ�һ
							getsymdo;
						}
						else {
							error(42);	//�����½��ʽ����ֻ���ǳ������޷�������
						}
					}
					else if (sym == number) {
						arraysize = num - arraysize;
						getsymdo;
					}
					else {
						error(42);	//�����Ͻ��ʽ����
					}
					if (arraysize < 0) {
						error(43);	//�����Ͻ��ֵС���½��ֵ
					}
					table[(*ptx)].size = arraysize + 1;
					while (arraysize--) {
						strcpy(id, "");	//Ϊ�˱���ʹ��position����ʱ�޷��ҵ�����ĵ�һ��Ԫ�أ���id�ÿ�
						enter(array, ptx, lev, pdx);	//��ÿ��Ԫ�ض������ֱ���ռ��һ��λ��
					}
					if (sym == rparen) {
						getsymdo;
					}
					else {
						error(41);	//ȱ��������
					}
				}
				else {
					error(42);	//�������½��ʽ����, ȱ�ٷֺ�
				}
			}
			else {
				error(42);	//�����½��ʽ����
			}
		}//end if (sym == lparen)
		else {
			enter(variable, ptx, lev, pdx);	//��д���ֱ�
		}
	}//end if (sym == ident) 
	else {
		error(4);	//var��Ӧ�Ǳ�ʶ��
	}
	return 0;
}

/*
* �����ֱ��м���һ��
* k:	��������.const var procedure
* ptx:	���ֱ�βָ���ָ�룬Ϊ�˿��Ըı����ֱ�βָ���ֵ
* lev:	�������ڵĲ��
* pdx:	dxΪ��ǰӦ����ı�������Ե�ַ�������Ҫ����1
*/
void enter(enum object k, int* ptx, int lev, int* pdx) {
	(*ptx)++;	//���ƶ���ָ�룬���table[0]�ǿյ�
	strcpy(table[(*ptx)].name, id);
	table[(*ptx)].kind = k;
	switch (k)
	{
	case constant:	//��������
		if (num > amax) {
			error(31);	//��Խ��, ׼ȷ��˵��const���͵���Խ�磬const�������ֵ����Ϊ2047
			num = 0;
		}
		table[(*ptx)].val = num;
		break;
	case variable:	//��������
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	case procedur:	//��������
		table[(*ptx)].level = lev;
		break;
	case array:		//��������
		table[(*ptx)].level = lev;
		table[(*ptx)].adr = (*pdx);
		(*pdx)++;
		break;
	}
}

/*
* ���Ե�ǰ�����Ƿ�Ϸ�
* ��ĳһ����(��һ�����, һ�����ʽ)��Ҫ����ʱ����ϣ����һ����������ĳ����
* (�ò��ֵĺ������), test����������, ���Ҹ��𵱼�ⲻͨ��ʱ�Ĳ��ȴ�ʩ,
* ��������Ҫ���ʱָ����ǰ��Ҫ�ķ��ż��ϺͲ����õļ���(��֮ǰδ��ɲ��ֵĺ��
* ����), �Լ���ⲻͨ��ʱ�Ĵ����
* s1:	������Ҫ�ķ���
* s2:	�������������Ҫ��, ����Ҫһ�������õļ���
* n:	�����
*/
int test(bool* s1, bool* s2, int n) {
	if (!inset(sym, s1)) {
		error(n);
		/*����ⲻͨ��ʱ, ��ͣ��ȡ����, ֱ����������Ҫ�ļ��ϻ򲹾ȵļ���*/
		while ((!inset(sym, s1)) && (!inset(sym, s2))) {
			getsymdo;
		}
	}
	return 0;
}

/*��䴦��*/
int statement(bool* fsys, int* ptx, int lev) {
	int i, cx1, cx2;
	bool nxtlev[symnum];

	if (sym == ident) {		//׼������ֵ��䴦��
		i = position(id, *ptx);	//�����ֱ��в���id��λ��
		if (i == 0) {
			error(11);	//����δ�ҵ�
		}
		else {
			if (table[i].kind == variable || table[i].kind == array) {
				if (table[i].kind == array) {
					getsymdo;
					if (sym == lparen) {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						expressiondo(nxtlev, ptx, lev);	//���������ڱ��ʽ
						gendo(chk, table[i].lbound, table[i].size);	//����±��Ƿ�Խ��
					}
					else {
						error(41);	//ȱ��������
					}
				}
				getsymdo;
				if (sym == becomes) {
					getsymdo;
				}
				else {
					error(13);	//û�м�⵽��ֵ����
				}
				memcpy(nxtlev, fsys, sizeof(bool) * symnum);
				expressiondo(nxtlev, ptx, lev);	//����ֵ�����Ҳ���ʽ
				//��ʱ���ʽ��ֵ��ջ��������һ��expression�������ֵ�ڴ�ջ��
				if (i != 0) {
					//expression��ִ��һϵ����������ս�����ᱣ����ջ����ִ��sto������ɸ�ֵ
					if (table[i].kind == variable) {
						gendo(sto, lev - table[i].level, table[i].adr);
					}
					else {
						gendo(ssta, lev - table[i].level, table[i].adr - table[i].lbound);
					}
				}
			}
			else {
				error(12);	//��ֵ����ʽ����, ֻ�б���������������ֵ����
				i = 0;
			}
		}//end else
	}//end if (sym == ident)
	else {
		if (sym == readsym) {	//׼����read��䴦��
			getsymdo;
			if (sym != lparen) {
				error(34);	//����read������������
			}
			else {
				do {//while (sym == comma)
					getsymdo;
					if (sym == ident) {
						i = position(id, *ptx);	//����Ҫ���ı���
					}
					else {
						i = 0;	//���sym���Ǳ�ʶ����������δ�ҵ�
					}

					if (i == 0) {
						error(35);	//read()��Ӧ���������ı���
					}
					else if (table[i].kind == variable) {
						gendo(opr, 0, 16);	//��������ָ��, ��ȡֵ��ջ��
						gendo(sto, lev - table[i].level, table[i].adr);	//�洢������
					}
					else if (table[i].kind == array) {
						getsymdo;
						if (sym == lparen) {
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool) * symnum);
							expressiondo(nxtlev, ptx, lev);	//���������ڱ��ʽ
							gendo(chk, table[i].lbound, table[i].size);	//����±��Ƿ�Խ��
							gendo(opr, 0, 16);	//��������ָ���ȡֵ��ջ��
							gendo(ssta, lev - table[i].level, table[i].adr - table[i].lbound);	//��ջ����ֵ�����Դ�ջ��Ϊ�±��������
						}
						else {
							error(41);	//ȱ��������
						}
					}
					else {
						error(32);	//read()��Ӧ�Ǳ���������
					}
					getsymdo;
				} while (sym == comma);	//һ��read���ɶ��������
			}//end else
			if (sym != rparen) {
				error(33);	//δ��������
				while (!inset(sym, fsys)) {	//�����ȣ�ֱ���յ��ϲ㺯���ĺ������
					getsymdo;
				}
			}
			else {
				getsymdo;
			}
		}//end if (sym == readsym)
		else {
			if (sym == writesym) {	//׼������write��䴦����read����
				getsymdo;
				if (sym == lparen) {
					do {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						nxtlev[rparen] = true;
						nxtlev[comma] = true;	//wirte�ĺ������Ϊ ) �� , 
						expressiondo(nxtlev, ptx, lev);	//���ñ��ʽ�����˴���read��ͬ
						gendo(opr, 0, 14);		//�������ָ����ջ����ֵ
					} while (sym == comma);
					if (sym != rparen) {
						error(33);	//write()��ӦΪ�������ʽ
					}
					else {
						getsymdo;
					}
				}//end if (sym == lparen)
				gendo(opr, 0, 15);	//�������
			}//end if (sym == writesym)
			else {
				if (sym == callsym) {	//׼������call��䴦��
					getsymdo;
					if (sym != ident) {
						error(14);		//call��ӦΪ��ʶ��
					}
					else {
						i = position(id, *ptx);
						getsymdo;
						if (i == 0) {
							error(11);	//����δ�ҵ�
						}
						else {
							if (table[i].kind == procedur) {
								int para = 0;	//�����ʵ�θ���
								if (sym == lparen) {
									do {
										getsymdo;
										para++;
										if (para <= table[i].val) {
											memcpy(nxtlev, fsys, sizeof(bool)* symnum);
											expressiondo(nxtlev, ptx, lev);	//����ʵ���еı��ʽ
											int address = table[i].adr + 1 + (para - 1) * 2;
											gendo(ssto, 0, address);	//���л���
										}
										else {
											error(41);	//������������
										}
									} while (sym == comma);	//���������Ǳ�ʶ��������
									if (sym == rparen) {
										getsymdo;
									}
									else {
										error(40);	//ȱ��������
									}
								}
								if (para < table[i].val) {
									error(41);	//������������
								}
								gendo(cal, lev - table[i].level, table[i].adr);	//����callָ��
							}
							else {
								error(15);	//call���ʶ��ӦΪ����
							}
						}
					}//end else
				}//end if (sym == callsym)
				else {
					if (sym == ifsym) {	//׼������if��䴦��
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool)* symnum);
						nxtlev[thensym] = true;
						nxtlev[dosym] = true;		//�������Ϊthen��do, do ���ܴ��� then, Ӧ���������÷�
						conditiondo(nxtlev, ptx, lev);	//�������������߼����㣩����
						if (sym == thensym) {
							getsymdo;
						}
						else {
							error(16);	//ȱ��then
						}
						cx1 = cx;	//���浱ǰָ���ַ
						gendo(jpc, 0, 0);	//����������תָ���ת��ַδ֪����ʱд0
						statementdo(fsys, ptx, lev);	//����then������
						code[cx1].a = cx;	//��statement�����cxΪthen�����ִ�����λ�ã�������ǰ��δ������ת��ַ
						//��ַ����
					}//end if (sym == ifsym)
					else {
						if (sym == beginsym) {	//׼�����ո�����䴦��
							getsymdo;
							memcpy(nxtlev, fsys, sizeof(bool)* symnum);
							nxtlev[semicolon] = true;
							nxtlev[endsym] = true;

							/*ѭ��������䴦������ֱ����һ�����Ų�����俪ʼ���Ż��յ�end*/
							statementdo(nxtlev, ptx, lev);
							while (inset(sym, statbegsys) || sym == semicolon) {
								if (sym == semicolon) {
									getsymdo;
								}
								else {
									error(10);	//ȱ�ٷֺ�
								}
								statementdo(nxtlev, ptx, lev);
							}
							if (sym == endsym) {
								getsymdo;
							}
							else {
								error(17);		//ȱ��end��ֺ�
							}
						}//end if (sym == beginsym)
						else {
							if (sym == whilesym) {	//׼������while��䴦��
								cx1 = cx;	//�����ж�����������λ��
								getsymdo;
								memcpy(nxtlev, fsys, sizeof(bool)* symnum);
								nxtlev[dosym] = true;	//�������Ϊdo
								conditiondo(nxtlev, ptx, lev);	//������������
								cx2 = cx;	//����ѭ�����������һ��λ��
								gendo(jpc, 0, 0);	//����������ת��������ѭ���ĵ�ַδ֪
								if (sym == dosym) {
									getsymdo;
								}
								else {
									error(18);	//ȱ��do
								}
								statementdo(fsys, ptx, lev);	//ѭ����
								gendo(jmp, 0, cx1);	//��ͷ�����ж�����
								code[cx2].a = cx;	//��������ѭ���ĵ�ַ����if����
							}//end if (sym == whilesym)
							else {
								memset(nxtlev, 0, sizeof(bool)* symnum);	//�������޲��ȼ���
								testdo(fsys, nxtlev, 19);	//�������������ȷ��
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
* �������ֵ�λ��
* �ҵ��򷵻������ֱ��е�λ�ã����򷵻�0
* idt:	Ҫ���ҵ�����
* tx:	��ǰ���ֱ��βָ��
*/
int position(char* idt, int tx) {
	int i;
	strcpy(table[0].name, idt);	//��idt���ڵ�0λ���������0����ʾû�ҵ�
	i = tx;
	while (strcmp(table[i].name, idt) != 0) {
		i--;
	}
	return i;
}

/*
* ���ʽ����
* ����expressionʱ��expression���ȵ���term����term�ֻ��ȵ���factor
* ��˳�����ȴ���factor�е����ӣ��������ڵı��ʽ
* Ȼ���ٴ���term�еĳ˳���
* ����ٴ���expression�еļӼ���
* ����ʵ�ּ�������ȼ�
*/
int expression(bool* fsys, int* ptx, int lev) {
	enum symbol addop;	//���ڱ���������
	bool nxtlev[symnum];

	if (sym == plus || sym == minus) {	//��ͷ�������ţ���ʱ���ʽ������һ�����Ļ򸺵���
		addop = sym;	//���濪ͷ��������
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);	//������, �ڼ���Ӽ���֮ǰ�ȼ���˳�
		if (addop == minus) {
			gendo(opr, 0, 1);	//�����ͷΪ��������ȡ��ָ��
		}
	}
	else {	//��ʱ���ʽ��������ļӼ�
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);	//������
	}
	while (sym == plus || sym == minus) {
		addop = sym;
		getsymdo;
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[plus] = true;
		nxtlev[minus] = true;
		termdo(nxtlev, ptx, lev);	//������
		if (addop == plus) {
			gendo(opr, 0, 2);	//���ɼӷ�ָ��
		}
		else {
			gendo(opr, 0, 3);	//���ɼ���ָ��
		}
	}
	return 0;
}

/*��������*/
int condition(bool* fsys, int* ptx, int lev) {
	enum symbol relop;	//���浱ǰ��ϵ�����
	bool nxtlev[symnum];

	if (sym == oddsym) {	//׼������odd���㴦��
		getsymdo;
		expressiondo(fsys, ptx, lev);
		gendo(opr, 0, 6);	//odd����
	}
	else {
		/*�߼����ʽ����*/
		memcpy(nxtlev, fsys, sizeof(bool) * symnum);
		nxtlev[eql] = true;
		nxtlev[neq] = true;
		nxtlev[lss] = true;
		nxtlev[leq] = true;
		nxtlev[gtr] = true;
		nxtlev[geq] = true;
		expressiondo(nxtlev, ptx, lev);
		if (sym != eql && sym != neq && sym != lss && sym != leq && sym != gtr && sym != geq) {
			error(20);	//���ǹ�ϵ����
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

/*���*/
int term(bool* fsys, int* ptx, int lev) {
	enum symbol mulop;	//���ڱ���˳�������
	bool nxtlev[symnum];

	memcpy(nxtlev, fsys, sizeof(bool) * symnum);
	nxtlev[times] = true;
	nxtlev[slash] = true;
	factordo(nxtlev, ptx, lev);	//��������, �ټ���˳�֮ǰ�ȼ��������ڱ��ʽ
	while (sym == times || sym == slash) {
		mulop = sym;
		getsymdo;
		factordo(nxtlev, ptx, lev);
		if (mulop == times) {
			gendo(opr, 0, 4);	//���ɳ˷�ָ��
		}
		else {
			gendo(opr, 0, 5);	//���ɳ���ָ��
		}
	}
	return 0;
}

/*���Ӵ���*/
int factor(bool* fsys, int* ptx, int lev) {
	int i;
	bool nxtlev[symnum];
	testdo(facbegsys, fsys, 24);	//������ӵĿ�ʼ����
	if (inset(sym, facbegsys)) {	/* BUG: ԭ���ķ���var1(var2+var3)�ᱻ����ʶ��Ϊ���� */
		if (sym == ident) {		//����Ϊ����������������
			i = position(id, *ptx);	//��������
			if (i == 0) {
				error(11);	//��ʶ��δ����
			}
			else {
				switch (table[i].kind)
				{
				case constant:	//��ʶ��Ϊ����
					gendo(lit, 0, table[i].val);	//ֱ�Ӱѳ�����ֵ��ջ
					break;
				case variable:	//��ʶ��Ϊ����
					gendo(lod, lev - table[i].level, table[i].adr);	//�ҵ�������ַ������ֵ��ջ
					break;
				case procedur:	//��ʶ��Ϊ����
					error(21);	//����Ϊ����
					break;
				case array:		//��ʶ��Ϊ����
					getsymdo;
					if (sym == lparen) {
						getsymdo;
						memcpy(nxtlev, fsys, sizeof(bool) * symnum);
						expressiondo(nxtlev, ptx, lev);
						gendo(chk, table[i].lbound, table[i].size);	//����±��Ƿ�Խ��
						gendo(vis, lev - table[i].level, table[i].adr - table[i].lbound);
						if (sym != rparen) {
							error(41);	//ȱ��������
						}
					}
					else {
						error(42);	//�����±����
					}
				}
			}
			getsymdo;
		}//end if (sym == ident)
		else {
			if (sym == number) {	//����Ϊ��
				if (num > amax) {
					error(31);		//������Ϊ����ʱ���ܴ���amax��������õ���ֵ���Դ���amax
					num = 0;
				}
				gendo(lit, 0, num);
				getsymdo;
			}
			else {
				if (sym == lparen) {	//����Ϊ���ʽ
					getsymdo;
					memcpy(nxtlev, fsys, sizeof(bool) * symnum);
					nxtlev[rparen] = true;
					expressiondo(nxtlev, ptx, lev);
					if (sym == rparen) {
						getsymdo;
					}
					else {
						error(22);	//ȱ��������
					}
				}
				testdo(fsys, facbegsys, 23);	//���Ӻ��зǷ�����
			}//end else
		}//end else
	}//end if (inset(sym, facbegsys))
	return 0;
}

/*���Ŀ������嵥*/
void listcode(int cx0) {
	int i;
	if (listswitch) {
		for (i = cx0; i < cx; i++) {
			printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
			fprintf(fa, "%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
		}
	}
}

/*���ͳ���*/
void interpret() {
	int p, b, t;	//ָ��ָ�룬ָ���ַ��ջ��ָ��
	struct instruction i;	//��ŵ�ǰָ��
	int s[stacksize];	//ջ

	printf("start pl0\n");
	t = 0;
	b = 0;
	p = 0;
	s[0] = s[1] = s[2] = 0;
	do {//while (p != 0)
		i = code[p];	//����ǰָ��
		p++;
		switch (i.f)
		{
		case lit:	//��a��ֵȡ��ջ��
			s[t] = i.a;
			t++;
			break;
		case opr:	//��ѧ���߼�����
			switch (i.a)
			{
			case 0:
				t = b;
				p = s[t + 2];
				b = s[t + 1];
				break;
			case 1:		//δʵ��
				s[t - 1] = -s[t - 1];
				break;
			case 2:		//�ӷ�ָ��
				t--;
				s[t - 1] = s[t - 1] + s[t];
				break;
			case 3:		//����ָ��
				t--;
				s[t - 1] = s[t - 1] - s[t];
				break;
			case 4:		//�˷�ָ��
				t--;
				s[t - 1] = s[t - 1] * s[t];
				break;
			case 5:		//����ָ��
				t--;
				s[t - 1] = s[t - 1] / s[t];
				break;
			case 6:		//oddָ��
				s[t - 1] = s[t - 1] % 2;
				break;
			case 8:		//����
				t--;
				s[t - 1] = (s[t - 1] == s[t]);
				break;
			case 9:		//����
				t--;
				s[t - 1] = (s[t - 1] != s[t]);
				break;
			case 10:	//С��
				t--;
				s[t - 1] = (s[t - 1] < s[t]);
				break;
			case 11:	//���ڵ���
				t--;
				s[t - 1] = (s[t - 1] >= s[t]);
				break;
			case 12:	//����
				t--;
				s[t - 1] = (s[t - 1] > s[t]);
				break;
			case 13:	//С�ڵ���
				t--;
				s[t - 1] = (s[t - 1] <= s[t]);
				break;
			case 14:	//���ջ����ֵ, ����������
				printf("%d", s[t - 1]);
				fprintf(fa2, "%d", s[t - 1]);
				t--;
				break;
			case 15:	//�������
				printf("\n");
				fprintf(fa2, "\n");
				break;
			case 16:	//��ȡֵ��ջ��
				printf("?");
				fprintf(fa2, "?");
				scanf("%d", &(s[t]));
				fprintf(fa2, "%d\n", s[t]);
				t++;
				break;
			}//end switch (i.a)
			break;
		case lod:	//ȡ��Ե�ǰ���̵����ݻ���ַΪa���ڴ��ֵ��ջ��
			s[t] = s[base(i.l, s, b) + i.a];
			t++;
			break;
		case vis:	//����Ե�ǰ���̵����ݻ���ַΪa+s[t-1]���ڴ��ֵ����ջ��ֵ��ԭջ��ֵ�Ǽ���õ����±�
			s[t - 1] = s[base(i.l, s, b) + i.a + s[t - 1]];	//i.a��ֵΪ�������������ֱ��е�adr-lbound
			break;
		case sto:	//ջ����ֵ�浽��Ե�ǰ���̵����ݻ���ַΪa���ڴ�
			t--;
			s[base(i.l, s, b) + i.a] = s[t];
			break;
		case ssto:	//��ջ����ֵ����litָ��
			code[i.a].a = s[t - 1];
			break;
		case ssta:	//��ջ����ֵ�����Դ�ջ��Ϊ�±��������
			t--;
			s[base(i.l, s, b) + i.a + s[t - 1]] = s[t];
			break;
		case chk:	//���ջ����ֵ�Ƿ���������±귶Χ��
			if (s[t - 1] < i.l) {
				error(43);	//�����±�Խ�½�
			}
			if (s[t - 1] > i.l + i.a - 1) {
				error(43);	//�����±�Խ�Ͻ�
			}
			break;
		case cal:	//�����ӹ���
			s[t] = base(i.l, s, b);	//�������̻���ַ��ջ
			s[t + 1] = b;	//�������̻���ַ��ջ������������base����
			s[t + 2] = p;	//����ǰָ��ָ����ջ
			b = t;			//�ı����ַָ��ֵΪ�¹��̵Ļ���ַ
			p = i.a;		//��ת
			break;
		case inte:	//�����ڴ�
			t += i.a;
			break;
		case jmp:	//ֱ����ת
			p = i.a;
			break;
		case jpc:	//������ת
			t--;
			if (s[t] == 0) {	//��������������ת
				p = i.a;
			}
			break;
		}//end switch (i.f)
	} while (p != 0);
}

/*ͨ�����̻�ַ����1����̵Ļ�ַ*/
int base(int l, int* s, int b) {
	int b1;
	b1 = b;
	while (l > 0) {
		b1 = s[b1];
		l--;
	}
	return b1;
}