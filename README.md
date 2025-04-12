# Moore Automaton Library

This is a C library for simulating Moore automata. The library provides functions for creating and manipulating Moore machines, including state transitions, input/output management, and synchronization of multiple automata. It is well-tested and includes a test suite located in the `tests` folder. The `ma_example` program demonstrates how to use the library and run tests.

## Features

- **Create Moore Automata**: Create fully customized Moore machines or simple ones with default behavior.
- **State Transition**: Perform transitions based on input signals and the current state.
- **Output Calculation**: Calculate output signals based on the current state.
- **Multiple Automata Support**: Manage and simulate multiple automata simultaneously.
- **Flexible API**: Provide customizable transition and output functions.
- **Memory Management**: Handles dynamic memory allocation and deallocation.

## Functions

### `ma_create_full`

Creates a fully customized Moore automaton.

```c
moore_t* ma_create_full(size_t n, size_t m, size_t s, transition_function_t t, output_function_t y, const bits_t* q);
```
**Parameters:**
- `n`: Number of input signals of the automaton.
- `m`: Number of output signals of the automaton.
- `s`: Number of bits representing the internal state of the automaton.
- `t`: Transition function, which calculates the next state of the automaton based on the current state and input signals.
- `y`: Output function, which calculates the output signals based on the current state of the automaton.
- `q`: Pointer to a bit sequence representing the initial state of the automaton.

### `ma_create_simple`

Creates a simple Moore automaton with default behavior.

```c
moore_t* ma_create_simple(size_t n, size_t m, transition_function_t t);
```
**Parameters:**
- `n`: Number of input signals.
- `m`: Number of output signals, which will be equal to the number of state bits (`m == s`).
- `t`: Transition function to compute the next state based on the current state and inputs. The output function is automatically set to identity (i.e., output equals the state).

### `ma_delete`

Deletes a Moore automaton and frees associated memory.

```c
void ma_delete(moore_t* a);
```

### `ma_connect`

Connects input signals of one automaton to output signals of another. If there were connected signals, then disconnects them.

```c
int ma_connect(moore_t* a_in, size_t in, moore_t* a_out, size_t out, size_t num);
```
**Parameters:**
- `a_in`: Pointer to the automaton whose input signals are to be connected.
- `in`: The starting index of the input signals in `a_in` that will be connected.
- `a_out`: Pointer to the automaton whose output signals will be connected.
- `out`: The starting index of the output signals in `a_out` to which the input signals will be connected.
- `num`: The number of signals to connect.

**Return Value:**
- Returns `0` on success.
- Returns `-1` on error (e.g., if any pointer is `NULL`, or if invalid signal numbers are specified).


### `ma_disconnect`

Disconnects input signals from output signals.

```c
int ma_disconnect(moore_t* a_in, size_t in, size_t num);
```
**Parameters:**
- `a_in`: Pointer to the automaton whose input signals are to be disconnected.
- `in`: The index of the first input signal to disconnect.
- `num`: The number of signals to disconnect.

**Return Value:**
- Returns `0` on success.
- Returns `-1` on error (e.g., if `a_in` is `NULL`, or invalid signal ranges are specified).

### `ma_set_input`

Sets input values for the automaton.

```c
int ma_set_input(moore_t* a, const bits_t* input);
```

### `ma_set_state`

Sets the internal state of the automaton.

```c
int ma_set_state(moore_t* a, const bits_t* state);
```

### `ma_get_output`

Gets the current output of the automaton.

```c
const bits_t* ma_get_output(const moore_t* a);
```

### `ma_step`

Performs a synchronous step of computation for multiple automata.

```c
int ma_step(moore_t* at[], size_t num);
```
**Parameters:**
- `at`: An array of pointers to automata that should take a step.
- `num`: The number of automata in the array.

**Return Value:**
- Returns `0` on success.
- Returns `-1` on error (e.g., if any pointer in the array is `NULL`, or `num` is `0`).

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/MMax337/moore-automaton-lib
   ```

2. Compile the library:
   ```bash
   make
   ```

## Testing

The library includes a suite of tests located in the `tests` folder. You can run the tests using the `ma_example` program:

```bash
./ma_example all
```
or 
```bash
make test
```

The tests include various scenarios to ensure the library functions as expected. The tests also simulate a lack of memory by intercepting `malloc`, `calloc`, and other memory allocation functions during test execution, to ensure the library handles memory allocation errors properly.
