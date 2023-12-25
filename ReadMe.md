# Test LLVM

Clone the project and go into project folder:

```bash
git clone https://github.com/hkinke/testllvm.git && cd testllvm
```

Configure and Build the project

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug .
cmake --build build
```

Debug the project:

```bash

gdb ./build/main
```

Set a break point:

```gdb
(gdb) b 114
```

Run the project

```gdb
(gdb) run
```

Continue

```gdb
(gdb) continue
```

Exit gdb

```gdb
(gdb) quit
```

The file `KaleidoscopeJIT.h` has been modified to support `malloc` and `free` of `LLVM` linked against `ucrtbase.dll`. To test old behavior, please download the original file [KaleidoscopeJIT.h](https://raw.githubusercontent.com/llvm/llvm-project/main/llvm/examples/Kaleidoscope/include/KaleidoscopeJIT.h) or use the previous commit.

