CC = gcc
SRC = $(wildcard src/*.c)
OUT = latte
INCLUDE = -Iexternal -Isrc -Iexternal/sabotar/src
CFLAGS = -std=c89 -Wall
LFLAGS = -l$(OUT) 

LIBNAME = lib$(OUT)

OBJS = $(SRC:%.c=%.o)

$(OUT): main.c $(LIBNAME).a
	$(CC) main.c -o $(OUT) -L. $(CFLAGS) $(INCLUDE) $(LFLAGS)

$(LIBNAME).a: $(OBJS)
	ar rcs $(LIBNAME).a src/*.o

%.o: %.c
	$(CC) -c $< -o $@ $(INCLUDE) $(CFLAGS) $(LFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
	rm -f $(LIBNAME).a
	rm -f $(OBJS)
