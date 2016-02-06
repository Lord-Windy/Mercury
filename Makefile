ifeq ($(OS), Darwin)

	INCLUDE_PATHS = -I/usr/local/include
	LIBRARY_PATHS = -L/usr/local/lib
else

	INCLUDE_PATHS = -I/usr/include -I/usr/pgsql-9.4/include
	LIBRARY_PATHS = -L/usr/lib -L/usr/pgsql-9.4/lib
endif

COMPILER_FLAGS = -std=gnu++11 -Wall -Wpedantic
C_COMPILER_FLAGS = -std=gnu11
LINKER_FLAGS = -levent -lpq

OBJS = main.cpp
CC = clang++

OBJ_NAME = bin/Mercury.out

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

