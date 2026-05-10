SKETCH := paper.ino
FQBN   ?= esp32:esp32:esp32
PORT   ?= $(firstword $(wildcard /dev/cu.usbserial-* /dev/cu.SLAB_USBtoUART* /dev/cu.wchusbserial* /dev/cu.usbmodem*))
BUILD  := build
LIBS   := GxEPD2

.DEFAULT_GOAL := help

.PHONY: help build upload clean deps

help:
	@printf "Usage: make <target>\n\n"
	@printf "  deps     Install required Arduino libraries\n"
	@printf "  build    Compile the sketch\n"
	@printf "  upload   Compile and upload to the board\n"
	@printf "  clean    Remove build artifacts\n\n"
	@printf "Variables (override on the command line):\n"
	@printf "  PORT=<dev>   Serial port (auto-detected: '%s')\n" "$(PORT)"
	@printf "  FQBN=<fqbn>  Board FQBN  (default: %s)\n" "$(FQBN)"

build:
	arduino-cli compile --fqbn $(FQBN) --build-path $(BUILD) .

upload:
	@[ -n "$(PORT)" ] || { echo "Error: no serial port found; set PORT=/dev/cu.xxx"; exit 1; }
	arduino-cli compile --fqbn $(FQBN) --build-path $(BUILD) --upload --port $(PORT) .

deps:
	arduino-cli lib install $(LIBS)

clean:
	rm -rf $(BUILD)
