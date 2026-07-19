/**
 * JiUI - Error Handling Unit Tests
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

/* ---- Result codes ---- */
static void test_result_codes(void) {
    ASSERT(JI_SUCCEEDED(JI_OK), "JI_OK is success");
    ASSERT(JI_FAILED(JI_ERROR_UNKNOWN), "ERROR_UNKNOWN is failure");
    ASSERT(JI_FAILED(JI_ERROR_OUT_OF_MEMORY), "OOM is failure");
    ASSERT(JI_FAILED(JI_ERROR_PARSE), "PARSE is failure");

    const char* str = ji_result_to_string(JI_OK);
    ASSERT(str != NULL && strcmp(str, "OK") == 0, "result string for OK");

    str = ji_result_to_string(JI_ERROR_OUT_OF_MEMORY);
    ASSERT(str != NULL && strcmp(str, "Out of memory") == 0, "result string for OOM");
}

/* ---- JiError ---- */
static void test_error_create(void) {
    JiError* err = ji_error_create(JI_ERROR_PARSE, "test error %d", 42);
    ASSERT(err != NULL, "error create");
    ASSERT(err->code == JI_ERROR_PARSE, "error code");
    ASSERT(err->message != NULL, "error message not null");
    ASSERT(strcmp(err->message, "test error 42") == 0, "error message content");
    ji_error_destroy(err);
}

static void test_error_chain(void) {
    JiError* cause = ji_error_create(JI_ERROR_IO, "file not found");
    JiError* err = ji_error_create(JI_ERROR_PARSE, "parse failed");
    err->cause = cause;

    ASSERT(err->cause != NULL, "chained error");
    ASSERT(err->cause->code == JI_ERROR_IO, "chain code");
    ASSERT(strcmp(err->cause->message, "file not found") == 0, "chain message");

    ji_error_destroy(err); /* should destroy chain too */
}

/* ---- JiResult ---- */
static void test_result_ok(void) {
    JiResult r = ji_result_ok();
    ASSERT(ji_result_is_ok(&r), "ok result is ok");
    ASSERT(!ji_result_is_err(&r), "ok result is not err");
    ji_result_destroy(&r);
}

static void test_result_fail(void) {
    JiError* err = ji_error_create(JI_ERROR_STATE, "bad state");
    JiResult r = ji_result_fail(err);
    ASSERT(ji_result_is_err(&r), "fail result is err");
    ASSERT(!ji_result_is_ok(&r), "fail result is not ok");
    ASSERT(r.code == JI_ERROR_STATE, "fail code");
    ji_result_destroy(&r);
}

static void test_result_fail_code(void) {
    JiResult r = ji_result_fail_code(JI_ERROR_NULL_PTR);
    ASSERT(ji_result_is_err(&r), "fail_code is err");
    ASSERT(r.code == JI_ERROR_NULL_PTR, "fail_code code");
    ASSERT(r.error == NULL, "fail_code no error object");
    ji_result_destroy(&r);
}

static void test_result_ptr(void) {
    int value = 42;
    JiResult r = ji_result_ok_ptr(&value);
    ASSERT(ji_result_is_ok(&r), "ok_ptr is ok");
    ASSERT(r.value.ptr == &value, "ok_ptr value");
    ji_result_destroy(&r);
}

/* ---- Logging ---- */
static JiLogLevel g_last_level = JI_LOG_TRACE;
static char g_last_message[256] = {0};

static void test_log_callback(JiLogLevel level, const char* message, void* user_data) {
    g_last_level = level;
    (void)user_data;
    snprintf(g_last_message, sizeof(g_last_message), "%s", message);
}

static void test_logging(void) {
    ji_log_set_callback(test_log_callback, NULL);
    ji_log_set_level(JI_LOG_TRACE);

    JI_INFO("test %d", 123);
    ASSERT(g_last_level == JI_LOG_INFO, "log level");
    ASSERT(strcmp(g_last_message, "test 123") == 0, "log message");

    /* Reset to default */
    ji_log_set_callback(NULL, NULL);
}

int main(void) {
    ji_initialize();

    test_result_codes();
    test_error_create();
    test_error_chain();
    test_result_ok();
    test_result_fail();
    test_result_fail_code();
    test_result_ptr();
    test_logging();

    ji_shutdown();

    printf("\n=== Error Test Results ===\n");
    printf("Total: %d  Passed: %d  Failed: %d\n",
           g_tests_run, g_tests_passed, g_tests_failed);

    return g_tests_failed > 0 ? 1 : 0;
}
