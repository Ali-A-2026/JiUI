/**
 * JiUI - Memory Management Unit Tests
 */

#include <jiui/jiui.h>
#include <stdio.h>
#include <string.h>

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define ASSERT(expr, msg) do { \
    g_tests_run++; \
    if (expr) { g_tests_passed++; } \
    else { g_tests_failed++; fprintf(stderr, "FAIL: %s (line %d): %s\n", __func__, __LINE__, msg); } \
} while(0)

/* ---- Allocation ---- */
static void test_alloc_free(void) {
    void* p = ji_alloc(128);
    ASSERT(p != NULL, "alloc 128 bytes");
    memset(p, 0xAB, 128);
    ji_free(p);
}

static void test_calloc_zeroed(void) {
    int* arr = (int*)ji_calloc(10, sizeof(int));
    ASSERT(arr != NULL, "calloc 10 ints");
    for (int i = 0; i < 10; i++) {
        ASSERT(arr[i] == 0, "calloc zeroed");
    }
    ji_free(arr);
}

static void test_realloc(void) {
    void* p = ji_alloc(64);
    ASSERT(p != NULL, "alloc 64 bytes");
    memset(p, 0xAA, 64);
    void* p2 = ji_realloc(p, 256);
    ASSERT(p2 != NULL, "realloc to 256");
    ji_free(p2);
}

static void test_new_macro(void) {
    typedef struct { int x; int y; } TestStruct;
    TestStruct* s = JI_NEW(TestStruct);
    ASSERT(s != NULL, "JI_NEW alloc");
    ASSERT(s->x == 0 && s->y == 0, "JI_NEW zeroed");
    JI_FREE(s);
}

/* ---- Reference counting ---- */
static void test_ref_count_basic(void) {
    JiRefCount rc;
    ji_ref_init(&rc);
    ASSERT(ji_ref_count(&rc) == 1, "initial ref count = 1");

    ji_ref_acquire(&rc);
    ASSERT(ji_ref_count(&rc) == 2, "after acquire = 2");

    ji_ref_release(&rc);
    ASSERT(ji_ref_count(&rc) == 1, "after release = 1");
}

/* ---- Ref-counted object ---- */
static int g_destroy_count = 0;

static void test_destroy_func(void* obj) {
    g_destroy_count++;
    ji_free(obj);
}

static void test_ref_object(void) {
    g_destroy_count = 0;
    JiRefObject* obj = JI_NEW(JiRefObject);
    ASSERT(obj != NULL, "ref object alloc");
    ji_ref_object_init(obj, test_destroy_func);
    ASSERT(ji_ref_object_count(obj) == 1, "initial ref = 1");

    ji_ref_object_acquire(obj);
    ASSERT(ji_ref_object_count(obj) == 2, "after acquire = 2");

    ji_ref_object_release(obj);
    ASSERT(ji_ref_object_count(obj) == 1, "after release = 1");

    ji_ref_object_release(obj);
    /* obj is now destroyed, g_destroy_count should be 1 */
    ASSERT(g_destroy_count == 1, "destroy called on last release");
}

/* ---- Memory pool ---- */
static void test_pool_basic(void) {
    JiMemoryPool* pool = ji_pool_create(1024);
    ASSERT(pool != NULL, "pool create");

    void* p1 = ji_pool_alloc(pool, 100);
    ASSERT(p1 != NULL, "pool alloc 100");

    void* p2 = ji_pool_alloc(pool, 200);
    ASSERT(p2 != NULL, "pool alloc 200");

    ji_pool_reset(pool);

    void* p3 = ji_pool_alloc(pool, 50);
    ASSERT(p3 != NULL, "pool alloc after reset");

    ji_pool_destroy(pool);
}

int main(void) {
    ji_initialize();

    test_alloc_free();
    test_calloc_zeroed();
    test_realloc();
    test_new_macro();
    test_ref_count_basic();
    test_ref_object();
    test_pool_basic();

    ji_shutdown();

    printf("\n=== Memory Test Results ===\n");
    printf("Total: %d  Passed: %d  Failed: %d\n",
           g_tests_run, g_tests_passed, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
