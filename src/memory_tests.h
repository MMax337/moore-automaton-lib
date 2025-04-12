#ifndef MEMORY_TESTS_H
#define MEMORY_TESTS_H

// This is a structure that holds information about memory operations.
// We do not allow the compiler to optimize operations on these values.
typedef struct {
  volatile unsigned call_total;    // counter for all function calls
  volatile unsigned call_counter;  // counter for allocation calls
  volatile unsigned fail_counter;  // number of failed allocations
  volatile unsigned alloc_counter; // number of successful allocations
  volatile unsigned free_counter;  // number of deallocations
  volatile char* function_name;    // name of the failed function
} memory_test_data_t;

// Provides access to the structure defined above.
memory_test_data_t* get_memory_test_data(void);

// Tests the functionality of the memory management testing module.
void memory_tests_check(void);

#endif
