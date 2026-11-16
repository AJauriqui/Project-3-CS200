CXX = g++
CXXFLAGS = -std=c++17 -Wall

ifeq ($(shell uname), Darwin)
    ifeq ($(shell test -d /opt/homebrew && echo yes), yes)
        CXXFLAGS += -I/opt/homebrew/include
        LDFLAGS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system
    else
        CXXFLAGS += -I/usr/local/include
        LDFLAGS = -L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system
    endif
else
    CXXFLAGS += -I/usr/include
    LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
endif

TARGET = hospital_sim
SRCS = main.cpp visual.cpp

# Default target when you just type 'make'
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Aliases for running the program
run: $(TARGET)
	./$(TARGET)

start: $(TARGET)
	./$(TARGET)

# Clean up build artifacts
clean:
	rm -f $(TARGET)

# Declare non-file targets
.PHONY: all run start clean