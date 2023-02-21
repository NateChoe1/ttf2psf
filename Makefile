SRC = $(wildcard src/*.c)
OBJ = $(subst .c,.o,$(subst src,work,$(SRC)))
CFLAGS := -O2 -pipe -Wall -Wpedantic -Wshadow
CFLAGS += -Isrc/include/
LIBS = freetype2
LDFLAGS = $(shell pkg-config --libs $(LIBS))
INSTALLDIR := /usr/bin/
OUT = ttf2psf
CLEAN = $(wildcard work/*) $(wildcard build/*)

build/$(OUT): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o build/$(OUT)

work/%.o: src/%.c $(wildcard src/include/*.h)
	$(CC) $(CFLAGS) $(shell pkg-config --cflags $(LIBS)) $< -c -o $@

install: build/$(OUT)
	cp build/$(OUT) $(INSTALLDIR)/$(OUT)
	cp -r ./data /usr/share/ttf2psf

uninstall: $(INSTALLDIR)/$(OUT)
	rm $(INSTALLDIR)/$(OUT)
	rm -r /usr/share/ttf2psf

clean: $(CLEAN)
	rm $(CLEAN)

.PHONY: install uninstall clean
