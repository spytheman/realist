SRC:=$(wildcard main*.nelua)
EXE:=$(patsubst %.nelua,%.elf,$(SRC))
PPM:=$(patsubst %.nelua,%.ppm,$(SRC))

NELUA:=nelua

all: $(EXE)
	du -sh .

check: $(PPM)
	md5sum *.ppm
	md5sum *.ppm | md5sum

OPT:=-O3
#DEBUG:=1
NFLAGS+=-q --no-cache --cache-dir . -c
ifdef DEBUG
#NFLAGS+=
endif

#PROF:=1
ifdef PROF
OPT+=-pg
endif

%.c: %.nelua
	$(NELUA) $(NFLAGS) -o $@ $^

%.elf: %.c
	$(CC) -o $@ $^ $(OPT) -lm

%.ppm: %.elf
	./$^ > $@

clean:
	$(RM) *.c *.elf *.ppm
