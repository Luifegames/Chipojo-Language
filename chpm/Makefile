CC = gcc
CFLAGS = -Wall -Iinclude -g
OBJDIR = obj
SRCDIR = src
INCDIR = include

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

PACKETDIR = $(HOME)/packets_chpm_create

.PHONY: all clean install

all: $(OBJDIR) chpm

$(OBJDIR):
	mkdir -p $(OBJDIR)

chpm: $(OBJS)
	$(CC) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Install built-in packets so chpm install <name> can find them in ~/packets_chpm_create/
install:
	mkdir -p $(PACKETDIR)
	cp -R packets_chpm_create/. $(PACKETDIR)/
	@echo "Built-in packets installed to $(PACKETDIR)/"

clean:
	rm -rf $(OBJDIR) chpm
