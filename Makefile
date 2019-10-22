CXX		  := g++
CXX_FLAGS         := -Wall -Wextra -std=c++17 -ggdb

BIN		:= bin
SRC		:= src
INC  	        := inc
LIB		:= lib

BOOST_ROOT := /usr/lib/x86_64-linux-gnu/
BOOST_INCL := ${BOOST_ROOT}

LIBRARIES	:= -lcrypto -lboost_program_options -lboost_filesystem
EXECUTABLE	:= pcid

staticlibrary: $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INC) -I$(BOOST_INCL) -L$(LIB) $^ -static -fPIC $(LIBRARIES) -o $(BIN)/pcid.a

sharedlibrary: $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INC) -I$(BOOST_INCL) -L$(LIB) $^ -shared -fPIC $(LIBRARIES) -o $(BIN)/pcid.so

all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp 
	$(CXX) $(CXX_FLAGS) -I$(INC) -I$(BOOST_INCL) -L$(LIB) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*
