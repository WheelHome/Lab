#目标项目名字
TARGET = client

#源文件
SRC = client.cpp

#object文件
OBJS = $(SRC:.cpp=.o)

#编译器
CC = g++

#头文件路径
INCLUDE = -l/usr/include

#链接库
LINKPARAM = -pthread

#编译选项
CFALGS = -std=c++11

#定义目标文件生成规则
all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(LINKPARAM) $(OBJS)

#定义object文件生成规则
.SUFFIXES:.cpp.o
.cpp.o:
	$(CC) $(LINKPARAM)  $(CFALGS) -c $<

#定义清理函数
clean:
	rm $(OBJS) $(TARGET)
