#include "memplumber.h"
#include "test-macros.h"
#include <string>
#include <stdio.h>

class TestClass1 {
    int x;
};

class TestClass2 {
    private:

    std::string m_Str;
    int m_Num;
    double* m_DoublePtr;

    public:

    TestClass2() {
        m_Str = "TestClass2";

        m_Num = 1000;
        m_DoublePtr = new double(1.2345);
    }

    ~TestClass2() {
        delete m_DoublePtr;
    }
};

TEST_CASE(BasicTest) {

    START_TEST;

    TestClass1* test1 = new TestClass1();

    CHECK_MEM_LEAK(1, sizeof(TestClass1));

    TestClass2* test2 = new TestClass2();

    #if !defined _MSC_VER || !defined _DEBUG
    CHECK_MEM_LEAK(3, sizeof(TestClass1) + sizeof(TestClass2) + sizeof(double));
    #endif

    delete test1;

    #if !defined _MSC_VER || !defined _DEBUG
    CHECK_MEM_LEAK(2, sizeof(TestClass2) + sizeof(double));
    #endif

    delete test2;

    CHECK_MEM_LEAK(0, 0);

    STOP_TEST;
}

TEST_CASE(MultipleAllocations) {

    START_TEST;

    TestClass1* testArray1[10];
    TestClass2* testArray2[10];

    for (int i = 0; i < 10; i++) {
        testArray1[i] = new TestClass1();
        testArray2[i] = new TestClass2();
    }

    CHECK_MEM_LEAK(30, 10*(sizeof(TestClass1) + sizeof(TestClass2) + sizeof(double)));

    for (int i = 0; i < 10; i++) {
        delete testArray2[i];
    }

    CHECK_MEM_LEAK(10, 10*sizeof(TestClass1));

    for (int i = 0; i < 10; i++) {
        delete testArray1[i];
    }

    CHECK_MEM_LEAK(0, 0);

    STOP_TEST;
}

TEST_CASE(ArrayAllocation) {

    START_TEST;

    TestClass1* arr1 = new TestClass1[100];

    CHECK_MEM_LEAK(1, 100*sizeof(TestClass1));

    TestClass2* arr2 = new TestClass2[30];

    CHECK_MEM_LEAK(2+30, 100*sizeof(TestClass1) + 30*sizeof(TestClass2) + 30*sizeof(double) + sizeof(void*));

    delete [] arr2;

    CHECK_MEM_LEAK(1, 100*sizeof(TestClass1));

    delete [] arr1;

    CHECK_MEM_LEAK(0, 0);

    STOP_TEST;
}

#ifdef COLLECT_STATIC_VAR_DATA
#define MAIN tests_main
#else
#define MAIN main
#endif

int MAIN(int argc, char* argv[]) {

    START_RUNNING_TESTS;
    
    RUN_TEST(BasicTest);
    
    #if !defined _MSC_VER || !defined _DEBUG
    RUN_TEST(MultipleAllocations);
    RUN_TEST(ArrayAllocation);
    #else
    SKIP_TEST(MultipleAllocations, "Additional debug allocations made by VS make it difficult to track real memory allocations");
    SKIP_TEST(ArrayAllocation, "Additional debug allocations made by VS make it difficult to track real memory allocations");
    #endif

    END_RUNNING_TESTS;
}

#ifdef COLLECT_STATIC_VAR_DATA
MEMPLUMBER_MAIN(tests_main);
#endif





