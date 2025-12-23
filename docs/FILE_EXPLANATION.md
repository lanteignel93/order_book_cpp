# `compile_commands.json` 

## What each field means
### `directory`

```json
"directory": "/home/laurent/git_projects/order_book_cpp/build"
```

This is the working directory where the compile command is run. Relative paths inside "command" are interpreted from here.

### `command`

Example:
```json
"/usr/bin/c++  -I/home/laurent/git_projects/order_book_cpp/include -g -std=c++20 -o CMakeFiles/orderbook_lib.dir/src/OrderBook.cpp.o -c /home/laurent/git_projects/order_book_cpp/src/OrderBook.cpp"
```


This is the full compiler command line. Key parts:

* `/usr/bin/c++`
The compiler driver. On Linux, this is often GCC’s g++ or Clang’s clang++ depending on system config.

* `-I.../include`
Adds an include search path. This is why #include "orderbook/OrderBook.h" works: the compiler searches inside /.../include.

* `-g`
Include debug symbols (useful for debugging with gdb/lldb).

* `-std=c++20`
Compile as C++20.

* `-c <file>`
Compile only (do not link). Produces an object file (.o).

* `-o <output.o>`
Where the object file will be written.

So this entry is: “compile src/OrderBook.cpp into an object file for the orderbook_lib target.”

### `file`

Example:
```json
"file": "/home/laurent/git_projects/order_book_cpp/src/OrderBook.cpp"
```


This is the source file the entry corresponds to.

### `output`

Example:
```json
"output": "CMakeFiles/orderbook_lib.dir/src/OrderBook.cpp.o"
```

Where the object file lands (relative to "directory").

## What your specific file tells us about your project

1. You have two translation units:
    * `src/OrderBook.cpp (the library)`
    * `app/main.cpp (the executable)`

2. Both compile with:
    * `-std=c++20`
    * `-g`
    * `-I.../include`

3. The build is structured as:
    * `orderbook_lib` compiled from `OrderBook.cpp`
    * `orderbook_app` compiled from `main.cpp` and linked against the library (that link step isn’t shown here—compile_commands captures compilation, not necessarily the final link command unless configured).

## Why you have a symlink at repo root

You showed earlier:
* `compile_commands.json -> build/compile_commands.json`

That is a common convenience pattern so clangd/IDEs can find the file at the project root, even though CMake generates it in build/.

## What this file is not

* It is not used at runtime.
* It is not required to build (CMake/Ninja/Make already know how).
* It is mainly for editor tooling and static analysis.

------------------------------------------------------------
# CMake

CMake is a build system generator. It does not compile code itself.
Instead, it describes how your project is built and generates native
build files (Makefiles, Ninja, etc.). It is the single source of truth
for how your C++ project is compiled and linked.

In this project, CMake:
- Defines the project structure
- Selects the C++ standard
- Builds a library and an executable
- Generates compile_commands.json for tooling


## Project and Language

The project is declared as a C++ project:
```cmake
project(orderbook LANGUAGES CXX)
```

This enables C++ support and names the project.

## C++ Standard

The project enforces C++20:
```cmake
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
```

This guarantees:
- All code is compiled as C++20
- The build fails if C++20 is unavailable
- No compiler-specific extensions are used

## Targets (Most Important Concept)

Modern CMake revolves around targets.

This project defines two targets:

A) Library target (core engine)
```cmake
  add_library(orderbook_lib
    src/OrderBook.cpp
  )
```

- Builds a static library (liborderbook_lib.a)
- Contains the order book and matching logic
- Can be reused by apps and tests

B) Executable target (application)
```cmake
  add_executable(orderbook_app
    app/main.cpp
  )
```

- Builds the runnable program
- Acts as a thin driver around the library

## Include Directories

Public headers live in the include/ directory.
```cmake
  target_include_directories(orderbook_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
```

This:
- Adds include/ to the compiler include path (-I)
- Exports the include path to anything linking the library

This enables code like:
```cpp
  #include "orderbook/OrderBook.h"
```

## Linking Targets

The executable links against the library:
```cmake
  target_link_libraries(orderbook_app PRIVATE orderbook_lib)
```

This means:
- orderbook_app depends on orderbook_lib
- The app inherits include paths and compile settings
- PRIVATE indicates the dependency is not re-exported


## How Everything Fits Together

  src/OrderBook.cpp
          ↓
     orderbook_lib
          ↓
     orderbook_app
          ↑
  include/orderbook/*.h

CMake generates:
- Object files (.o)
- A static library (liborderbook_lib.a)
- An executable (orderbook_app)
- compile_commands.json for IDEs and tooling


## What This Setup Intentionally Does NOT Include
- No test framework
- No install rules
- No packaging
- No dependency management

This keeps the project simple, clear, and easy to extend.

------------------------------------------------------------


