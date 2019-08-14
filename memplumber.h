#pragma once

#include <string>
#include <new>
#include <stdint.h>
#include "memplumber-internals.h"

// Prototypes
void* operator new(std::size_t size, const char* file, int line);
void* operator new[](std::size_t size, const char* file, int line);
void operator delete(void* pointer, const char* file, int line);
void operator delete[](void* pointer, const char* file, int line);
// required for Windows compilers
void operator delete[](void* pointer);  
void operator delete(void* pointer, std::size_t size);
void operator delete[](void* pointer, std::size_t size);

// Macros
#define new new(__FILE__, __LINE__)

class MemPlumber {
    private:
        MemPlumber();

        // disable copy c'tor
        MemPlumber(const MemPlumber& other);
        
    public:

        static void start(bool verbose = false, const char* fileDumperName = "", bool append = false) {
            __start(verbose, fileDumperName, append);
        }

        static void stop() {
            __stop();
        }

        static void stopAndFreeAllMemory() {
            __stop_and_free_all_mem();
        }

        static void memLeakCheck(size_t& memLeakCount, uint64_t& memLeakSize, bool verbose = false, const char* fileDumperName = "", bool append = false) {
            __mem_leak_check(memLeakCount, memLeakSize, verbose, fileDumperName, append);
        }

        static void staticMemCheck(size_t& memCount, uint64_t& memSize, bool verbose = false, const char* fileDumperName = "", bool append = false) {
            __static_mem_check(memCount, memSize, verbose, fileDumperName, append);
        }
};

#ifdef COLLECT_STATIC_VAR_DATA

#define MEMPLUMBER_MAIN(programMain) \
    int main(int argc, char* argv[]) { \
        __program_started(); \
        return programMain(argc, argv); \
    }

#endif //COLLECT_STATIC_VAR_DATA
