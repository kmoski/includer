# C++ include optimizer

This util convert relative path relative to source.

Prerequisites:

[include MS](https://docs.microsoft.com/en-us/cpp/preprocessor/hash-include-directive-c-cpp?view=msvc-160)  
I think other CCs work in a similar way.
---

Example:

Repo contain 3 projects:  
- lib1  
- lib2  
- my_proj

*my_repo depends on lib1 and lib2

1. File struct:
   ```
   -+ repo
    ++ lib1
    |++ func1
    ||+- func1.h
    ||+- func1.cpp
    ||
    |++ parser
    ||+- parser.h
    ||+- parser.cpp
    |+- main1.h
    |+- main1.cpp
    |
    ++ lib2
    |++ func2
    ||+- func2.h
    ||+- func2.cpp
    |+- main2.h
    |+- main2.cpp
    |
    ++ my_proj
     ++ func3
     |+- func3.h
     |+- func3.cpp
     +- my_main.h
     +- my_main.cpp
   ```
1. Includes:
   main1.h
   ```c++
   #include "func1.h"
   ...
   ```
   func1.h
   ```c++
   #include "parser/parser.h"
   ...
   ```
   main2.h
   ```c++
   #include "func2.h"
   ...
   ```
   my_main.h
   ```c++
   #include <main1.h>
   #include <main2.h>
   #include "func3.h"
   ...
   ```
   Project include dirs:
   - lib1 - `func1 parser`  
   - lib2 - `func2`  
   - my_proj - `func3 ../ ../lib1/func1 ../lib1/parser ../lib2/func2`
   
   my_proj is forced to contain parasitic paths from lib1 and lib2.
    
* Output util:
   main1.h
   ```c++
   #include "func1/func1.h"
   ...
   ```
   func1.h
   ```c++
   #include "../parser/parser.h"
   ...
   ```
   main2.h
   ```c++
   #include "func2/func2.h"
   ...
   ```
   my_main.h
   ```c++
   #include <lib1/main1.h>
   #include <lib2/main2.h>
   #include "func3/func3.h"
   ...
   ```
   Project include dirs:
   - lib1 - nope  
   - lib2 - nope  
   - my_proj - `../` or `PATH_TO_LIB1_PARENT_DIR`&`PATH_TO_LIB2_PARENT_DIR`
