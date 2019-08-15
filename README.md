MemPlumber
==========

[![Build Status](https://travis-ci.org/seladb/MemPlumber.svg?branch=master)](https://travis-ci.org/seladb/MemPlumber)
[![Build status](https://ci.appveyor.com/api/projects/status/aw1jwoqa0sb2no45?svg=true)](https://ci.appveyor.com/project/seladb/memplumber)

MemPlumber is a C++ library that aims to help developers with debugging of memory allocations and detection of memory leaks in C++ applications. It is based on the [Debug_new](https://en.wikipedia.org/wiki/Debug_new) technique and provides a clean and easy-to-use interface.

It is multi platform and supported on Windows (MinGW and Visual Studio), Linux and MacOS.

It is different than tools like [Valgrind](http://www.valgrind.org/) in that it's not an external tool, but rather a library you link with your code. Once turned on from inside the code it will track all memory allocations and will help detecting memory leaks.

This library is very useful for testing and debugging, for example in unit-tests. Just link it to your test code and add a test that checks for memory leaks. Once memory leaks are found, the library provides easy to use tools for debugging and locating the exact origin of the memory leak.

Please note it is not recommended to use this library in production since tracking memory allocations has a cost in both performance and memory.
