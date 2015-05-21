CC = g++
CFLAGS = -Wall -std=c++11
EXEC_NAME = rpg
INCLUDES =
LIBS = -pthread

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SRC_FILES = $(wildcard $(SRCDIR)/*.cpp )
OBJ_FILES = $(SRC_FILES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all : $(BINDIR)/$(EXEC_NAME)

.PHONEY: clean

clean :
	rm -f $(BINDIR)/$(EXEC_NAME) $(OBJ_FILES)

$(BINDIR)/$(EXEC_NAME) : $(OBJ_FILES)
	$(CC) -o $(BINDIR)/$(EXEC_NAME) $(OBJ_FILES) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc $(CFLAGS) $(INCLUDES) -o $@ -c $<
