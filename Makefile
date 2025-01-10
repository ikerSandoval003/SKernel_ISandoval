# Compiler
CC = gcc

# Source files
SRCS = kernel.c 

# Header files
HDRS = kernel.h

# Output executable
TARGET = kernel

# Compiler flags
CFLAGS = -g 

# Build target
$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean target
clean:
	rm -f $(TARGET)

# Run target
run: $(TARGET)
	./$(TARGET) 5 2 2 2 2 2 2

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) 5 2 2 2 2 1