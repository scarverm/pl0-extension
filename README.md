# pl0扩展程序
## 2023/5/11
添加初始文件，初始文件源自课本后附源码，添加了许多个人注释
## 2023/5/12
添加注释功能，用$作为注释的标识
修改步骤：
1.添加符号comment，符号数量symnum从32改为33
2.设置单字符号，即添加ssym['$'] = comment;表示使用$符号作为注释的标识
3.考虑到是行注释，因此可以将当前行'$'后的所有内容都忽略，因此需要修改getsym函数，为忽略空格、换行、回车和TAB，再增加一条忽略$，且当忽略$时，令cc=ll，即可完成忽略$后的所有内容
## 2023/5/14
支持了带参数的过程

1.示例

```pl0
var x, y, z;
procedure p(var a, var b, var c);
	procedure pp(var d, var e, var f);
	begin
		write(d);
		write(e);
		write(f);
	end;
begin
	call pp(a, b, c);
end;
begin
	x := 1;
	y := 2;
	z := 3;
	call p(x + 2, y * z, (z - x) * y);
end.
```

2.步骤

①扩展block函数，增加一个offset参数作为偏移量，偏移量代表参数的个数，令tx=tx+offset，dx=dx+offset

②读取形参变量，每读取一个形参变量，就令table[i].val加1，其中i是过程名在名字表中的位置。在读取形参变量的同时调用declaration函数，将变量名加入名字表，参数为(&tx1, lev+1, &dx1)

③为每个形参变量都生成赋值指令，包括lit指令和sto指令，其中lit指令的第三个参数不确定，先填0。生成指令的代码要放在在内存分配指令inte和statement函数调用语句之间，方便对lit指令进行寻址。

④扩充类P-code虚拟机，增加一条指令代码ssto，格式是(ssto, 0, a)，作用是将当前栈顶的值填入code[a].a，用于对上一步的lit指令进行回填。

⑤修改call语句处理代码，对每一个参数都调用expression函数进行处理，对处理的参数进行计数，形参和实参的数量要对应。在expression语句之后，生成上一步增加的ssto代码，对lit指令进行回填，当然要先找到lit指令的地址，在第③步保证了代码位置正确后，可以通过`int address = table[i].adr + 1 + (para - 1) * 2`得到lit指令的地址，其中i是过程名在名字表中的位置，para是当前对实参的计数。

## 2023/5/16
增加了数组

1.测试代码

```pl0
var a(2:5), c, d;
procedure p;
const x=3, y=4;
var b(x:y);
begin
	b(3) := 30;
	b(y) := 40;
	write(b(x));
	write(b(4));
end;
begin
	a(2) := 1;
	a(3) := 2;
	c := 4;
	d := 5;
	read(a(c), a(d));
	write(a(2));
	write(a(3));
	write(a(c));
	write(a(d));
	call p;
	read(a(a(2)+a(3)+1));	$相当于read(a(4))
	write(a(a(2)+a(3)+1));
end.var a(1:5), b(2:3);
read(a(1));
write(a(1));
```

2.步骤：

①修改enter函数，在switch中增加一项case array，内容与case variable相同，因为其本质也是变量，同时也要在block函数的输出名字表处增加case array

②增加冒号符号colon，修改符号数

③在getsym函数中，在检测赋值符号这一步，将else中的内容更改为sym = colon; 由于在这一步检测了冒号，所以可以不用设置单字符号

④修改varclaration函数，使其能区分数组和变量

⑤增加vis指令，格式为(vis, l, a)，用于将相对当前过程的数据基地址为a+s[t-1]的内存的值取到栈顶，原栈顶值是用expression函数计算出的下标

⑥增加ssta指令，格式为(ssta, l, a), 用于将栈顶的值存入以次栈顶为下标的数组中

⑦增加chk指令，格式为(chk, 0, a), 通过table[a]检查栈顶的值是否越界

⑧修改factor函数，让程序能处理包含数组的表达式

⑨修改statement函数，让程序能对数组元素进行赋值

⑩修改read处理语句

⑪修改block函数，输出含有数组的名字表

## 2023/5/17

增加了repeat until语句

### 1.示例

```pl0
var x;
begin
	x := 0;
	repeat
	    begin
			x := x + 1;
			write(x);
	    end
	until x > 4;
end.
```

### 2.步骤

①增加repeatsym和untilsym两个符号，并修改符号数symnum

②添加repeat和until保留字，按字母顺序排列

③设置repeat和until保留字符号

④修改关键字个数

⑤设置repeatsym为语句开始符号

⑥在statement函数中增加if (sym==repeatsym)语句
