CC      = gcc
CFLAGS  = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc \
          -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup \
          -Wl,--wrap=strndup

TEST_DIR = tests
SRC_DIR = src
BUILD_DIR = build
LIBRARY_DIR = $(BUILD_DIR)/lib
EXAMPLE_DIR = $(BUILD_DIR)/example
TEST_BUILD_DIR = $(BUILD_DIR)/tests

TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
TEST_OBJ = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRC))
DEPS = $(OBJS:.o=.d)
Library = $(LIBRARY_DIR)/libma.so
Example_SRC = $(SRC_DIR)/ma_example.c
Example_OBJ = $(BUILD_DIR)/ma_example.o
Example = $(EXAMPLE_DIR)/ma_example

INCLUDES = -I$(TEST_DIR)

.PHONY: clean all test


all: $(Library) $(Example)

# Rule for compiling src files into object files in build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)/$(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(Library): $(OBJS)
	@mkdir -p $(LIBRARY_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# Rule for compiling test files into object files in build/tests/
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(TEST_BUILD_DIR)/$(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Rule for compiling example source into object file.
$(Example_OBJ): $(Example_SRC)
	@mkdir -p $(BUILD_DIR)/$(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Rule for linking the example
$(Example): $(Example_OBJ) $(Library) $(TEST_OBJ)
	@mkdir -p $(EXAMPLE_DIR)
	$(CC) $^ -o $@ -L$(LIBRARY_DIR) -lma -Wl,-rpath=$(LIBRARY_DIR)

test: $(Example)
	valgrind --leak-check=full --show-leak-kinds=all -q ./$(Example) all
	
-include $(DEPS)

clean:
	rm -f $(Library) $(Example)
	rm -rf $(BUILD_DIR)
