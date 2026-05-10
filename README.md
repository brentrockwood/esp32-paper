Two files created:

**`config.h`** — fill in your SSID/password and adjust the 4 GPIO pins to match your wiring. SCK (18) and MOSI (23) are fixed to the VSPI hardware bus; the other four can be any free GPIO.

**`paper.ino`** — the full sketch. Key points:

- `onBitmapUpload()` receives binary chunks directly into a `uint8_t` buffer — no string conversion, no null-byte problem.
- `handleDisplay()` is called by `WebServer` *after* the upload completes, so the buffer is fully populated before the display refresh starts.
- The GxEPD2 page loop (`firstPage` / `nextPage`) handles the display's limited internal RAM transparently.

**To send a bitmap from your computer:**
```bash
curl -F bitmap=@image.bin http://<esp32-ip>/display
```

Where `image.bin` is 3904 bytes: 122 rows × 32 bytes, 1bpp MSB-first (bit 7 = leftmost pixel, 1 = black).

**One thing to watch:** if the image appears rotated on first boot, change `display.setRotation(1)` in `setup()` — try `0`, `1`, `2`, or `3`.