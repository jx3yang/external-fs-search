SERVICE = service

# source files
SRC_DIR = src
SRC_EXT = cpp
SRC_FILES = $(wildcard $(SRC_DIR)/*.$(SRC_EXT))

# header files
INC_DIR = include
INCLUDES = -I. -I,$(INC_DIR)

# objects
OBJ_DIR = obj
OBJS = $(SRC_FILES:$(SRC_DIR)/%.$(SRC_EXT)=$(OBJ_DIR)/%.o)

# dependencies
DEPS = $(OBJS:.o=.d)

# compiler
CC = clang++
CFLAGS = -std=c++17 -Wall -g
CFLAGS += $(INCLUDES)
LFLAGS = -framework CoreServices

all: clean $(SERVICE)

$(SERVICE): $(OBJS)
	@mkdir -p $(@D)
	@echo "Building $@"
	$(CC) $(LFLAGS) $^ -o $(SERVICE)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	-rm -f $(SERVICE)
	-rm -f $(OBJS)
	-rm -f $(DEPS)
