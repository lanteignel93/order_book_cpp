# Debugging 

## Configure Debug Build 

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
What this does

* `-S .` → source directory (your repo root)
* `-B` build → build directory
Debug enables:
    * `-g` debug symbols
    * no optimizations (or minimal)
    * correct stepping in debugger

You only need to re-run this if:
* You change CMakeLists.txt
* You delete the build/ directory
* You switch build types

## Build Project (Debug) 
```bash
cmake --build build -j
```

**What this does
* Invokes the underlying build tool (Make or Ninja) 
* Compiles only files that changed 
* Produces 
    * `build/liborderbook_lib.a` 
    * `build/orderbook_app`

# Run the Program 

```bash
./build/orderbook_app
```

# Release build 
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
