SDCC ?= sdcc
PACKIHX ?= packihx
CFLAGS = -mmcs51 --std-sdcc99 --model-large --opt-code-size --xram-size 2048 --code-size 61440 -Iinclude -DFOSC=11059200UL
SOURCES = main.c src/hal.c src/protocol.c src/devices.c src/oled.c src/petcargo.c
RELS = $(patsubst %.c,build/%.rel,$(notdir $(SOURCES)))

.PHONY: all clean
all: build/petcargo.hex

build:
	mkdir -p build

build/%.rel: main.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@

build/hal.rel: src/hal.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@
build/protocol.rel: src/protocol.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@
build/devices.rel: src/devices.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@
build/oled.rel: src/oled.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@
build/petcargo.rel: src/petcargo.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@

build/petcargo.ihx: $(RELS)
	$(SDCC) -mmcs51 --model-large --xram-size 2048 --code-size 61440 $(RELS) -o $@

build/petcargo.hex: build/petcargo.ihx
	$(PACKIHX) $< > $@

clean:
	rm -rf build
