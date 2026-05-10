ENV  ?= esp32dev
PORT ?= $(firstword $(wildcard /dev/cu.usbserial-* /dev/cu.SLAB_USBtoUART* /dev/cu.wchusbserial* /dev/cu.usbmodem*))

.DEFAULT_GOAL := help

.PHONY: help build upload monitor clean

help:
	@printf "Usage: make <target>\n\n"
	@printf "  build    Compile the project\n"
	@printf "  upload   Compile and upload to the board\n"
	@printf "  monitor  Open serial monitor\n"
	@printf "  clean    Remove build artifacts\n\n"
	@printf "Variables (override on the command line):\n"
	@printf "  PORT=<dev>   Serial port (auto-detected: '%s')\n" "$(PORT)"
	@printf "  ENV=<env>    PlatformIO environment (default: %s)\n" "$(ENV)"

build:
	pio run -e $(ENV)

upload:
	@[ -n "$(PORT)" ] || { echo "Error: no serial port found; set PORT=/dev/cu.xxx"; exit 1; }
	pio run -e $(ENV) --target upload --upload-port $(PORT)

monitor:
	@[ -n "$(PORT)" ] || { echo "Error: no serial port found; set PORT=/dev/cu.xxx"; exit 1; }
	pio device monitor --port $(PORT)

clean:
	pio run --target clean
