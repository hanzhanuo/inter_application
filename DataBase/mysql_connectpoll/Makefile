SRCS:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp, %.o, $(SRCS))
BIN:=Test_mysql_cilent.exe
CC:=g++
FLAGS:=-g -w

$(BIN):$(OBJS)
	$(CC) -o $@ $^ $(FLAGS) -lmysqlclient -llog4cpp
%.o:%.cpp
	$(CC) -c $^ -o $@ $(FLAGS)

clean:
	rm -rf $(OBJS) $(BIN)