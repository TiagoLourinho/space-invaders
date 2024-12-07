# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude # All ".h" will be inside include/
LDFLAGS = -lncurses -lzmq 

# Collect all ".c" files inside common and generate the corresponding ".o" in bin
COMMON_SRCS = $(wildcard src/common/*.c)
COMMON_OBJS = $(patsubst src/common/%.c, ./bin/%.o, $(COMMON_SRCS))

# Collect all the ".c" files of each program
GAME_SERVER_SRCS = $(wildcard src/game-server/*.c)
ASTRONAUT_CLIENT_SRCS = $(wildcard src/astronaut-client/*.c)
OUTER_SPACE_DISPLAY_SRCS = $(wildcard src/outer-space-display/*.c)

#################### Targets ####################

all: teste directories game-server astronaut-client outer-space-display


teste:
	@echo ################# DEBUG #################
	@echo Common sources: $(COMMON_SRCS)
	@echo Common objects: $(COMMON_OBJS)
	@echo Game server sources: $(GAME_SERVER_SRCS)
	@echo Astronaut client sources: $(ASTRONAUT_CLIENT_SRCS)
	@echo Outer space display sources: $(OUTER_SPACE_DISPLAY_SRCS)
	@echo #################       #################


# Creates the bin and run folder if they dont exist
directories:
	mkdir -p bin run

# Each program depends on all the common objects created and all the source files in the respective folder
game-server: $(COMMON_OBJS) $(GAME_SERVER_SRCS)
	$(CC) $(CFLAGS) $(GAME_SERVER_SRCS) $(COMMON_OBJS) -o run/$@ $(LDFLAGS)
astronaut-client: $(COMMON_OBJS) $(ASTRONAUT_CLIENT_SRCS)
	$(CC) $(CFLAGS) $(ASTRONAUT_CLIENT_SRCS) $(COMMON_OBJS) -o run/$@ $(LDFLAGS)
outer-space-display: $(COMMON_OBJS) $(OUTER_SPACE_DISPLAY_SRCS)
	$(CC) $(CFLAGS) $(OUTER_SPACE_DISPLAY_SRCS) $(COMMON_OBJS) -o run/$@ $(LDFLAGS)

# Compile common source files into object files
./bin/%.o: src/common/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm ./run/* ./bin/*
