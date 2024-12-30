# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Isrc/proto # All ".h" will be inside include/ and src/proto/
LDFLAGS = -lncurses -lzmq -lpthread -lprotobuf -lprotobuf-c

# ProtoBuf settings
PROTOC = protoc
PROTO_SRC_DIR = src/proto
PROTO_SRC_FILES = $(PROTO_SRC_DIR)/scores.proto
PROTO_C_FILES = $(PROTO_SRC_DIR)/scores.pb-c.c
PROTO_H_FILES = $(PROTO_SRC_DIR)/scores.pb-c.h

# Collect all ".c" files inside common and generate the corresponding ".o" in bin
COMMON_SRCS = $(wildcard src/common/*.c)
COMMON_OBJS = $(patsubst src/common/%.c, ./bin/%.o, $(COMMON_SRCS))

# Collect all the ".c" files of each program
GAME_SERVER_SRCS = $(wildcard src/game-server/*.c)
ASTRONAUT_CLIENT_SRCS = $(wildcard src/astronaut-client/*.c)
OUTER_SPACE_DISPLAY_SRCS = $(wildcard src/outer-space-display/*.c)
ASTRONAUT_DISPLAY_CLIENT_SRCS = $(wildcard src/astronaut-display-client/*.c)

#################### Targets ####################

all: directories proto_files game-server astronaut-client outer-space-display astronaut-display-client

# Debug information
debug:
	@echo ################# DEBUG #################
	@echo Common sources: $(COMMON_SRCS)
	@echo Common objects: $(COMMON_OBJS)
	@echo Game server sources: $(GAME_SERVER_SRCS)
	@echo Astronaut client sources: $(ASTRONAUT_CLIENT_SRCS)
	@echo Outer space display sources: $(OUTER_SPACE_DISPLAY_SRCS)
	@echo Astronaut display client sources: $(ASTRONAUT_DISPLAY_CLIENT_SRCS)
	@echo Proto source files: $(PROTO_SRC_FILES)
	@echo #################       #################

# Creates the bin and run folder if they don't exist
directories:
	mkdir -p bin run

# Generate proto files
proto_files: $(PROTO_C_FILES) $(PROTO_H_FILES)

$(PROTO_C_FILES) $(PROTO_H_FILES): $(PROTO_SRC_FILES)
	$(PROTOC) --proto_path=$(PROTO_SRC_DIR) --c_out=$(PROTO_SRC_DIR) --python_out=$(PROTO_SRC_DIR) $<

# Each program depends on all the common objects created and all the source files in the respective folder
game-server: $(COMMON_OBJS) $(GAME_SERVER_SRCS) $(PROTO_C_FILES) $(PROTO_H_FILES)
	$(CC) $(CFLAGS) $(GAME_SERVER_SRCS) $(COMMON_OBJS) $(PROTO_C_FILES) -o run/$@ $(LDFLAGS)
astronaut-client: $(COMMON_OBJS) $(ASTRONAUT_CLIENT_SRCS)
	$(CC) $(CFLAGS) $(ASTRONAUT_CLIENT_SRCS) $(COMMON_OBJS) -o run/$@ $(LDFLAGS)
outer-space-display: $(COMMON_OBJS) $(OUTER_SPACE_DISPLAY_SRCS)
	$(CC) $(CFLAGS) $(OUTER_SPACE_DISPLAY_SRCS) $(COMMON_OBJS) -o run/$@ $(LDFLAGS)
astronaut-display-client: $(COMMON_OBJS) $(ASTRONAUT_DISPLAY_CLIENT_SRCS)
	$(CC) $(CFLAGS) $(ASTRONAUT_DISPLAY_CLIENT_SRCS) $(COMMON_OBJS) -o run/$@ $(LDFLAGS)

# Compile common source files into object files
./bin/%.o: src/common/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm ./run/* ./bin/* src/proto/scores.pb-c.* src/proto/scores_pb2.py
