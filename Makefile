CC = clang
CFLAGS = -Wall -Werror -Wpedantic -Wextra -g -O2
LDFLAGS = -lm
OBJECTS = code.o huffman.o io.o node.o pq.o stack.o

all: encode decode

encode: $(OBJECTS) encode.o
	$(CC) $(LDFLAGS) -o encode $(OBJECTS) encode.o

decode: $(OBJECTS) decode.o
	$(CC) $(LDFLAGS) -o decode $(OBJECTS) decode.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f encode decode $(OBJECTS) encode.o decode.o

format:
	clang-format -i -style=file *.[c,h]

scan-build: clean
	scan -build make
