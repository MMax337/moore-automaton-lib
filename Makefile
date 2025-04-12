CC      =  gcc
CFLAGS  = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc \
          -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup \
          -Wl,--wrap=strndup

TEST_DIR = tests
TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
SRC = ma.c memory_tests.c
OBJS = $(SRC:.c=.o)

DEPS = $(SRC:.c=.d)
Library = libma.so
Example = ma_example

INCLUDES = -I$(TEST_DIR)

.PHONY: clean all test

all: $(Library) $(Example)

$(Library): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(Example): $(Example).o $(Library) $(TEST_OBJ)
	$(CC) $^ -o $@ -L. -lma -Wl,-rpath=.

test: $(Example)
	valgrind --leak-check=full --show-leak-kinds=all -q ./$(Example) all
	
-include $(DEPS)

clean:
	rm -f $(Library) $(Example) *.o *.d
	rm -f $(TEST_DIR)/*.o $(TEST_DIR)/*.d

