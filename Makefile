ifeq ($(DESTDIR)$(PREFIX),)
	PREFIX := /usr/local
endif

CC=gcc
CCFLAGS=-Wall -Wextra -fPIE -pie -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -Wformat -Wformat=2 -Wimplicit-fallthrough -fstack-clash-protection -fstack-protector-strong 
BUILDDIR=./build
SRCDIR=./src
LIBS=-lpthread -lconfig
objects=$(addprefix $(BUILDDIR)/, allowlist.o main.o aprs.o nagios.o net.o settings.o)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CCFLAGS)

raven: $(objects)
	$(CC) -o $@ $^ $(CCFLAGS) $(LIBS)

all: raven

.PHONY: clean

clean: 
	rm -f $(BUILDDIR)/*.o
	rm -f raven 

symbols:
	objdump -tC $(BUILDDIR)/*.o

debug: CCFLAGS += -g
debug: clean raven

install: raven
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 raven $(DESTDIR)$(PREFIX)/bin/
	install -m 600 -o raven etc/raven.cfg $(DESTDIR)/etc/
	install -m 644 etc/raven.service $(DESTDIR)/etc/systemd/system/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/raven
	rm -f $(DESTDIR)/etc/raven.cfg
	rm -f $(DESTDIR)/etc/systemd/system/raven.service

