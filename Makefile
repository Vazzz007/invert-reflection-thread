CC=g++
CFLAGS=-c -Wall -Werror -Wshadow -pedantic-errors \
	-Wpointer-arith  \
	-Wmissing-declarations \
	-Wlong-long -Winline -Wredundant-decls \
	-Wcast-qual -Wcast-align -D__STRICT_ANSI__ -pthread -g
LDFLAGS=-pthread
SOURCES=main.cpp create_matrix.cpp args.cpp func_eval.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=invert

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o $(EXECUTABLE)
