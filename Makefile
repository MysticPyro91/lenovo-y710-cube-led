CC ?= gcc
CFLAGS ?= -O2
CFLAGS += -Wall -Wextra
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

.PHONY: all install uninstall clean
all: y710-led

y710-led: y710-led.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

install: y710-led
	install -Dm755 y710-led $(DESTDIR)$(BINDIR)/y710-led

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/y710-led

clean:
	rm -f y710-led
