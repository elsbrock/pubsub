RELEASE = 0.1alpha
COMMIT  = $(shell git rev-parse --short=6 HEAD)
VERSION = $(RELEASE)-$(COMMIT)

# auto-enable debugging in case a git repo was checked out
ifneq ("$(COMMIT)", "")
DEBUG  ?= 1
endif

ifeq ($(DEBUG),1)
CFLAGS ?= -g3 -Fdwarf -DDEBUG
else
CFLAGS ?= -O3 -DNDEBUG
endif

CFLAGS  += -DVERSION='"$(VERSION)"'
CFLAGS  += -Wall -Wextra -Wno-unused-parameter -pedantic -pipe
CFLAGS  += -std=c99 -D_GNU_SOURCE
CFLAGS  += -Iinclude
LDFLAGS += -lev
# -ljansson -lz

all: pubsubd

pubsubd: src/main.o src/net.o src/log.o src/sig.o src/misc.o src/mqtt.o src/util.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o : src/main.c src/net.c src/log.c src/sig.c src/misc.c src/mqtt.c src/util.c

clean:
	rm -f **/*.o **/.*.swp .*.swp pubsubd

#.SILENT:
.PHONY: clean
