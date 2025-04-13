#include "ma.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

#define INIT_MEM_SIZE 10

typedef uint64_t bits_t;

typedef struct {
  moore_t* automaton; // Pointer to the connected automaton.
  size_t  bit_idx;    // Index of the bit within the connected automaton (either input or output).
} connection_t;

typedef struct {
  connection_t args;
  size_t output_connection_idx;
} input_connection_t;

typedef struct {
  size_t sz;
  size_t capacity;
  connection_t* connections;  // Dynamic array of connections.
} output_connection_t;

struct moore {
  size_t state_bit_count;    // Number of bits representing a state.
  size_t num_input_bits;     // Number of bit signals for `input`.
  size_t num_output_bits;    // Number of bit signals for `output`.

  bits_t* next_state;        // Used for performance: avoids repeated allocations of next state bits.
  bits_t* state;             // Current state.
  bits_t* output;
  bits_t* input;

  input_connection_t* input_connections;     // Array of size `num_input_bits`.
  output_connection_t* output_connections;   // Array of size `num_output_bits`.

  transition_function_t trans_func;
  output_function_t out_func;
};

// Initializes a dynamic array `output_connection_t` with `0` capacity.
static int lazy_init_output_connection(output_connection_t* conn) {
  conn->sz = 0;
  conn->capacity = 0;
  conn->connections = NULL;
  
  return 0;
}

// Initializes a dynamic array `output_connection_t` with default (`INIT_MEM_SIZE`) capacity.
static int init_output_connection(output_connection_t* conn) {
  conn->sz = 0;
  conn->connections = malloc(INIT_MEM_SIZE * sizeof(*conn->connections));
  
  if (!conn->connections)  {
    errno = ENOMEM;
    return -1;
  }

  conn->capacity = INIT_MEM_SIZE;

  return 0;
}

static int add_output_connection(output_connection_t* conns, connection_t conn) {
  if (conns->capacity == 0) {
    // The connection array was lazily initialized. Perform a proper intialization.
    if (init_output_connection(conns) == -1) {
      return -1;
    }
  }
  if (conns->sz == conns->capacity) {
    size_t new_capacity = 2 * conns->capacity;
    connection_t* tmp = realloc(conns->connections, new_capacity * sizeof(*tmp));

    if (!tmp) {
      errno = ENOMEM;
      return -1;
    }

    conns->connections = tmp;
    conns->capacity = new_capacity;
  }

  conns->connections[conns->sz++] = conn;

  return 0;
}

static void pop_back(output_connection_t* conns) {
  if (conns->sz == 0) {
    return;
  }
  --conns->sz;
}

// Converts the `s` bits to `ceil(s/word_len)` where 
// the `word_len` is given by number of bits in `bits_t`.
static size_t bits_to_words(size_t s) {
  size_t bits_per_word = CHAR_BIT * sizeof(bits_t);
  return (s + bits_per_word - 1) / bits_per_word;
}

static void id_output(bits_t* output, const bits_t* state, size_t, size_t s) {
  memcpy(output, state, sizeof(bits_t) * bits_to_words(s)); 
}

// Retrieves the `n`-th bit from `bits`. Indexing is 0-based.
static int get_bit(const bits_t* bits, size_t n) {
  size_t idx = n / (CHAR_BIT * sizeof(bits_t));
  size_t bit_idx = n % (CHAR_BIT * sizeof(bits_t));

  return (bits[idx] >> bit_idx) & ((bits_t) 1);
}

// Sets the `n`-th bit of `bits`. Indexing is 0-based.
static void set_bit(bits_t* bits, size_t n) {
  size_t idx = n / (CHAR_BIT * sizeof(bits_t));
  size_t bit_idx = n % (CHAR_BIT * sizeof(bits_t));

  bits[idx] |= ((bits_t) 1) << bit_idx;
}

// Unsets the `n`-th bit of `bits`. Indexing is 0-based.
static void unset_bit(bits_t* bits, size_t n) {
  size_t idx = n / (CHAR_BIT * sizeof(bits_t));
  size_t bit_idx = n % (CHAR_BIT * sizeof(bits_t));

  bits[idx] &= ~(((bits_t) 1) << bit_idx);
}

// Sets n-th bit of `bits` to `bit`. Indexing is 0-based.
static void copy_bit(bits_t* bits, int bit, size_t n) {
  if (bit) {
    set_bit(bits, n);
  } else {
    unset_bit(bits, n);
  }
}

// Returns true if the range [`start`, `start + num`) is within total_bits, safely handling overflow.
static bool is_valid_range(size_t start, size_t total_bits, size_t num) {
  return num <= SIZE_MAX - start && start + num <= total_bits;
}

// Removes the connection between `aut_idx` automaton in output connections `conn`
// on both sides - input and output.
static void disconnect_output(output_connection_t* conn, size_t aut_idx) {
  if (!conn) {
    return;
  }
  
  connection_t tmp = conn->connections[aut_idx];
  conn->connections[aut_idx] = conn->connections[conn->sz - 1];
  pop_back(conn);

  moore_t* swapped = conn->connections[aut_idx].automaton;
  size_t bit = conn->connections[aut_idx].bit_idx;
  swapped->input_connections[bit].output_connection_idx = aut_idx;

  tmp.automaton->input_connections[tmp.bit_idx].args.automaton = NULL;
}

// Removes the connection on both sides given the input connection.
static void disconnect_input(input_connection_t* conn) {
  if (!conn->args.automaton) {
    return;
  }
  
  moore_t* output_aut = conn->args.automaton;
  size_t output_bit = conn->args.bit_idx;

  disconnect_output(&output_aut->output_connections[output_bit], conn->output_connection_idx);
}

moore_t* ma_create_full(size_t n, size_t m, size_t s, transition_function_t t,
                        output_function_t y, const uint64_t* q) {

  if (m == 0 || s == 0 || !t || !y || !q) {
    errno = EINVAL;
    return NULL;
  }

  moore_t* aut = malloc(sizeof(*aut));
  
  if (!aut) {
    errno = ENOMEM;
    return NULL;
  }
  
  aut->state_bit_count = s;
  aut->num_input_bits = n;
  aut->num_output_bits = m;

  aut->trans_func = t;
  aut->out_func = y;
  
  aut->state = calloc(bits_to_words(s), sizeof(*aut->state));
  aut->next_state = calloc(bits_to_words(s), sizeof(*aut->next_state));

  aut->input = n == 0 ? NULL : calloc(bits_to_words(n), sizeof(*aut->input));
  aut->input_connections = n == 0 ? NULL : malloc(n * sizeof(*aut->input_connections));

  aut->output = calloc(bits_to_words(m), sizeof(*aut->output));
  aut->output_connections = malloc(m * sizeof(*aut->output_connections));

  if (aut->output_connections) {
    for (size_t i = 0; i < m; ++i) {
      // Lazily initialize arrays to avoid allocating memory when there are
      // no active connections, preventing unnecessary memory usage.
      lazy_init_output_connection(&aut->output_connections[i]);
    }
  }
  
  if (aut->input_connections) {
    for (size_t i = 0; i < n; ++i) {
      aut->input_connections[i].args.automaton = NULL;
    }
  }

  if (!aut->state || !aut->next_state || !aut->output ||
      (n != 0 && (!aut->input || !aut->input_connections)) ||
      !aut->output_connections) {
    ma_delete(aut);
    errno = ENOMEM;
    return NULL;
  }
  
  memcpy(aut->state, q, sizeof(bits_t) * bits_to_words(s));
  aut->out_func(aut->output, q, aut->num_output_bits, aut->state_bit_count);

  return aut;
}

moore_t* ma_create_simple(size_t n, size_t m, transition_function_t t) {
  if (m == 0 || !t) {
    errno = EINVAL;
    return NULL;
  }

  bits_t* init_state = calloc(bits_to_words(m), sizeof(*init_state));

  if (!init_state) {
    errno = ENOMEM;
    return NULL;
  }

  moore_t* aut = ma_create_full(n, m, m, t, id_output, init_state);
  free(init_state);
  
  return aut;
}

void ma_delete(moore_t* a) {
  if (!a) return;
  
  free(a->state);
  free(a->next_state);
  free(a->output);
  free(a->input);

  if (a->input_connections) {
    for (size_t bit = 0; bit < a->num_input_bits; ++bit) {
      input_connection_t* in_conn = &a->input_connections[bit];
      disconnect_input(in_conn);
    }
  }
  
  if (a->output_connections) {
    for (size_t bit = 0; bit < a->num_output_bits; ++bit) {
      size_t sz = a->output_connections[bit].sz;
      for (size_t i = 0; i < sz; ++i) {
        disconnect_output(&a->output_connections[bit], a->output_connections[bit].sz - 1);
      }
      free(a->output_connections[bit].connections);
    }
  }

  free(a->input_connections);
  free(a->output_connections);
  free(a);
}

int ma_set_state(moore_t* a, const uint64_t* state) {
  if (!a || !state) {
    errno = EINVAL;
    return -1;
  }
  
  memcpy(a->state, state, sizeof(bits_t) * bits_to_words(a->state_bit_count));
  a->out_func(a->output, a->state, a->num_output_bits, a->state_bit_count);

  return 0;
}

int ma_set_input(moore_t* a, const uint64_t* input) {
  if (!a || !input || a->num_input_bits == 0) {
    errno = EINVAL;
    return -1;
  }
  
  for (size_t i = 0; i < a->num_input_bits; ++i) {
    if (a->input_connections[i].args.automaton) {
      // The signal is connected.
      continue;
    }
    copy_bit(a->input, get_bit(input, i), i);
  }

  return 0;
}

const uint64_t* ma_get_output(const moore_t* a) {
  if (!a) {
    errno = EINVAL;
    return NULL;
  }

  return a->output;
}

int ma_connect(moore_t* a_in, size_t in, moore_t* a_out, size_t out, size_t num) {
  if (!a_in || !a_out || num == 0 ||
      !is_valid_range(in, a_in->num_input_bits, num) || 
      !is_valid_range(out, a_out->num_output_bits, num)) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < num; ++i) {
    input_connection_t* in_conn = &a_in->input_connections[in + i];
    disconnect_input(in_conn);

    // Try to add the connection.
    if (add_output_connection(&a_out->output_connections[out + i],
                              (connection_t) {.automaton = a_in, .bit_idx = in + i}) == -1) {
      errno = ENOMEM;
      return -1;
    }

    in_conn->args.bit_idx = out + i;
    in_conn->args.automaton = a_out;
    in_conn->output_connection_idx = a_out->output_connections[out + i].sz - 1;
  }

  return 0;
}

int ma_disconnect(moore_t* a_in, size_t in, size_t num) {
  if (!a_in || num == 0 || !is_valid_range(in, a_in->num_input_bits, num)) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < num; ++i) {
    moore_t* a_out = a_in->input_connections[in + i].args.automaton;
    size_t bit_idx = a_in->input_connections[in + i].args.bit_idx;
    size_t a_in_idx = a_in->input_connections[in + i].output_connection_idx;

    if (a_out) {
      disconnect_output(&a_out->output_connections[bit_idx], a_in_idx);
    }
  }

  return 0;  
}

int ma_step(moore_t* at[], size_t num) {
  bool ok = num != 0 && at;
  for (size_t i = 0; i < num && ok; ++i) {
    ok = at[i] != NULL;
  }

  if (!ok) {
    errno = EINVAL;
    return -1;
  }

  // Update the inputs.
  for (size_t i = 0; i < num; ++i) {
    for (size_t j = 0; j < at[i]->num_input_bits; ++j) {
      if (!at[i]->input_connections[j].args.automaton) {
        continue;
      }
      int bit = get_bit(at[i]->input_connections[j].args.automaton->output,
                        at[i]->input_connections[j].args.bit_idx);
      copy_bit(at[i]->input, bit, j);
    }
  }

  for (size_t i = 0; i < num; ++i) {
    at[i]->trans_func(at[i]->next_state, at[i]->input, at[i]->state,
                      at[i]->num_input_bits, at[i]->state_bit_count);

    memcpy(at[i]->state, at[i]->next_state, sizeof(bits_t) * bits_to_words(at[i]->state_bit_count));
    at[i]->out_func(at[i]->output, at[i]->state, at[i]->num_output_bits, at[i]->state_bit_count);
  }

  return 0;
}