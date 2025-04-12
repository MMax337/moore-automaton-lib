#include "test.h"
#include "errno.h"
#include "stdio.h"

#define V(code, where) (((unsigned long)code) << (3 * where))

static void sum_trans(bits_t* next_state, const bits_t* input,
                      const bits_t* old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

static void xor_trans(bits_t* next_state, const bits_t* input,
                      const bits_t* old_state, size_t, size_t) {
  next_state[0] = old_state[0] ^ input[0];
}

static void add_one(bits_t* output, const bits_t* state, size_t, size_t) {
  output[0] = state[0] + 1;
}

// Tests the implementation's response to memory allocation failure.
// The allocation failure is reported once. The second attempt should succeed.
static unsigned long alloc_fail_test(void) {
  const uint64_t q1 = 1;
  unsigned long visited = 0;
  moore_t *maf, *mas;
  size_t n = 1000;

  errno = 0;
  if ((maf = ma_create_full(n, n, n, sum_trans, add_one, &q1)) != NULL) {
    visited |= V(1, 0);
  } else if (errno == ENOMEM &&
            (maf = ma_create_full(n, n, n, sum_trans, add_one, &q1)) != NULL) {
    visited |= V(2, 0);
  } else {
    return visited |= V(4, 0); // To nie powinno się wykonać.
  }

  errno = 0;
  if ((mas = ma_create_simple(1, 1, xor_trans)) != NULL) {
    visited |= V(1, 1);
  } else if (errno == ENOMEM && (mas = ma_create_simple(1, 1, xor_trans)) != NULL) {
    visited |= V(2, 1);
  } else {
    return visited |= V(4, 1); // To nie powinno się wykonać.
  }
  
  for (size_t i = 0; i < n; ++i) {
    if (ma_connect(maf, i, mas, 0, 1) != -1) {
      visited |= V(1, 3);
    } else if (errno == ENOMEM && ma_connect(maf, i, mas, 0, 1) != -1) {
      visited |= V(2, 3);
    } else {
      return visited |= V(4, 2);
    }
  }

  for (size_t i = 0; i < n; ++i) {
    if (ma_step(&maf, 1) != -1 && ma_step(&mas, 1) != -1) {
      visited |= V(1, 4);
    } else if (errno == ENOMEM && ma_step(&maf, 1) != -1 && ma_step(&mas, 1) != -1) {
      visited |= V(2, 4);
    } else {
      return visited |= V(4, 3);
    }
  }

  ma_delete(maf);
  ma_delete(mas);

  return visited;
}

// Tests the implementation's response to memory allocation failure.
static int memory_test_runner(unsigned long (* test_function)(void)) {
  memory_test_data_t* mtd = get_memory_test_data();

  unsigned fail = 0, pass = 0;
  mtd->call_total = 0;
  mtd->fail_counter = 1;
  while (fail < 3 && pass < 3) {
    mtd->call_counter = 0;
    mtd->alloc_counter = 0;
    mtd->free_counter = 0;
    mtd->function_name = NULL;
    unsigned long visited_points = test_function();
    if (mtd->alloc_counter != mtd->free_counter ||
        (visited_points & 0444444444444444444444UL) != 0) {
      // 0444444444444444444444UL is in binary:
      // 0100100100100100100100100100100100100100100100100100100100100100
      fprintf(stderr,
              "fail_counter %u, alloc_counter %u, free_counter %u, "
              "function_name %s, visited_point %lo\n",
              mtd->fail_counter, mtd->alloc_counter, mtd->free_counter,
              mtd->function_name, visited_points);
      ++fail;
    }

    if (mtd->function_name == NULL) {
      ++pass;
    }
    else {
      pass = 0;
    }
    mtd->fail_counter++;
  }
  // Disable bad malloc.
  mtd->fail_counter = 0;
  return mtd->call_total > 0 && fail == 0 ? PASS : FAIL;
}

int memory_test(void) {
  memory_tests_check();
  return memory_test_runner(alloc_fail_test);
}