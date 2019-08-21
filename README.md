MemPlumber
==========

[![Build Status](https://travis-ci.org/seladb/MemPlumber.svg?branch=master)](https://travis-ci.org/seladb/MemPlumber)
[![Build status](https://ci.appveyor.com/api/projects/status/aw1jwoqa0sb2no45?svg=true)](https://ci.appveyor.com/project/seladb/memplumber)

MemPlumber is a C++ library that aims to help developers with debugging of memory allocations and detection of memory leaks in C++ applications. It is based on the [Debug_new](https://en.wikipedia.org/wiki/Debug_new) technique and provides a clean and easy-to-use interface.

It is multi platform and supported on Windows (MinGW and Visual Studio), Linux and MacOS.

It is different than tools like [Valgrind](http://www.valgrind.org/) in that it's not an external tool, but rather a library you link your code with. Once turned on from inside your code it will track all memory allocations and will help detecting memory leaks.

This library is very useful for testing and debugging, and in particular in unit-tests. Just link it to your test code and initiate a mem leak test. Once memory leaks are found, the library provides easy to use tools for debugging and locating the exact origin of the memory leak.

Please note it is not recommended to use this library in production since tracking memory allocations has a cost in both performance and memory.

## Getting Started

Consider this piece of code:

```cpp
#include <stdio.h>

class MyClass {
    int x;
};

int main( int argc, const char* argv[]) {

    // init 2 objects
    int* num = new int(100);
    MyClass* myClass = new MyClass();

    // init 1 array of 10 objects
    MyClass* myClassArr = new MyClass[10];

    return 0;
}
```

You probably spotted the two leaked objects (one integer and one instance of `MyClass`) and the leaked array of 10 `MyClass` Objects. Let's add `MemPlumber` leak test now:

```cpp
#include "memplumber.h"
#include <stdio.h>

class MyClass {
    int x;
};

int main( int argc, const char* argv[]) {

    // start collecting mem allocations info
    MemPlumber::start();

    // init 2 objects
    int* num = new int(100);
    MyClass* myClass = new MyClass();

    // init 1 array of 10 objects
    MyClass* myClassArr = new MyClass[10];

    // run memory leak test
    size_t memLeakCount; 
    uint64_t memLeakSize;
    MemPlumber::memLeakCheck(memLeakCount, memLeakSize, true);

    // print memory leak results
    printf("Number of leaked objects: %d\nTotal number of memory leaked: %d[bytes]\n", (int)memLeakCount, (int)memLeakSize);

    return 0;
}
```

When running this code we get the following output:

```
Found leaked object at 0x001A6E90 (size 4[bytes]) allocated in: C:\MemPlumber\Examples\example1.cpp:14
Found leaked object at 0x001A6F10 (size 4[bytes]) allocated in: C:\MemPlumber\Examples\example1.cpp:15
Found leaked object at 0x001A7020 (size 40[bytes]) allocated in: C:\MemPlumber\Examples\example1.cpp:18
Number of leaked objects: 3
Total number of memory leaked: 48[bytes]
```

Please note that we ran the mem leak test in verbose mode, so in addition to the number of leaked objects and total memory leaked, we also got information about where each mem leak happened.
