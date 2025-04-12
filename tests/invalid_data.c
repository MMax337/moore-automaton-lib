#include "test.h"

#include "errno.h"
#include "stdio.h"
#include "stdbool.h"

static void sum_trans(bits_t* next_state, const bits_t* input,
                      const bits_t* old_state, size_t, size_t) {
  next_state[0] = old_state[0] + input[0];
}

static void add_one(bits_t* output, const bits_t* state, size_t, size_t) {
  output[0] = state[0] + 1;
}

// Test functions
static int invalid_constructor(void);
static int invalid_connect(void);
static int invalid_disconnect(void);
static int invalid_set_input(void);
static int invalid_set_state(void);
static int invalid_get_output(void);
static int invalid_step(void);

// Tests the library functions for handling illegal arguments.
int invalid_data_test(void) {
  bool ok = true;
  if (invalid_constructor() != PASS) {
    ok = false;
    fprintf(stderr, "error in constructor\n");
  }
  if (invalid_connect() != PASS) {
    ok = false;
    fprintf(stderr, "error in connect\n");
  }
  if (invalid_disconnect() != PASS) {
    ok = false;
    fprintf(stderr, "error in disconnect\n");
  }
  if (invalid_set_input() != PASS) {
    ok = false;
    fprintf(stderr, "error in set input\n");
  }
  if (invalid_set_state() != PASS) {
    ok = false;
    fprintf(stderr, "error in set state\n");
  }
  if (invalid_get_output() != PASS) {
    ok = false;
    fprintf(stderr, "error in get output\n");
  }
  if (invalid_step() != PASS) {
    ok = false;
    fprintf(stderr, "error in invalid step\n");
  }

  return ok ? PASS : FAIL;
}

static int invalid_constructor(void) {
  const bits_t q1 = 1;
  
  ASSERT(ma_create_full(1, 0, 1, sum_trans, add_one, &q1) == NULL && errno == EINVAL);
  errno = 0;

  ASSERT(ma_create_full(1, 1, 0, sum_trans, add_one, &q1) == NULL && errno == EINVAL);
  errno = 0;

  ASSERT(ma_create_full(1, 1, 1, NULL, add_one, &q1) == NULL && errno == EINVAL);
  errno = 0;

  ASSERT(ma_create_full(1, 1, 1, sum_trans, NULL, &q1) == NULL && errno == EINVAL);
  errno = 0;

  ASSERT(ma_create_full(1, 1, 1, sum_trans, add_one, NULL) == NULL && errno == EINVAL);
  errno = 0;

  ASSERT(ma_create_simple(1, 0, sum_trans) == NULL && errno == EINVAL);
  errno = 0;

  ASSERT(ma_create_simple(1, 1, NULL) == NULL && errno == EINVAL);
  errno = 0;

  moore_t* m = ma_create_full(0, 1, 1, sum_trans, add_one, &q1);
  ASSERT(m != NULL);
  ma_delete(m);

  m = ma_create_simple(0, 1, sum_trans);
  ASSERT(m != NULL);
  ma_delete(m);

  return PASS;
}

static int invalid_connect(void) {
  moore_t* m[2];
  size_t input0 = 337, input1 = input0 + 1;

  m[0] = ma_create_simple(input0, 1, sum_trans);
  m[1] = ma_create_simple(1, input1, sum_trans);

  ASSERT(m[0] != NULL && m[1] != NULL);

  ASSERT(ma_connect(NULL, 0, m[1], 0, 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_connect(m[0], 0, NULL, 0, 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_connect(m[0], input0, m[1], 0, 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_connect(m[0], 0, m[1], input1, 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_connect(m[0], 0, m[1], 0, input0 + 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_connect(m[0], 0, m[1], 0, input0 + 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_connect(m[0], 5, m[1], 0, input0 + 6) == -1 && errno == EINVAL);
  errno = 0;
  
  ASSERT(ma_connect(m[0], 0, m[1], 1, input0) == 0);
  errno = 0;

  ma_delete(m[0]);
  ma_delete(m[1]);

  return PASS;
}

static int invalid_disconnect(void) {
  size_t input = 337;
  moore_t* m = ma_create_simple(input, 1, sum_trans);
  ASSERT(m != NULL);

  ASSERT(ma_disconnect(NULL, 0, 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_disconnect(m, 0, 0) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_disconnect(m, 0, input + 1) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_disconnect(m, input, 1) == -1 && errno == EINVAL);
  errno = 0;

  ma_delete(m);
  return PASS;
}

static int invalid_set_input(void) {
  bits_t q = 0;
  moore_t* m = ma_create_simple(1, 1, sum_trans);
  moore_t* m_empty = ma_create_simple(0, 1, sum_trans);

  ASSERT(m != NULL && m_empty != NULL);

  ASSERT(ma_set_input(NULL, &q) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_set_input(m, NULL) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_set_input(m_empty, NULL) == -1 && errno == EINVAL);
  errno = 0;

  ma_delete(m);
  ma_delete(m_empty);

  return PASS;
}

static int invalid_set_state(void) {
  bits_t q = 0;
  moore_t* m = ma_create_simple(1, 1, sum_trans);

  ASSERT(m != NULL);

  ASSERT(ma_set_state(NULL, &q) == -1 && errno == EINVAL);
  errno = 0;

  ASSERT(ma_set_input(m, NULL) == -1 && errno == EINVAL);
  errno = 0;

  ma_delete(m);

  return PASS;
}

static int invalid_get_output(void) {
  ASSERT(ma_get_output(NULL) == NULL && errno == EINVAL);
  errno = 0;

  return PASS;
}

static int invalid_step(void) {
  size_t n = 10;
  moore_t* m[n];

  ASSERT(ma_step(NULL, 10) == -1 && errno == EINVAL);
  errno = 0;

  for (size_t i = 0; i < n; ++i) {
    m[i] = ma_create_simple(1, 1, sum_trans);
    ASSERT(m[i] != NULL);
  }

  for (size_t i = 0; i < n; ++i) {
    moore_t* tmp = m[i];
    m[i] = NULL;

    ASSERT(ma_step(m, 10) == -1 && errno == EINVAL);
    errno = 0;

    m[i] = tmp;
  }
  
  for (size_t i = 0; i < n; ++i) {
    ma_delete(m[i]);
  }
  
  return PASS;
}