#include <new>
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef MEMPLUMBER_FILENAME_LEN
#define MEMPLUMBER_FILENAME_LEN  100
#endif

#ifndef MEMPLUMBER_HASHTABLE_SIZE
#define MEMPLUMBER_HASHTABLE_SIZE 16384
#endif

#ifndef MEMPLUMBER_HASH
#define MEMPLUMBER_HASH(p) (((unsigned long)(p) >> 8) % MEMPLUMBER_HASHTABLE_SIZE)
#endif

class MemPlumberInternal {
    private:
    
    struct new_ptr_list_t {
        new_ptr_list_t* next;
        char file[MEMPLUMBER_FILENAME_LEN];
        int line;
        size_t size;
    };

    new_ptr_list_t* m_PointerListHashtable[MEMPLUMBER_HASHTABLE_SIZE];
    new_ptr_list_t* m_StaticPointerListHashtable[MEMPLUMBER_HASHTABLE_SIZE];

    bool m_Started;
    int m_ProgramStarted;
    bool m_Verbose;

    // private c'tor
    MemPlumberInternal() {
        m_Started = false;
        m_Verbose = false;

        #ifdef COLLECT_STATIC_VAR_DATA
        m_ProgramStarted = 0;
        #else
        m_ProgramStarted = -1;
        #endif //COLLECT_STATIC_VAR_DATA
    }

    public:

    static MemPlumberInternal& getInstance() {
        static MemPlumberInternal instance;
        return instance;
    }

    void* allocateMemory(std::size_t size, const char* file, int line) {

        // if not started, allocate memory and exit
        if (m_ProgramStarted != 0 && !m_Started) {
            if (m_Verbose) {
                printf("Request for memory allocation before program started\n");
            }
            return malloc(size);
        }

        // total memory to allocated is the requested size + metadata size
        size_t totalSizeToAllocate = size + sizeof(new_ptr_list_t);

        // allocated memory
        new_ptr_list_t* pointerMetaDataRecord = (new_ptr_list_t*)malloc(totalSizeToAllocate);
        memset(pointerMetaDataRecord, 0, sizeof(new_ptr_list_t));

        // if cannot allocate, return NULL
        if (pointerMetaDataRecord == NULL)
            return pointerMetaDataRecord;
        
        // calculate the actual pointer to provide to the user
        void* actualPointer = (char*)pointerMetaDataRecord + sizeof(new_ptr_list_t);

        // find the hash index for this pointer
        size_t hashIndex = MEMPLUMBER_HASH(actualPointer);

        new_ptr_list_t** hashtable = (m_ProgramStarted == 0 ? m_StaticPointerListHashtable : m_PointerListHashtable);

        // chain this metadata to the linked list of the specific bucket 
        pointerMetaDataRecord->next = hashtable[hashIndex];

        // fill in the metadata
        pointerMetaDataRecord->line = line;
        pointerMetaDataRecord->size = size;
        strncpy(pointerMetaDataRecord->file, file, MEMPLUMBER_FILENAME_LEN - 1);
		pointerMetaDataRecord->file[MEMPLUMBER_FILENAME_LEN - 1] = '\0';

        // put this metadata in the head of the list
        hashtable[hashIndex] = pointerMetaDataRecord;

        if (m_Verbose) {
            if (m_ProgramStarted == 0) {
                printf("Allocate static variable: %d[bytes] in 0x%p in %s:%d\n", (int)size, actualPointer, file, line);
            } else {
                printf("Allocate: %d[bytes] in 0x%p in %s:%d\n", (int)size, actualPointer, file, line);
            }
        }

        return actualPointer;
    }

    void freeMemory(void* pointer, const char* file, int line) {

        if (pointer == NULL) {
            return;
        }

        // find the metadata record bucket in the hash table
        size_t hashIndex = MEMPLUMBER_HASH(pointer);
        new_ptr_list_t* metaDataBucketLinkedListElement = m_PointerListHashtable[hashIndex];
	    new_ptr_list_t* metaDataBucketLinkedListPrevElement = NULL;

        // inside the bucket, go over the linked list until you find the specific pointer
        while (metaDataBucketLinkedListElement != NULL) {

            // get the actual pointer from the record
            void* actualPointerInRecord = (char*)metaDataBucketLinkedListElement + sizeof(new_ptr_list_t);

            // if this is not the pointer we're looking for - continue the search
            if (actualPointerInRecord != pointer) {
                metaDataBucketLinkedListPrevElement = metaDataBucketLinkedListElement;
                metaDataBucketLinkedListElement = metaDataBucketLinkedListElement->next;
                continue;
            }
            else { // this is the pointer we're looking for

                // remove the current element from the linked list
                if (metaDataBucketLinkedListPrevElement == NULL) { // this is the first item in the list
                    m_PointerListHashtable[hashIndex] = metaDataBucketLinkedListElement->next;
                }
                else { // this is not the first item in the list
                    metaDataBucketLinkedListPrevElement->next = metaDataBucketLinkedListElement->next;
                }

                if (m_Verbose) {
                    printf("Free: 0x%p (size %d[bytes]) allocated in: %s:%d\n", 
                        pointer,
                        (int)metaDataBucketLinkedListElement->size,
                        metaDataBucketLinkedListElement->file, 
                        metaDataBucketLinkedListElement->line);
                }

                free(metaDataBucketLinkedListElement);

                return;
            }
        }

        // if got to here it means memory was allocated before monitoring started. Simply free the memory and return 
        if (m_Verbose) {
            printf("Pointer 0x%p wasn't found\n", pointer);
        }

        free(pointer);
    }

    void programStarted() {
        m_ProgramStarted = 1;
    }

    void start(bool verbose) {
        m_Started = true;
        m_Verbose = verbose;
    }

    void stop() {
        m_Started = false;
    }

    void checkLeaks(size_t& memLeakCount, uint64_t& memLeakSize, bool verbose) {

        memLeakCount = 0;
        memLeakSize = 0;

        // go over all buckets in the hashmap
        for (int index = 0; index < MEMPLUMBER_HASHTABLE_SIZE; ++index) {
            new_ptr_list_t* metaDataBucketLinkedListElement = m_PointerListHashtable[index];

            // if bucket is empty - continue
            if (metaDataBucketLinkedListElement == NULL) {
                continue;
            }

            // go over all of the elements in the link list in this bucket
            while (metaDataBucketLinkedListElement != NULL) {

                memLeakCount++;
                memLeakSize += (uint64_t)metaDataBucketLinkedListElement->size;

                if (verbose) {
                    printf("Found leaked object at 0x%p (size %d[bytes]) allocated in: %s:%d\n",
                        (char*) metaDataBucketLinkedListElement + sizeof(new_ptr_list_t), 
                        (int) metaDataBucketLinkedListElement->size,
                        metaDataBucketLinkedListElement->file, 
                        metaDataBucketLinkedListElement->line);
                }

                metaDataBucketLinkedListElement = metaDataBucketLinkedListElement->next;
            }
        }
    }

    void staticMemAllocation(size_t&  memCount, uint64_t& memSize, bool verbose) {
        memCount = 0;
        memSize = 0;

        for (int index = 0; index < MEMPLUMBER_HASHTABLE_SIZE; ++index) {
            new_ptr_list_t* metaDataBucketLinkedListElement = m_StaticPointerListHashtable[index];

            // if bucket is empty - continue
            if (metaDataBucketLinkedListElement == NULL) {
                continue;
            }

            // go over all of the elements in the link list in this bucket
            while (metaDataBucketLinkedListElement != NULL) {
                memCount++;
                memSize += (uint64_t)metaDataBucketLinkedListElement->size;
                metaDataBucketLinkedListElement = metaDataBucketLinkedListElement->next;
            }
        }
    }

    void freeAllMemory() {
        for (int index = 0; index < MEMPLUMBER_HASHTABLE_SIZE; ++index) {
            new_ptr_list_t* metaDataBucketLinkedListElement = m_PointerListHashtable[index];

            // if bucket is empty - continue
            if (metaDataBucketLinkedListElement == NULL) {
                continue;
            }

            // go over all of the elements in the link list in this bucket
            while (metaDataBucketLinkedListElement != NULL) {
                new_ptr_list_t* next = metaDataBucketLinkedListElement->next;

                void* actualPointerInRecord = (char*)metaDataBucketLinkedListElement + sizeof(new_ptr_list_t);

                if (m_Verbose) {
                    printf("FreeAllMem: freeing 0x%p (size %d[bytes]) allocated in %s:%d\n", 
                        actualPointerInRecord,
                        (int)metaDataBucketLinkedListElement->size,
                        metaDataBucketLinkedListElement->file,
                        metaDataBucketLinkedListElement->line);
                }

                free(metaDataBucketLinkedListElement);
                metaDataBucketLinkedListElement = next;
            }
        }
    }
};


#ifdef _MSC_VER
char* getCaller() {
    //TODO
    return "Unknown";
}
#else
#include <execinfo.h>
char* getCaller() {
    void* backtraceArr[3];
    size_t backtraceArrSize;

    // get void*'s for all entries on the stack
    backtraceArrSize = backtrace(backtraceArr, 3);

    if (backtraceArrSize < 3) {
        return "Unknown";
    }

    // get the symbols
    char** backtraceSymbols = backtrace_symbols(backtraceArr, backtraceArrSize);

    // the caller is second in the backtrace
    return backtraceSymbols[2];
}
#endif

void* operator new(std::size_t size, const char* file, int line) {
    return MemPlumberInternal::getInstance().allocateMemory(size, file, line);
}

void* operator new[](std::size_t size, const char* file, int line) {
    return operator new(size, file, line);
}

void* operator new[](size_t size) {
	return operator new(size, getCaller(), 0);
}

void* operator new(size_t size) {
	return operator new(size, getCaller(), 0);
}

void* operator new(size_t size, const std::nothrow_t&) throw () {
	return operator new(size, getCaller(), 0);
}

void* operator new[](size_t size, const std::nothrow_t&) throw () {
	return operator new[](size, getCaller(), 0);
}

void operator delete(void* pointer, const char* file, int line) {
    MemPlumberInternal::getInstance().freeMemory(pointer, file, line);
}

void operator delete(void* pointer) throw() {
    operator delete(pointer, __FILE__, __LINE__);
}

void operator delete[](void* pointer, const char* file, int line) {
    operator delete(pointer, file, line);
}

void operator delete(void* pointer, const std::nothrow_t&) throw() {
	operator delete(pointer);
}

void operator delete[](void* pointer, const std::nothrow_t&) throw() {
	operator delete(pointer, std::nothrow);
}

void __mem_leak_check(size_t& memLeakCount, uint64_t& memLeakSize, bool verbose) {
    MemPlumberInternal::getInstance().checkLeaks(memLeakCount, memLeakSize, verbose);
}

void __static_mem_check(size_t&  memCount, uint64_t& memSize, bool verbose) {
    MemPlumberInternal::getInstance().staticMemAllocation(memCount, memSize, verbose);
}

void __start(bool verbose) {
    MemPlumberInternal::getInstance().start(verbose);
}

void __stop() {
    MemPlumberInternal::getInstance().stop();
}

void __stop_and_free_all_mem() {
    MemPlumberInternal::getInstance().stop();
    MemPlumberInternal::getInstance().freeAllMemory();
}

void __program_started() {
    MemPlumberInternal::getInstance().programStarted();
}
