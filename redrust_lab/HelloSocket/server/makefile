#目标项目名字
TARGET = server

#源文件
SRC = server.cpp\
	  Allocator.cpp

#object文件
OBJS = $(SRC:.cpp=.o)

#编译器
CC = g++

#头文件路径
INCLUDE = -I../include/

#链接库
LINKPARAM = -pthread

#编译选项
CFLAGS = -g -std=c++11

#宏定义选项
COMFLAGS = 

#定义目标文件生成规则
all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(LINKPARAM) $(OBJS)

#定义object文件生成规则
.SUFFIXES:.cpp.o
.cpp.o:
	$(CC) $(COMFLAGS) $(LINKPARAM) $(INCLUDE) -c $<

#定义清理函数
clean:
	rm $(OBJS) $(TARGET) core
