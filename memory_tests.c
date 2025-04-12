#ifdef NDEBUG
#undef NDEBUG
#endif

#include "memory_tests.h"
#include <assert.h>
#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// We intercept the memory allocation and deallocation functions.
void * __real_malloc(size_t size) __attribute__((weak));
void * __real_calloc(size_t nmemb, size_t size) __attribute__((weak));
void * __real_realloc(void *ptr, size_t size) __attribute__((weak));
void * __real_reallocarray(void *ptr, size_t nmemb, size_t size) __attribute__((weak));
char * __real_strdup(const char *s) __attribute__((weak));
char * __real_strndup(const char *s, size_t size) __attribute__((weak));
void   __real_free(void *ptr) __attribute__((weak));

// We store global information about memory allocations and deallocations.
static memory_test_data_t test_data;

// Simple accessor needed for testing.
memory_test_data_t* get_memory_test_data(void) {
  return &test_data;
}

// At a given moment, memory allocation fails.
static bool should_fail(void) {
  return ++test_data.call_counter == test_data.fail_counter;
}

// Reallocation must succeed unless we are increasing the allocated memory size.
static bool can_fail(void const *old_ptr, size_t new_size) {
  if (old_ptr == NULL)
    return true;
  else
    return new_size > malloc_usable_size((void *)old_ptr);
}

// Simulate memory shortage.
#define UNRELIABLE_ALLOC(ptr, size, fun, name)                           \
  do {                                                                   \
    test_data.call_total++;                                              \
    if (ptr != NULL && size == 0) {                                      \
      /* This realloc call is equivalent to a free(ptr) call. */ \
      test_data.free_counter++;                                          \
      return fun;                                                        \
    }                                                                    \
    void *p = can_fail(ptr, size) && should_fail() ? NULL : (fun);       \
    if (p) {                                                             \
      test_data.alloc_counter += ptr != p;                               \
      test_data.free_counter += ptr != p && ptr != NULL;                 \
    }                                                                    \
    else {                                                               \
      errno = ENOMEM;                                                    \
      test_data.function_name = name;                                    \
    }                                                                    \
    return p;                                                            \
  } while (0)

void *__wrap_malloc(size_t size) {
  UNRELIABLE_ALLOC(NULL, size, __real_malloc(size), "malloc");
}

void *__wrap_calloc(size_t nmemb, size_t size) {
  UNRELIABLE_ALLOC(NULL, nmemb * size, __real_calloc(nmemb, size), "calloc");
}

void *__wrap_realloc(void *ptr, size_t size) {
  UNRELIABLE_ALLOC(ptr, size, __real_realloc(ptr, size), "realloc");
}

void *__wrap_reallocarray(void *ptr, size_t nmemb, size_t size) {
  UNRELIABLE_ALLOC(ptr, nmemb * size, __real_reallocarray(ptr, nmemb, size), "reallocarray");
}

char *__wrap_strdup(const char *s) {
  UNRELIABLE_ALLOC(NULL, 0, __real_strdup(s), "strdup");
}

char *__wrap_strndup(const char *s, size_t size) {
  UNRELIABLE_ALLOC(NULL, 0, __real_strndup(s, size), "strndup");
}

// Freeing memory always succeeds. We only log the fact of deallocation.
void __wrap_free(void *ptr) {
  test_data.call_total++;
  __real_free(ptr);
  if (ptr)
    test_data.free_counter++;
}

void memory_tests_check(void) {
  memory_test_data_t *mtd = get_memory_test_data();
  mtd->call_total = 0;
  mtd->call_counter = 0;
  mtd->fail_counter = 0;
  mtd->alloc_counter = 0;
  mtd->free_counter = 0;
  mtd->function_name = NULL;

  char *p1 = malloc(24);
  assert(p1);
  p1 = realloc(p1, 64);
  assert(p1);
  free(p1);

  int *p2 = calloc(18, sizeof (int));
  assert(p2);
  p2 = reallocarray(p2, 80, sizeof (int));
  assert(p2);
  free(p2);

  char *p3 = strdup("abcdefghijklmnopqrstuvwxyz");
  assert(p3);
  char *p4 = strndup(p3, 24);
  assert(p4);
  free(p3);
  free(p4);

  assert(mtd->call_total == 10);
  assert(mtd->call_counter == 6);
  assert(mtd->alloc_counter >= 4);
  assert(mtd->free_counter >= 4);
  assert(mtd->alloc_counter == mtd->free_counter);
}
