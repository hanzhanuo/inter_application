SRCS:=$(wildcard *.cpp *.cc)
OBJS:=$(patsubst %.cpp,%.cc, %.o, $(SRCS))
BIN:=Test_Server.exe
CC:=g++
FLAGS:=-g -w

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS)
%.o:%.cpp
	$(CC) -c $^ -o $@ $(FLAGS)

clean:
	rm -rf $(OBJS) $(BIN)