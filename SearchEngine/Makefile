INC_DIR := ./Inc	./Inc/threadpool	./Inc/net	./Inc/timer 
SRC_DIR := ./Src	./Src/threadpool	./Src/net	./Src/timer
SRCS:=$(wildcard ./Src/*.cc) $(wildcard ./Src/threadpool/*.cc) $(wildcard ./Src/net/*.cc) $(wildcard ./Src/timer/*.cc)
EXE:=./Bin/Server
CXX:=g++
LIBS:=-lpthread -ljson
CXXFLAGS:= -w  -std=c++11  $(addprefix -I, $(INC_DIR)) $(LIBS) -Wno-deprecated

$(EXE):$(SRCS)
	$(CXX) -o $(EXE) $(SRCS) $(CXXFLAGS)

clean:
	rm -rf $(EXE)
