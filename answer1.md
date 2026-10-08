## [仓库链接](https://github.com/5654002zrj/lr-core-vector.git)
## 学习心得
分配内存的时候需要格外小心，很多容易出错的情况，比如悬空指针，内存越界，分配失败等等，导致写的时候很顺利，后面出错一直在改一直在改，改的时间比写的时间还长，并且变量周期的问题也很重要，free完再次引用函数使用那个变量就出错了，所以要提前保存好旧变量的值。总之在对内存进行操作的时候，要格外注意细节。
## 学习总结
### 时间复杂度
* 什么是时间复杂度  
粗略判断代码的执行速度，通过粗略计算代码执行的总次数得到时间复杂度。
* 如何计算    
用T（n）表示代码执行的总次数若T(n)为常数则时间复杂度是O(1);  

T(n)=3n+3时省去常数保留一次项并且省略系数，时间复杂度是O(n);  

T(n)=多项式时只保留最高次项并舍去系数，比如$T(n)=5n^3+55n+555$,此时时间复杂度是O($n^3$)  

对于普通的分支结构和顺序结构一般是O(1),循环一层一般是O(n),两层是O($n^2$),三层是O($n^3$)。但是实际上还是要具体大概计算一下循环总共有几次，时间复杂度一般和循环总次数挂钩（多层循环就是内外循环的总次数）

### make初步了解
* 基本介绍
make通过读取makefile文件中定义的规则来构建整个项目
* makefile的规则
目标文件：依赖文件  
（tap）要执行的命令
* 一个例子
现在我有一个文件message.h,声明message函数，又有一个message.c写了message函数,另有一个文件main.c(里面会写#include"message.h")中调用message函数。
那么用makefile就可以这样写(记得空格是表示两个文件)
```makefile
hello:message.o main.o
    gcc main.c message.c -o hello

main.o:main.c
    gcc -c main.c

message.o:message.c
    gcc -c message.c
```
这里就是先把message.c和main.c编译成目标文件，再链接成hello这个可执行文件  
因为make只会在源程序更新的时候才会执行更新，这样写在只有一个文件更新的时候只make一个文件，但如果把三行都合成一行
```makefile
hello:message.c main.c
    gcc message.c main.c -o hello
```
这样任意一个文件更新都会把两个文件都重新编译，有弊端
* 伪目标(.PHONY)
常用的用all和clean
   - all  
     当想要生成的可执行文件不止一个的时候，就可以使用all，比如
     ```makefile
     all:hello world
     ```
     然后make all  
     这就代表生成hello和world两个可执行文件
   - clean
     一般在末尾用于清除可执行文件，如
     ```makefile
     clean:
        rm -f *.o hello world
     ``` 
     就是删hello和world两个可执行文件
    - **注意：如果目录中有clean或者all文件的话会先把clean，all当作文件处理，make会失效，可以在最前面加上伪指令，如**
      ```makefile
      .PHONY:clean all
      ```
     进行声明伪指令后就可正常操作
     
* 定义变量
一些自动变量:  
$@:目标文件
$<:第一个依赖文件
$^:所有的依赖文件
```makefile
.PHONY:clean all

all:hello world

hello:message.o main.o
    gcc main.c message.c -o hello

main.o:main.c
    gcc -c main.c

message.o:message.c
    gcc -c message.c

clean:
    rm -f *.o hello world
```
可以写成
```makefile
.PHONY:clean all
CFLGAS：-Wall -g -02
targets:hello world
sources:main.c message.c
objects:main.o message.o

all:$(targets)

$(targets):$(sources)
    gcc $(CFLAGS) $(objects) -o $@(表示自动变量)

main.o:main.c
    gcc $(CFLAGS) -c $< -o $@

message.o:message.c
    gcc $(CFLAGS) -c $< -o $@

clean:
    rm -f *.o hello world
```
这里就是用变量代替了文件名和编译选项合集，变量的用法就是$(变量)进行引用
| 写法          | 意思                 |
| ----------- | ------------------ |
| `CFLAGS`    | C 编译选项变量           |
| `-Wall`     | 开启较多警告             |
| `-g`        | 加入调试信息             |
| `-O2`       | 开启 2 级优化           |
| `$(CFLAGS)` | 使用 `CFLAGS` 变量里的内容 |

还可以用通配符'%'再次简化，因为生成目标文件的main.c和message.c的规则相同，因此可以把
```makefile
main.o:main.c
    gcc $(CFLAGS) -c $< -o $@

message.o:message.c
    gcc $(CFLAGS) -c $< -o $@
```
写成
```make file
%.o:%.c
    gcc $(CFLAGS) -c $< -o $@
```
这样表示把所有.c文件生成.o文件

## [参考资料1,chatgpt](https://chatgpt.com/c/6ac4c8a4-7a0c-83ee-969d-dfa34ff7c70e)
## [参考资料2,b站make入门](https://www.bilibili.com/video/BV1tyWWeeEpp/?spm_id_from=333.1391.0.0)
## [参考资料3，b站时间复杂度入门](https://www.bilibili.com/video/BV1nE411x7qP/?spm_id_from=333.1391.0.0)