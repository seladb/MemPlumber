#include "memplumber.h"
#include <stdio.h>
#include "test-macros.h"
#include "basic-tests.cpp"
#include "static-var-tests.cpp"

int tests_main(int argc, char* argv[]) {

    START_RUNNING_TESTS;
    
    //RUN_TEST(BasicTest);
    //RUN_TEST(MultipleAllocations);
    //RUN_TEST(ArrayAllocation);
    
    RUN_TEST(StaticVarTest);

    END_RUNNING_TESTS;
}

MEMPLUMBER_MAIN(tests_main);
