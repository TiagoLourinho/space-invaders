# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude # All ".h" will be inside include/
LDFLAGS = -lncurses -lzmq 

# Directories
SRC_DIR = src # Contains the source code
BIN_DIR = bin # Will contain the object files
RUN_DIR = run # Will contain the final program executables
COMMON_DIR = $(SRC_DIR)/common # Contains the shared functions

# Collect all ".c" files inside common and generate the corresponding ".o" in bin
COMMON_SRCS = $(wildcard $(COMMON_DIR)/*.c)
COMMON_OBJS = $(patsubst $(COMMON_DIR)/%.c, $(BIN_DIR)/%.o, $(COMMON_SRCS))

# Collect all the ".c" files of each program
GAME_SERVER_SRCS = $(wildcard $(SRC_DIR)/game-server/*.c)
ASTRONAUT_CLIENT_SRCS = $(wildcard $(SRC_DIR)/astronaut-client/*.c)
OUTER_SPACE_DISPLAY_SRCS = $(wildcard $(SRC_DIR)/outer-space-display/*.c)

#################### Targets ####################

all: directories game-server astronaut-client outer-space-display

# Creates the bin and run folder if they dont exist
directories:
	mkdir -p $(BIN_DIR) $(RUN_DIR)

# Each program depends on all the common objects created and all the source files in the respective folder
game-server: $(COMMON_OBJS) $(GAME_SERVER_SRCS)
	$(CC) $(CFLAGS) $(GAME_SERVER_SRCS) $(COMMON_OBJS) -o $(RUN_DIR)/$@ $(LDFLAGS)
astronaut-client: $(COMMON_OBJS) $(ASTRONAUT_CLIENT_SRCS)
	$(CC) $(CFLAGS) $(ASTRONAUT_CLIENT_SRCS) $(COMMON_OBJS) -o $(RUN_DIR)/$@ $(LDFLAGS)
outer-space-display: $(COMMON_OBJS) $(OUTER_SPACE_DISPLAY_SRCS)
	$(CC) $(CFLAGS) $(OUTER_SPACE_DISPLAY_SRCS) $(COMMON_OBJS) -o $(RUN_DIR)/$@ $(LDFLAGS)

# Compile common source files into object files
$(BIN_DIR)/%.o: $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(RUN_DIR)/* $(BIN_DIR)/*
