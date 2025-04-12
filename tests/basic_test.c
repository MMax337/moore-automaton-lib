#include "test.h"

static void sum_trans(bits_t* next_state, const bits_t* input,
                      const bits_t* old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

static void add_one(bits_t* output, const bits_t* state, size_t, size_t) {
  output[0] = state[0] + 1;
}

// Does some simple addtions.
int basic_test(void) {
  const bits_t q1 = 1, x3 = 3, *y;

  moore_t* a = ma_create_full(64, 64, 64, sum_trans, add_one, &q1);
  assert(a);

  y = ma_get_output(a);
  ASSERT(y != NULL);
  ASSERT(ma_set_input(a, &x3) == 0);
  ASSERT(y[0] == 2);

  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 5);

  ASSERT(ma_step(&a, 1) == 0);

  ASSERT(y[0] == 8);
  ASSERT(ma_set_input(a, &q1) == 0);
  ASSERT(ma_set_state(a, &x3) == 0);
  ASSERT(y[0] == 4);

  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 5);
  ASSERT(ma_step(&a, 1) == 0);
  ASSERT(y[0] == 6);
  
  ma_delete(a);
  return PASS;
}