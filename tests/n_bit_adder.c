#include "test.h"

// Transition function for a single automaton.
// If the input is all 1s, toggle the state. Otherwise, keep the current state.
static void t_three(bits_t* next_state, const bits_t* input, 
                    const bits_t* old_state, size_t n, size_t) {
  bits_t all_set = (1ULL << n) - 1;

  if (input[0] == all_set) {
    next_state[0] = old_state[0] == 1 ? 0 : 1; 
  } else {
    next_state[0] = old_state[0];
  }
}

// Tests n automata forming an n-bit binary counter.
int n_bit_adder_test(void) {
  bits_t x = 1;
  size_t n = 10;
  const bits_t* y[n];
  moore_t* a[n];

  for (size_t i = 0; i < n; ++i) {
    size_t input_states = i < 2 ? 1 : i;
    a[i] = ma_create_simple(input_states, 1, t_three);
    assert(a[i]);
    y[i] = ma_get_output(a[i]);
    ASSERT(y[i] != NULL);
  }

  ASSERT(ma_set_input(a[0], &x) == 0);

  // Connect the i-th automaton to all automata from index 0 to i-1.
  for (size_t i = 1; i < n; ++i) {
    for (size_t j = 0; j < i; ++j) {
      ASSERT(ma_connect(a[i], j, a[j], 0, 1) == 0);
    }
  }

  // Check initial state: all automata should start with output 0.
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i][0] == 0);
  }

  size_t num = 0;

  // Perform 2^n - 1 steps, verifying binary counter behavior at each step.
  for (size_t i = 0; i < (( 1ULL << n) - 1); ++i) {
    ASSERT(ma_step(a, n) == 0);
    ++num;
    for (size_t j = 0; j < n; ++j) {
      size_t expected = (num >> j) & 1ULL;
      ASSERT(y[j][0] == expected);
    }
  }

  // Delete every second automaton (even indices) for additional robustness check.
  for (size_t i = 0; i < n; i += 2) {
    ma_delete(a[i]);
    a[i] = NULL;
  }

  // Step the remaining automata.
  for (size_t i = 0; i < n; ++i) {
    if (a[i]) {
      ASSERT(ma_step(&a[i], 1) == 0);
    }
  }

  // Clean up the remaining automata.
  for (size_t i = 0; i < n; ++i) {
    ma_delete(a[i]);
  }

  return PASS;
}