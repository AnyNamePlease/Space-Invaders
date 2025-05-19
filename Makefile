
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = \
    main.cpp \
    Game.cpp \
    Terminal.cpp \
    Entity.cpp \
    Bullet.cpp \
    Explosion.cpp \
    Player.cpp \
    Enemy.cpp \
    Barrier.cpp

OBJ = $(SRC:.cpp=.o)
TARGET = space_invaders

all: $(TARGET)

$(TARGET): $(OBJ)
    $(CXX) $(CXXFLAGS) -o $@ $^ -lncurses

%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
    rm -f $(OBJ) $(TARGET)
