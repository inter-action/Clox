## Todos

* debug with lldb - https://lldb.llvm.org/use/tutorial.html


## About

following book [crafting interpreters](https://craftinginterpreters.com/contents.html)


## build

```
make configure
make 
make run
```



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
    