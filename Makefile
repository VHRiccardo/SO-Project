TARGET=	exec

CC= gcc

CFLAGS= -Wall -Werror -g

SRC=	functions.c FileSystem.c File.c ./linenoise/linenoise.c Shell.c main.c
HEADERS=	functions.h FileSystem.h File.h ./linenoise/linenoise.h Shell.h
OBJ=	functions.o FileSystem.o File.o ./linenoise/linenoise.o Shell.o main.o

all: $(TARGET)

$(TARGET):	$(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o:	%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)


