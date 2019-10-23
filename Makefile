CXX		  := g++
CXX_FLAGS         := -Wall -Wextra -std=c++17 -ggdb

BIN		:= bin
SRC		:= src
INC  	        := inc
LIB		:= lib

EXECUTABLE	:= libpcid

staticexec: $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INC) -I$(BOOST_INCL) -L$(LIB) $^ -static -fPIC $(LIBRARIES) -o $(BIN)/pcid

sharedlibrary: $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INC) -I$(BOOST_INCL) -L$(LIB) $^ -shared -fPIC $(LIBRARIES) -o $(BIN)/libpcid.so

all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp 
	$(CXX) $(CXX_FLAGS) -I$(INC) -I$(BOOST_INCL) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
