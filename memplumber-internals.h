#pragma once

#include <stdint.h>
#include <string>

void __mem_leak_check(size_t& memLeakCount, uint64_t& memLeakSize, bool verbose);
void __static_mem_check(size_t&  memCount, uint64_t& memSize, bool verbose);
void __start(bool verbose);
void __stop();
void __stop_and_free_all_mem();
void __program_started();