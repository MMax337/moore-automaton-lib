#ifndef MA_H
#define MA_H

#include <stddef.h>
#include <stdint.h>

typedef uint64_t bits_t;

typedef struct moore moore_t;
typedef void (*transition_function_t)(bits_t *next_state, const bits_t* input,
                                      const bits_t* state, size_t n, size_t s);
                                      
typedef void (*output_function_t)(bits_t *output, const bits_t* state,
                                  size_t m, size_t s);

moore_t* ma_create_full(size_t n, size_t m, size_t s, transition_function_t t,
                        output_function_t y, const bits_t* q);
moore_t* ma_create_simple(size_t n, size_t m, transition_function_t t);
void ma_delete(moore_t* a);
int ma_connect(moore_t* a_in, size_t in, moore_t* a_out, size_t out, size_t num);
int ma_disconnect(moore_t* a_in, size_t in, size_t num);
int ma_set_input(moore_t* a, const bits_t* input);
int ma_set_state(moore_t* a, const bits_t* state);
const bits_t* ma_get_output(const moore_t* a);
int ma_step(moore_t* at[], size_t num);

#endif
