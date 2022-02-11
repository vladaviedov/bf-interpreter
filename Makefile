CC = g++
CXXFLAGS = -Wall -Wextra -g
EXEC_NAME = bfi

all: main.cpp
	$(CC) $(CXXFLAGS) $? -o $(EXEC_NAME)

clean:
	rm $(EXEC_NAME)