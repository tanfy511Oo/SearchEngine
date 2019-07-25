SRCS:=./client_success.cc
EXE:=../Bin/Client
CXX:=g++
CXXFLAGS:= -w  -ljson 
$(EXE):$(SRCS)
	$(CXX) -o $(EXE) $(SRCS) $(CXXFLAGS)

clean:
	rm -rf $(EXE)
