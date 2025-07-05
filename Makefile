OBJDIR = .
SRCDIR = .
INCLDIR = .
OUTDIR = .

## General Flags
PROGRAM = lex
CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -O0 -I $(INCLDIR)

## Objects that must be built in order to link
OBJECTS = $(OBJDIR)/lex.o


## Build both compiler and program
all: $(OBJECTS)

## Compile source files
$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/$*.o $< 

clean:
	-rm -rf $(OBJECTS)

