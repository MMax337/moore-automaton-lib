#include "test.h"
#include "limits.h"


// Transition function: preserves state unchanged
static void t_steady(bits_t* next_state, const bits_t*, 
                     const bits_t* old_state, size_t, size_t s) {
  size_t full_words = (s + CHAR_BIT * sizeof(bits_t) - 1) / (CHAR_BIT * sizeof(bits_t));

  for (size_t i = 0; i < full_words; ++i) {
    next_state[i] = old_state[i];
  }
}

// Transition function: copies input to state (used as input latch)
static void t_copy_input(bits_t* next_state, const bits_t* input,
                          const bits_t*, size_t n, size_t) {  
  size_t full_words = (n + CHAR_BIT * sizeof(bits_t) - 1) / (CHAR_BIT * sizeof(bits_t));

  for (size_t i = 0; i < full_words; ++i) {
    next_state[i] = input[i];
  }
}

// Tests correctness of `ma_connect`, `ma_disconnect` and dynamic input overriding.
int connection_test(void) {
  size_t n = 10;
  moore_t* a[3];
  const bits_t* y;
  bits_t all_one = ~0ULL;
  
  // a[0] and a[1]: steady-state automata (just hold their state)
  a[0] = ma_create_simple(0, 64, t_steady);
  a[1] = ma_create_simple(0, 64, t_steady);

  // a[2]: state follows input; will be wired to a[0] and a[1] dynamically.
  a[2] = ma_create_simple(64 * n, 64 * n, t_copy_input);

  ASSERT(a[0] != NULL && a[1] != NULL && a[2] != NULL);

  // Set all bits of a[1]'s state to 1.
  ASSERT(ma_set_state(a[1], &all_one) == 0);
  
  y = ma_get_output(a[2]);
  ASSERT(y != NULL);

  // Connect all words of a[2]'s input to a[1]'s output.
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_connect(a[2], 64*i, a[1], 0, 64) == 0);
  }

  ASSERT(ma_step(a, SIZE(a)) == 0);
  
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == ~0ULL);
  }

  // Override 32 bits starting at bit 16 of each word from a[0] (all zeros).
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_connect(a[2], 64*i + 16, a[0], 0, 32) == 0);
  }

  ASSERT(ma_step(a, SIZE(a)) == 0);
  
  // Resulting pattern: 16 ones | 32 zeros | 16 ones.
  bits_t expected = (0xFFFFULL << 48) | (0xFFFFULL);

  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == expected);
  }

  // Add 8 more bits from a[1] at bit offset 28 of each word.
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_connect(a[2], 64*i + 28, a[1], 0, 8) == 0);
  }

  ASSERT(ma_step(a, SIZE(a)) == 0);

  // Final bit pattern: 16 ones | 12 zeros | 8 ones | 12 zeros | 16 ones.
  expected = (0xFFFFULL << 48) | (0xFFULL << 28) | 0xFFFFULL;
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == expected);
  }

  // Override entire input of a[2] with a[0] (all zeros)
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_connect(a[2], 64*i, a[0], 0, 64) == 0);
  }
  ASSERT(ma_step(a, SIZE(a)) == 0);

  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == 0ULL);
  }

  // Set a[2]'s input directly (all 1s).
  bits_t input[n];
  for (size_t i = 0; i < n; ++i) {
    input[i] = 1;
  }

  // Nothing should change (connections override direct input).
  ASSERT(ma_set_input(a[2], input) == 0);
  ASSERT(ma_step(&a[2], 1) == 0);
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == 0ULL);
  }

  ma_delete(a[0]);
  ASSERT(ma_set_input(a[2], input) == 0);
  ASSERT(ma_step(&a[1], 2) == 0);

  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == 1);
  }

  // Now reconnect a[1] to selective slices.
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_connect(a[2], 64*i, a[1], 0, 16) == 0);
    ASSERT(ma_connect(a[2], 64*i + 28, a[1], 10, 8) == 0);
    ASSERT(ma_connect(a[2], 64*i + 48, a[1], 0, 16) == 0);
  }

  ASSERT(ma_step(&a[2], 1) == 0);
  // Connections to a[2] are structured as follows:
  //  (16+), (12-), (8+), (12-), (16+)
  // Where "+" indicates a connection  and "-" indicates no connection.
  expected = (0xFFFFULL << 48) | (0xFFULL << 28) | 0xFFFFULL;

  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == expected);
  }

  for (size_t i = 0; i < n; ++i) {
    input[i] = ~expected;
  }
  ASSERT(ma_set_input(a[2], input) == 0);
  ASSERT(ma_step(&a[2], 1) == 0);

  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == ~0ULL);
  }

  // Disconnect leading 16 bits of each word.
  for (size_t i = 0; i < n; ++i) {
    ASSERT(ma_disconnect(a[2], 64*i, 16) == 0);
  }

  // Connections to a[2] are as follows:
  //  (28 -), (8 +), (12 -), (16 +)
  // Where "+" indicates a connection and "-" indicates no connection.
  // The input bit pattern:
  // 28 zeroes | 8 ones | 12 zeroes | 16 zeros | 16 ones.
  for (size_t i = 0; i < n; ++i) {
    input[i] = 0;
  }

  ASSERT(ma_set_input(a[2], input) == 0);
  ASSERT(ma_step(&a[2], 1) == 0);

  expected = (0xFFULL << 28) |  (0xFFFFULL << 48); 
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == expected);
  }

  // Delete a[1], now nothing is connected.
  ma_delete(a[1]);

  ASSERT(ma_set_input(a[2], input) == 0);
  ASSERT(ma_step(&a[2], 1) == 0);
  for (size_t i = 0; i < n; ++i) {
    ASSERT(y[i] == 0);
  }

  ma_delete(a[2]);

  return PASS;
}