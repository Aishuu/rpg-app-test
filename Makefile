CC = gcc
CFLAGS = -Wall
EXEC_NAME = ann
INCLUDES =
LIBS = -lX11 -lGL -lGLU

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SRC_FILES = $(wildcard $(SRCDIR)/*.c )
OBJ_FILES = $(SRC_FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all : $(BINDIR)/$(EXEC_NAME)

.PHONEY: clean

clean :
	rm -f $(BINDIR)/$(EXEC_NAME) $(OBJ_FILES)

$(BINDIR)/$(EXEC_NAME) : $(OBJ_FILES)
	$(CC) -o $(BINDIR)/$(EXEC_NAME) $(OBJ_FILES) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<
