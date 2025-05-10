CC=gcc
CCFLAGS=-Wall -Wextra -fPIE -pie -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -Wformat -Wformat=2 -Wimplicit-fallthrough -fstack-clash-protection -fstack-protector-strong 
BUILDDIR=./build
SRCDIR=./src
LIBS=-lpthread
objects=$(addprefix $(BUILDDIR)/, main.o app.o kiss.o ringbuffer.o serial.o yframe.o)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CCFLAGS)

raven: $(objects)
	$(CC) -o $@ $^ $(CCFLAGS) $(LIBS)

all: raven

.PHONY: clean

clean: 
	rm -f $(BUILDDIR)/*.o
	rm -f dvtnc 
	rm -f test_rbuffer
	rm -f test_yframe

symbols:
	objdump -tC $(BUILDDIR)/*.o

debug: CCFLAGS += -g
debug: clean raven

