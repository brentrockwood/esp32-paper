## ESP32 e-paper display

Hosts a tiny HTTP server on an ESP32. Send a raw 1bpp bitmap via multipart POST and it appears on a 2.13" e-paper display (LAFVIN / Waveshare, SSD1680 driver).

### Hardware

| Display pin | Function | ESP32 GPIO |
|---|---|---|
| DIN | MOSI | 23 (VSPI, fixed) |
| CLK | SCK | 18 (VSPI, fixed) |
| CS | Chip select | 5 |
| DC | Data/Command | 17 |
| RST | Reset | 16 |
| BUSY | Busy signal | 4 |
| VCC | Power | 3.3V |
| GND | Ground | GND |

Pin assignments are in `include/config.h`.

### Setup

1. Install [PlatformIO](https://platformio.org/).
2. Edit `include/config.h` — fill in your WiFi SSID/password and adjust GPIO pins if needed.
3. Build and upload:

```bash
make upload   # auto-detects serial port
# or
pio run -e esp32dev --target upload --upload-port /dev/cu.usbserial-xxx
```

### HTTP API

```
POST /display   multipart/form-data, field "bitmap"
                Raw binary, 3904 bytes (122 rows × 32 bytes, 1bpp MSB-first)
                bit 1 = black, bit 0 = white; rows padded to 32 bytes

GET  /clear     Fill display white
GET  /          Status page
```

Send a bitmap:

```bash
curl -F bitmap=@image.bin http://<esp32-ip>/display
```

`image.bin` must be exactly 3904 bytes: 122 rows × 32 bytes, 1bpp MSB-first (bit 7 = leftmost pixel, 1 = black).

If the image appears rotated, change `display.setRotation(1)` in `src/main.cpp` — try `0`, `1`, `2`, or `3`.
