#include "test.h"
#include "limits.h"

static void xor_trans(bits_t* next_state, const bits_t* input,
                      const bits_t* old_state, size_t, size_t) {
  next_state[0] = old_state[0] ^ input[0];
}

// Output function: emits 1 when state is 0, otherwise 0
static void output_if_zero(bits_t* output, const bits_t* state, size_t, size_t) {
  output[0] = state[0] == 0 ? 1 : 0;
}

// Transition function: adds input to old_state word-by-word (each word is a `bits_t`)
static void accumulate_input(bits_t* next_state, const bits_t* input, const bits_t* old_state, size_t n, size_t) {
  size_t full_words = (n + CHAR_BIT * sizeof(bits_t) - 1) / (CHAR_BIT * sizeof(bits_t));

  for (size_t i = 0; i < full_words; ++i) {
    next_state[i] = old_state[i] + input[i];
  }
}

// Test:
// - Automaton `a[0]`: toggles output between 1 and 0 on each step (produces a square wave)
// - Automaton `a[1]`: accumulates the toggling signal across multiple `bits_t` words
int accumulator_test(void) {
  size_t n = 10;
  moore_t* a[2];
  const bits_t* y;

  bits_t initial_state = 0;
  bits_t constant_input = 1;

  // Automaton a[0]: toggles its output on every step
  // Initial output is 1 due to output_if_zero logic
  a[0] = ma_create_full(1, 1, 1, xor_trans, output_if_zero, &initial_state);
  ASSERT(a[0] != NULL);

  // Automaton a[1]: acts as a multi-bit accumulator
  size_t input_size = CHAR_BIT * sizeof(bits_t) * (n - 1) + 13; // Enough for non-word-aligned inputs
  size_t state_size = CHAR_BIT * sizeof(bits_t) * n;
  a[1] = ma_create_simple(input_size, state_size, accumulate_input);
  ASSERT(a[1] != NULL);

  ASSERT(ma_set_input(a[0], &constant_input) == 0);

  y = ma_get_output(a[1]);
  ASSERT(y != NULL);

  // Initially, the accumulator should be zero
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == 0);
  }

  // Connect a[0]'s output to the first bit of a[1]'s input bits_t.
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_connect(a[1], CHAR_BIT * sizeof(bits_t) * i, a[0], 0, 1) == 0);
  }

  size_t steps = 50;
  for (size_t i = 0; i < steps; ++i) {
    ASSERT(ma_step(a, SIZE(a)) == 0);
    for (size_t j = 0; j < n; ++j) {
      ASSERT(y[j] == i / 2 + 1); // Accumulator increases every two steps.
    }
  }

  ma_delete(a[0]);
  for (size_t i = 0; i < steps; ++i) {
    ASSERT(ma_step(&a[1], 1) == 0);
  }

  ma_delete(a[1]);
  return PASS;
}