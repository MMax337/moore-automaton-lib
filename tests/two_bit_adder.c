#include "test.h"

static void xor_trans(bits_t* next_state, const bits_t* input,
                  const bits_t* old_state, size_t, size_t) {
  next_state[0] = old_state[0] ^ input[0];
}

int two_bit_adder_test(void) {
  bits_t x = 1;
  const bits_t* y[2];
  moore_t* a[2];

  a[0] = ma_create_simple(1, 1, xor_trans);
  a[1] = ma_create_simple(1, 1, xor_trans);
  assert(a[0]);
  assert(a[1]);

  y[0] = ma_get_output(a[0]);
  y[1] = ma_get_output(a[1]);
  ASSERT(y[0] != NULL);
  ASSERT(y[1] != NULL);

  // At the begining counter is set to 00.
  ASSERT(ma_set_input(a[0], &x) == 0);
  ASSERT(ma_connect(a[1], 0, a[0], 0, 1) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 0);

  // After one step counter is set to 01.
  ASSERT(ma_step(a, SIZE(a)) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 1);

  // After two steps counter is set to 10.
  ASSERT(ma_step(a, SIZE(a)) == 0);
  ASSERT(y[1][0] == 1 && y[0][0] == 0);

  // After three steps counter is set to 11.
  ASSERT(ma_step(a, SIZE(a)) == 0);
  ASSERT(y[1][0] == 1 && y[0][0] == 1);

  // After four steps counter is set to 00.
  ASSERT(ma_step(a, SIZE(a)) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 0);
  ASSERT(ma_step(a, 2) == 0);

  // After five steps counter is set to 00.
  ASSERT(y[1][0] == 0 && y[0][0] == 1);

  // When automata are disconnected the counter must stop changing.
  ASSERT(ma_disconnect(a[1], 0, 1) == 0);
  x = 0;
  ASSERT(ma_set_input(a[1], &x) == 0);
  ASSERT(ma_step(a, SIZE(a)) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 0);
  ASSERT(ma_step(a, SIZE(a)) == 0);
  ASSERT(y[1][0] == 0 && y[0][0] == 1);

  ma_delete(a[0]);
  ma_delete(a[1]);
  return PASS;
}