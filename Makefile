# C Compiler (Try zig's maybe?)
CC = gcc

# Flags for Compiler
CFLAGS = -Wall -g -O2

# Libraries
LIBS = -lpthread -lpigpio

# Source C files
SRC = src/main.c src/sensors.c

# Output (per specification)
OUT = followLine

# BUILD
$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LIBS)

# CLEAN OUTPUT
clean:
	rm -f $(OUT)
