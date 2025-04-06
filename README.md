



## Todos

* https://en.wikipedia.org/wiki/Flexible_array_member



## About

following book [crafting interpreters](https://craftinginterpreters.com/contents.html)

## before coding
* read build
* read tips
* read c23 feature

## build

```
make configure
make 
make run
```

## visualize vm execution

say we have 
```
var a = 3;
print a + 4
```

```
// after compilation, it turns into
// instructions
[OP_GLOBAL_DEFINE #1 OP_LOAD_GLOBAL a OP_LOAD #2 OP_ADD OP_PRINT OP_RETURN]
// constants array, store string, number...etc
[3, 4]

// when vm executing, we have a gloabl varialbe table with entries
[a->#1]

// an execution memory stack
[3]
[3, 4]
[7]
// print 7
[]
// done. exiting.

```

## debug

### with lldb
* debug with lldb - https://lldb.llvm.org/use/tutorial.html

### with gdb
-> gdb quick ref
```
break main

# view sources
(gdb) list 

print *scanner.current

# continue
(gdb) c

# step
(gdb) n

# step in
(gdb) s

```

-> debug segment fault

```
# 
gdb ./build/bin/Cloxd
                

(gdb) run < input-files

(gdb) list
(gdb) where
```

example:
```
git checkout <tag>
make run

# inside clox, you'll see segment fault error 
> "a" + "b"

# then u can debug with gdb
```

-> read from user input
```
# option 1
(gdb) run < inputfile.txt

# option 2, view your tty by type `tty` command
(gdb) tty /dev/ttyb
```

## src files

```
scanner          // read source by token
compiler         // construct ast nodes using tokens, handling operator precedence
vm               // the actual vm implementation, simply run instructions one by one
```


## implementation notes

* handle operator precedence is an important part of any parser. the book use `pratt-parser` to handle this part. as described in here - https://craftinginterpreters.com/compiling-expressions.html#a-pratt-parser


## Tips

```
# view function doc
man [3] realloc

# in vim, same with `K` key in normal mode
:Man realloc
```

* Lsp configure clangd - https://clangd.llvm.org/installation.html
* clang-format
    * manual - https://clang.llvm.org/docs/ClangFormatStyleOptions.html
    * format file in vim
        * select all in visual mode
        * `:!clang-format`



# Notes

## c language, clang

all c related notes in code is marked with `// c:`

-> c new standard features
    * c23 - 
        * https://lemire.me/blog/2024/01/21/c23-a-slightly-better-c/
        * https://en.cppreference.com/w/c/23


-> ! `realloc` vs `malloc`
    * realloc will copy original content to new allocated heap. prefer to use this over `malloc`
    * 

-> pointer unit
the unit for pointer is byte. as u can see from this link https://www.tutorialspoint.com/cprogramming/c_pointer_arithmetic.htm

-> printf - https://cplusplus.com/reference/cstdio/printf/
%[flags][width][.precision][length]specifier


-> when to put things in clang's header files
* it's like a public exports. when u wanna other's to use/see this symbol


-> clang conversion from Array to Pointer

```
int* arr = {1, 2, 4}

arr[0] // type is int
&arr[0] // type is int*

```

-> clang function pointer
```
typedef void (*ParseFn)(bool canAssign);
//                           ^ clang require varialbe name
```

-> others
- in c, function order matters. if A depends on B, then B must declared before A

# chapter 14

## memory allocator
    
link:
* https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-3.html


### notes
* archieve/allocator.c
    * c macro, string concatenation
    * c `__va_args__` marco: 
        * https://www.tutorialspoint.com/c_standard_library/c_macro_va_arg.htm
        * https://stackoverflow.com/questions/26053959/what-does-va-args-in-a-macro-mean
    * c LOG macro
    * 
    