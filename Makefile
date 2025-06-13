CXX = g++
CXXFLAGS = -Wall -std=c++17 -Igenerated

LDFLAGS = -lprotobuf

CLIENT_SRC = client/client.cpp
SERVER_SRC = server/server.cpp server/client_manager.cpp server/game_manager.cpp generated/game.pb.cc

CLIENT_BIN = bin/client
SERVER_BIN = bin/server

all: client server

client: $(CLIENT_SRC) generated/game.pb.cc
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC) generated/game.pb.cc $(LDFLAGS)

server: $(SERVER_SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(SERVER_BIN) $(SERVER_SRC) $(LDFLAGS)

clean:
	rm -rf bin

.PHONY: all client server clean
