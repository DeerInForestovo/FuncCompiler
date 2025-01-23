# FuncComplier

这个分支保留了一些额外的文件，例如测试样例和示例代码。在master分支里，对应文件将被删除。

----

## 介绍

本仓库是2024秋季学期《编译原理》课程项目的自由版本（实现非 SPL 的语言），组员见本仓库 Contributors （提交版本中另付名单）。我们实现了一个函数式编程语言 Func 的编译器，它支持 SPL 基本要求中的所有功能，同时作为函数式编程语言，支持一些额外的特性。

**在 VsCode 中高亮我们的语言：** 见[这个GitHub仓库](https://github.com/DeerInForestovo/func-highlighter)。

**工具链：** Func 语言源代码 - Flex - Bison - （搭建 AST - 类型检查 - 生成 G-machine 代码 - 编译为 LLVM IR）- 在 LLVM-10 上运行。括号内为我们用 C++ 语言实现的部分。

本仓库中，docs和res文件夹下的内容以及根目录下的test.sh仅用于半期检查，array.func, fib.func, test.func 为一些示例，可以忽略。

**参考资料：**

1. Jones, S. L. P., & Lester, D. R. (2000). *Implementing Functional Languages: a tutorial.* Department of Computer Science, University of Glasgow. [link](https://www.researchgate.net/profile/David-Lester-4/publication/228377970_Implementing_functional_languages_a_tutorial/links/02bfe51041cede4142000000/Implementing-functional-languages-a-tutorial.pdf)

2. Daniel's Blog - Compiling a Functional Language Using C++. [link](https://danilafe.com/blog/00_compiler_intro/)

3. Hudak, P., Hughes, J., Peyton Jones, S., & Wadler, P. (2007, June). *A history of Haskell: being lazy with class.* In Proceedings of the third ACM SIGPLAN conference on History of programming languages (pp. 12-1).

## 编译

如果不想重新编译本项目，可以下载 Linux x64 Release 版本运行，并跳过本节。

如果想重新编译本项目，Bison 版本为 3.8.2，flex 版本为 2.6.4，这二者一般不对版本敏感；LLVM **必须**采用 LLVM-10.0.1（相比其他版本，包括10.0.0，均有语法上的不同，因此会发生运行时错误）。建议从[here](https://releases.llvm.org/download.html#10.0.1)下载源码编译 LLVM-10.0.1。

安装好 LLVM-10.0.1 后，在根目录下运行 ```./build.sh``` 即可编译本项目。它会在根目录下生成 build 文件夹。

## 运行

1. 执行 ```./build/compiler < path_to_file/your_file_name.func``` 编译代码，如果编译成功，则会在根目录下生成 ```program.o``` 。

2. 执行 ```gcc -no-pie src/runtime.c program.o``` 生成可执行文件 ```a.out``` 。

3. 执行 ```./a.out``` 。

## 语法

1. 变量名

    Func 语言中，变量名由英文字母构成，根据首字母大小写分为 LID ([a-z][a-zA-Z]) 和 UID ([A-Z][a-zA-Z]) 两种，它们有严格的区分。UID 代表一种数据类型，用于数据类型和构造器的命名。LID 代表一个变量，用于变量和函数的命名（函数是第一类公民）。

2. 定义数据类型

    data 可以定义一个拥有任意多种构造器的类型，构造器拥有任意多参数类型，且参数类型可以是变量。

    ```
    data Name varTypeA varTypeB = {
        ConstructorA GlobalTypeA,
        ConstructorB varTypeA GlobalTypeB,
        ConstructorC varTypeB
    }
    ```

    例如：

    ```
    data Elements type = {
        Single type,
        Double type type,
        Triple type type type
    }
    ```

3. 基本类型

    不需要定义就可以使用的类型包括：```Int```，```Float```，```Char```，```List```，```Bool```。

    前三者为基本类型，在代码中可以直接使用，例如```1```，```1.5```，```1e5```，```'1'```，```'\n'```。

    后二者实际上是内嵌的 data type 。```Bool``` 有两个没有参数的构造器 ```True``` 和 ```False``` 。```List``` 则有如下定义规则：

    ```
    data List a = {
        Nil,
        Cons a List a
    }
    ```

    初始化一个 ```List``` 可以使用形如 ```[a,b,c]``` 的方法或 ```[a:b:anotherList]``` 来定义前几项和后续跟着的另一个 ```List```。

    特别地，在代码中使用字符串，例如 ```"123"``` ，它会被视为一个 ```List Char``` ，例如 ```['1','2','3']``` 。

4. 运算

    我们实现了C语言中常见的各种运算，包括五则运算、比较运算和位运算。

    位运算和取模运算只能使用 ```Int``` 作为输入，而其他运算则对 ```Int``` 和 ```Float``` 都适用，同时我们也希望 ```Int``` 类型在必要时将自己隐式地转化为 ```Float``` 。因此，代码中的 ```Int``` 类型实则为一个携带 num tag 的变量类型（为了方便，我们将它称为 num 类型，但需要明白它实际上不是类型而是变量类型）。在类型检查阶段，当 num 类型和变量类型绑定时，num tag 会被传递（被绑定的变量类型也会成为 num 类型）；当 num 类型其他类型绑定时，除非是 ```Int``` 与 ```Float``` ，否则类型检查失败。这一实现参考了 Haskell 语言。

    例如：

    ```
    defn add a b = { a + b }
    defn iAmInt = { 1 | 2 }
    defn iAmFloat = { 1.0 }
    defn correct = { add iAmInt iAmInt }
    defn alsoCorrect = { add iAmFloat iAmFloat }
    # defn wrong = { iAmInt + iAmFloat }
    ```

    这段代码中，```add``` 的类型为 ```forall a(Num) . a*  -> (a*  -> (a* ))``` ，因为根据其定义，两个参数参与了加法运算，因此被确定为 num 类型。```iAmInt``` 和 ```correct``` 的类型为 ```Int*``` ， ```iAmFloat``` 和 ```alsoCorrect``` 的类型为 ```Float*``` ，```add``` 函数在其中 generalize 为了不同的类型，代码可以通过类型检查。而最后一行取消注释时，代码不能通过类型检查，因为参加运算的两个变量的类型不统一。

    除了以上运算，还有用于连接两个 ```List``` 的运算 ```++``` 。

5. 模式匹配

    我们使用 case-of 模式匹配解析一个类型，例如：

    ```
    defn sum elements = {
        case elements of {
            Single a -> {a}
            Double a b -> {a + b}
            Triple a b c -> {a + b + c}
        }
    }
    ```

6. 定义函数类型

    defn 可以定义一个拥有任意多参数的函数，定义部分是一个值。

    ```
    defn name parA parB = {
        parA + parB
    }
    ```

    例如：

    ```
    defn add a b = { a + b }
    defn increase a = { add 1 a }
    defn main = { increase 2 }
    ```

    运行这段代码会得到 ```Result = 3``` 。

    然而这并不是使用 Func 的最好方式。由于函数是第一类公民，我们可以将第二行简化为 ```defn increase = { add 1 }``` 。在类型检查阶段， ```add``` 的类型为 ```num -> (num -> (num))``` ，而 ```increase``` 为它填充了一个参数，因而类型为 ```num -> (num)``` ，且参数的类型会被限制为 ```num``` 。

    为了让我们的语言不是一个单纯的求值工具，我们还可以定义一个 do-block 函数。

    ```
    defn readInt = do {
        ...  # Too complicated
    }
    defn work = do {
        defn a <- {readInt}
        defn b <- {readInt}
        defn c <- return {a + b}
        return {c}
    }
    ```

    在 do-block 出现以前，我们的语言本质上是一个求值工具，main 函数的值是确定的。我们称这样的过程为“纯的”。而在一个 do-block 中，变量的定义是按顺序进行的，每个值都是“不纯的”。一个 do-block 函数的返回值是最后一个值。在实现上，它们由 IO IOArg 构造，其中 IOArg 为变量类型。我们也可以用 return 关键字将一个纯的值包装成不纯的（不同于C语言，它不会导致函数结束运行）。

7. 预定义函数

    1. ```read: IO*  List*  Char*``` 从控制台读入一个字符串。
    2. ```print: List*  Char*  -> (IO*  Empty* )``` 输出一个字符串到控制台。
    3. ```floatToNum: forall Num(Num) . Float*  -> (Num* )``` 强制类型转换。
    4. ```numToChar: forall Num(Num) . Num*  -> (Char* )``` 强制类型转换。
    5. ```charToNum: forall Num(Num) . Char*  -> (Num* )``` 强制类型转换。
    6. ```intToFloat: forall Num(Num) . Int*  -> (Float* )``` 强制类型转换。
    7. ```array: forall ArrayArg . List*  ArrayArg -> (Array*  ArrayArg)``` 将一个 ```List``` 包装为一个 ```Array``` 。实现上， ```Array``` 是一个占用连续内存、支持随机访问的数组，而不是链表。
    8. ```access: forall ArrayArg . Array*  ArrayArg -> (Int*  -> (ArrayArg))``` 访问 ```Array``` 的一个元素。
    9. ```size: forall ArrayArg . Array*  ArrayArg -> (Int* )``` 获得 ```Array``` 的长度。
    10. ```modify: forall ArrayArg . Array*  ArrayArg -> (Int*  -> (ArrayArg -> (IO*  Array*  ArrayArg)))``` 修改 ```Array``` 的一个元素。

8. 完成

    Func 语言由 2. 定义数据类型 和 6. 定义函数类型 组成。在实现上，源代码中应该存在一个名为 main 的函数，而 runtime.c 将尝试对它求值。
