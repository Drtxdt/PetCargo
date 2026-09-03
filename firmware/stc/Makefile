SDCC ?= sdcc
PACKIHX ?= packihx
CFLAGS = -mmcs51 --std-sdcc99 --model-large --opt-code-size --xram-size 2048 --code-size 61440 -Iinclude -DFOSC=11059200UL
SOURCES = main.c src/hal.c src/protocol.c src/devices.c src/oled.c src/petcargo.c src/runtime.c src/music.c src/score.c
RELS = $(patsubst %.c,build/%.rel,$(notdir $(SOURCES)))

.PHONY: all diagnostic clean
all: build/petcargo.hex build/petcargo_diagnostic.hex
diagnostic: build/petcargo_diagnostic.hex

build:
	mkdir -p build

build/main.rel: main.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@
build/diagnostic.rel: diagnostic.c | build
	$(SDCC) $(CFLAGS) -c $< -o $@
build/%.rel: src/%.c | build
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

build/petcargo_diagnostic.ihx: build/diagnostic.rel build/hal.rel build/oled.rel build/runtime.rel build/music.rel build/score.rel
	$(SDCC) -mmcs51 --model-large --xram-size 2048 --code-size 61440 $^ -o $@
build/petcargo_diagnostic.hex: build/petcargo_diagnostic.ihx
	$(PACKIHX) $< > $@

clean:
	rm -f build/*.rel build/*.ihx build/*.hex build/*.mem
