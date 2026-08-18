## ESP32 e-paper display

Hosts a tiny HTTP server on an ESP32. Send a raw 1bpp bitmap via multipart POST and it appears on a 2.13" e-paper display (LAFVIN / Waveshare, SSD1680 driver).

Inspired by [TRMNL](https://trmnl.com/) — the device is intentionally dumb (bitmap sink only), so all content logic lives off-board in scripts.

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

#### Power supply

The display's onboard boost converter (which generates ±15V to drive the e-ink panel) pulls hard current spikes during refresh. This causes problems with noisy or high-impedance power sources:

- **Works:** dedicated USB charger, USB power bank, powered hub + USB isolator
- **Unreliable:** PC USB port directly, powered hub without isolator (shared ground with PC carries switching noise)

Also add a **100µF bypass capacitor** from VCC to GND as close to the display connector as possible. Without it, the current spikes droop the 3.3V rail mid-refresh and produce partial or garbled updates.

**Anti-static mats** are conductive — don't let the board rest on one while powered.

### Setup

1. Install [PlatformIO](https://platformio.org/).
2. Copy `include/config.h.example` to `include/config.h` and fill in your WiFi SSID/password. `include/config.h` is gitignored so credentials won't be committed.
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

### Converting images

`scripts/img_to_epaper.py` converts any image (JPEG, PNG, etc.) to the correct format. It resizes to fit 250×122 with white letterboxing and applies Floyd-Steinberg dithering.

```bash
python3 scripts/img_to_epaper.py photo.jpg image.bin
curl -F bitmap=@image.bin http://<esp32-ip>/display
```

Requires Pillow: `pip install Pillow`.

### Rendering text

`scripts/text_to_epaper.py` renders an argument or standard input directly to the raw bitmap.

```bash
python3 scripts/text_to_epaper.py 'Hello, e-paper' > message.bin
printf 'Hello\nworld' | python3 scripts/text_to_epaper.py > message.bin
curl -F bitmap=@message.bin http://<esp32-ip>/display
```

### Troubleshooting

**Busy Timeout in serial log** — the display enters deep sleep after each refresh and requires a hardware RST pulse to wake. The firmware re-initialises the display before every update to handle this; if you're still seeing timeouts, check that the RST wire (GPIO 16) has continuity.

**Garbled or partial image** — almost always a power or signal integrity issue, not a software bug. Check the power supply section above. SPI is run at 2 MHz (half the default) for better tolerance of long or loose jumper wires; if you have a solid soldered setup you can increase it to 4 MHz in `setup()`.

**Driver variant** — this board uses `GxEPD2_213_B74` (GDEH0213B74). If you have a different LAFVIN/Waveshare panel that doesn't initialise correctly, try `GxEPD2_213_BN` (Waveshare V2).
