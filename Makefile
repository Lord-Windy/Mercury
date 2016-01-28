INCLUDE_PATHS = -I/usr/local/include
LIBRARY_PATHS = -L/usr/local/lib 
COMPILER_FLAGS = -std=gnu++11 -Wall -Wpedantic
LINKER_FLAGS = -levent

OBJS = main.cpp
CC = g++

OBJ_NAME = bin/Mercury.out

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
