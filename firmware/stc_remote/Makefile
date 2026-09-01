SDCC ?= sdcc
PACKIHX ?= packihx
CFLAGS = -mmcs51 --std-sdcc99 --model-large --opt-code-size --xram-size 2048 --code-size 61440 -Iinclude
all: build/petcargo_remote.hex
build:
	mkdir -p build
build/petcargo_remote.ihx: main.c include/stc15.h | build
	$(SDCC) $(CFLAGS) main.c -o $@
build/petcargo_remote.hex: build/petcargo_remote.ihx
	$(PACKIHX) $< > $@
clean:
	rm -rf build
