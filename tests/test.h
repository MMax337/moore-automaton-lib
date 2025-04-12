#ifndef TEST_H
#define TEST_H

#include "../src/ma.h"
#include "../src/memory_tests.h"
#include "assert.h"

// Test results.
#define PASS 0
#define FAIL 1
#define WRONG_TEST 2

#define SIZE(x) (sizeof x / sizeof x[0])

#define ASSERT(f)                        \
  do {                                   \
    if (!(f))                            \
      return FAIL;                       \
  } while (0)

  
#define TEST(t) {#t, t}

typedef int (*test_t)(void);

typedef struct {
  const char* name;
  test_t function;
} test_list_t;


int basic_test(void);
int two_bit_adder_test(void);
int accumulator_test(void);
int n_bit_adder_test(void);
int connection_test(void);
int invalid_data_test(void);
int memory_test(void);



#endif