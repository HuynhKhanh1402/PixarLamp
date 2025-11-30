# Pixar Luxo Lamp Animation Makefile
# Compiler settings
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2

# Libraries
LDFLAGS = -lGL -lGLU -lglut -lm

# Target executable
TARGET = PixarLamp

# Source files
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete! Run with: ./$(TARGET)"

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild everything
rebuild: clean all

# Run the program
run: $(TARGET)
	./$(TARGET)

# Help target
help:
	@echo Available targets:
	@echo   all      - Build the project (default)
	@echo   clean    - Remove build artifacts
	@echo   rebuild  - Clean and build
	@echo   run      - Build and run the program
	@echo   help     - Show this help message

.PHONY: all clean rebuild run help
