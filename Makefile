CXX = g++
CXXFLAGS = -Wall -std=c++17

CLIENT_SRC = client/client.cpp
SERVER_SRC = server/server.cpp

CLIENT_BIN = bin/client
SERVER_BIN = bin/server

all: client server

client: $(CLIENT_SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC)

server: $(SERVER_SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(SERVER_BIN) $(SERVER_SRC)

clean:
	rm -rf bin

.PHONY: all client server clean
