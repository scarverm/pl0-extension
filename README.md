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
