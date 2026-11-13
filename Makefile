CXX = g++
CXXFLAGS = -std=c++17 -Wall

# Detect Operating System and set SFML include/library paths accordingly
ifeq ($(shell uname), Darwin)
    # macOS: Check Homebrew paths (Apple Silicon vs Intel)
    ifeq ($(shell test -d /opt/homebrew && echo yes), yes)
        CXXFLAGS += -I/opt/homebrew/include
        LDFLAGS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system
    else
        CXXFLAGS += -I/usr/local/include
        LDFLAGS = -L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system
    endif
else
    # Linux (common for grading servers or Ubuntu/Debian setups)
    CXXFLAGS += -I/usr/include
    LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
endif

TARGET = hospital_sim
SRC = main.cpp visual.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)