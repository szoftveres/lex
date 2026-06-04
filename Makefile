OBJDIR = .
SRCDIR = .
INCLDIR = .
OUTDIR = .

## General Flags
PROGRAM = lex

## Objects that must be built in order to link
OBJECTS = $(OBJDIR)/lex.o


## Build both compiler and program
all: $(OBJECTS)

## Compile source files
$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(PLATFORM_CC) $(PLATFORM_CFLAGS) -c -o $(OBJDIR)/$*.o $< 
	$(PLATFORM_AR) rcs $(PROGRAM).a $(OBJECTS)

clean:
	-rm -rf $(OBJECTS)

