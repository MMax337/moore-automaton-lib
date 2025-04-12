#ifdef NDEBUG
#undef NDEBUG
#endif

#include "../tests/test.h"
#include "stdio.h"
#include <string.h>

static const test_list_t test_list[] = {
  TEST(basic_test),
  TEST(two_bit_adder_test),
  TEST(accumulator_test),
  TEST(n_bit_adder_test),
  TEST(invalid_data_test),
  TEST(connection_test),
  TEST(memory_test),
};

static int do_test(test_t function) {
  int result = function();
  printf(result == PASS ? "\033[0;32m Passed! \033[0m\n"
                        : "\033[0;31m Failed :( \033[0m\n");
  return result;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    if (strcmp(argv[1], "all") == 0) {
      const int width = 40;
      for (size_t i = 0; i < SIZE(test_list); ++i) {
        printf("Running test: %-*s", width, test_list[i].name);
        int res = do_test(test_list[i].function);
        if (res != PASS) {
          return res;
        }
      }
      return PASS;
    } else {
      for (size_t i = 0; i < SIZE(test_list); ++i) {
        if (strcmp(argv[1], test_list[i].name) == 0) {
          return do_test(test_list[i].function);
        }
      }
    }
  }
  fprintf(stderr, "Usage:\n%s test_name or %s all\n", argv[0], argv[0]);
  return WRONG_TEST;
}
