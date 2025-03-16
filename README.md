



## Todos



## About

following book [crafting interpreters](https://craftinginterpreters.com/contents.html)


## build

```
make configure
make 
make run
```


## debug

### with lldb
* debug with lldb - https://lldb.llvm.org/use/tutorial.html

### with gdb
-> gdb quick ref
```
break main

# view sources
list 

print *scanner.current

# continue
c

# step
n

# step in
s

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
```

* Lsp configure clangd - https://clangd.llvm.org/installation.html
* clang-format
    * manual - https://clang.llvm.org/docs/ClangFormatStyleOptions.html
    * format file in vim
        * select all in visual mode
        * `:!clang-format`



# Notes

## c language

all c related notes in code is marked with `// c:`

-> pointer unit
the unit for pointer is byte. as u can see from this link https://www.tutorialspoint.com/cprogramming/c_pointer_arithmetic.htm

-> printf - https://cplusplus.com/reference/cstdio/printf/
%[flags][width][.precision][length]specifier


-> when to put things in clang's header files
* it's like a public exports. when u wanna other's to use/see this symbol

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
    