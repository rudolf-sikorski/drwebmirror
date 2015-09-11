CC ?= cc
CFLAGS ?= -O3 -Wall -Wextra
LDFLAGS ?= -s
PREFIX ?= /usr/local
EXECUTABLE = drwebmirror
SOURCES = \
	src/main.c \
	src/common.c \
	src/filesystem.c \
	src/network.c \
	src/decompress.c \
	src/checksum.c \
	src/drwebmirror.c \
	src/avltree/avltree.c \
	src/crc32/crc32.c \
	src/md5/md5c.c \
	src/sha256/sha256.c \
	src/lzma/Alloc.c \
	src/lzma/LzmaDec.c \
	src/lzma/7zFile.c \
	src/lzma/7zStream.c
OBJECTS = $(SOURCES:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

.PHONY: clean distclean install uninstall

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(EXECUTABLE)

install: all
	install -Dm 0755 $(EXECUTABLE) $(DESTDIR)$(PREFIX)/bin/$(EXECUTABLE)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(EXECUTABLE)
