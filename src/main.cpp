// main.cpp — ESP32 WiFi e-paper display server
//
// Hosts a tiny HTTP server. Send a raw 1bpp bitmap via multipart POST and it
// appears on a 2.13" e-paper display (LAFVIN / Waveshare, SSD1680 driver).
//
// Required library (platformio.ini lib_deps):
//   ZinggJM/GxEPD2
//
// ── HTTP API ──────────────────────────────────────────────────────────────────
//   POST /display   multipart/form-data, field name "bitmap"
//                   Body: raw binary, exactly EPD_BITMAP_BYTES (3904) bytes
//                   Format: 1bpp, MSB-first, row-major
//                           bit 1 = black, bit 0 = white
//                           rows padded to 32 bytes (250px + 6 unused bits)
//                   curl: curl -F bitmap=@image.bin http://<ip>/display
//
//   GET  /clear     Fill display white.
//   GET  /          Status page with IP and curl example.

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <GxEPD2_BW.h>

#include "config.h"

// ── Display dimensions ────────────────────────────────────────────────────────
static constexpr int EPD_WIDTH        = 250;
static constexpr int EPD_HEIGHT       = 122;
static constexpr int EPD_ROW_BYTES    = (EPD_WIDTH + 7) / 8; // 32 bytes, 6 padding bits
static constexpr int EPD_BITMAP_BYTES = EPD_ROW_BYTES * EPD_HEIGHT; // 3904 bytes total

// ── Display driver ────────────────────────────────────────────────────────────
// GxEPD2_213_B73 targets the GDEH0213B73 panel (SSD1680, 250×122).
// If your display doesn't init correctly, try GxEPD2_213_B74 instead.
using EpdClass = GxEPD2_213_B73;
GxEPD2_BW<EpdClass, EpdClass::HEIGHT> display(
    EpdClass(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY));

// ── HTTP server ───────────────────────────────────────────────────────────────
WebServer server(80);

// ── Upload state ──────────────────────────────────────────────────────────────
// The multipart upload handler fills this buffer in chunks; handleDisplay()
// renders it once the upload is complete.
static uint8_t bitmap[EPD_BITMAP_BYTES];
static size_t  uploadBytes = 0;
static bool    uploadGood  = false;

// ── Upload handler ────────────────────────────────────────────────────────────
// Called by WebServer for every chunk of the multipart body.
// Runs on the same task as loop(), so we accumulate into a global buffer.
void onBitmapUpload() {
    HTTPUpload& up = server.upload();

    if (up.status == UPLOAD_FILE_START) {
        uploadBytes = 0;
        uploadGood  = false;

    } else if (up.status == UPLOAD_FILE_WRITE) {
        if (up.name != "bitmap") return;
        // Copy this chunk, but never write past the end of the buffer.
        size_t space  = EPD_BITMAP_BYTES - uploadBytes;
        size_t toCopy = min((size_t)up.currentSize, space);
        memcpy(bitmap + uploadBytes, up.buf, toCopy);
        uploadBytes += toCopy;

    } else if (up.status == UPLOAD_FILE_END) {
        if (up.name != "bitmap") return;
        uploadGood = (uploadBytes == EPD_BITMAP_BYTES);
    }
}

// ── POST /display ─────────────────────────────────────────────────────────────
void handleDisplay() {
    if (!uploadGood) {
        server.send(400, "text/plain",
            "Expected " + String(EPD_BITMAP_BYTES) +
            " bytes, received " + String(uploadBytes) + "\n");
        return;
    }

    // Full-window refresh: clear to white, then paint 1-bits black.
    // Rotation 1 = landscape (250 wide × 122 tall).
    // Change display.setRotation() in setup() if content appears rotated.
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 0, bitmap, EPD_WIDTH, EPD_HEIGHT, GxEPD_BLACK);
    } while (display.nextPage());

    server.send(200, "text/plain", "OK\n");
}

// ── GET /clear ────────────────────────────────────────────────────────────────
void handleClear() {
    display.setFullWindow();
    display.firstPage();
    do { display.fillScreen(GxEPD_WHITE); } while (display.nextPage());
    server.send(200, "text/plain", "OK\n");
}

// ── GET / ─────────────────────────────────────────────────────────────────────
void handleRoot() {
    String ip = WiFi.localIP().toString();
    server.send(200, "text/plain",
        "ESP32 e-paper display\n"
        "  " + String(EPD_WIDTH) + "x" + String(EPD_HEIGHT) +
        " px, 1bpp, " + String(EPD_BITMAP_BYTES) + " bytes/frame\n\n"
        "POST /display   curl -F bitmap=@image.bin http://" + ip + "/display\n"
        "GET  /clear     fill white\n");
}

// ── setup ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    // init(baud, initial_reset, reset_ms, pulldown_rst_mode)
    display.init(115200, true, 2, false);
    display.setRotation(1); // 1 = landscape; try 3 if image is upside-down

    // Clear to a known white state on boot.
    display.setFullWindow();
    display.firstPage();
    do { display.fillScreen(GxEPD_WHITE); } while (display.nextPage());

    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    {
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - start >= 20000UL) {
                Serial.println("\nWiFi timeout — restarting");
                ESP.restart();
            }
            delay(500);
            Serial.print('.');
        }
    }
    Serial.println("\nIP: " + WiFi.localIP().toString());

    server.on("/",        HTTP_GET,  handleRoot);
    server.on("/clear",   HTTP_GET,  handleClear);
    server.on("/display", HTTP_POST, handleDisplay, onBitmapUpload);
    server.begin();
    Serial.println("HTTP server started");
}

// ── loop ──────────────────────────────────────────────────────────────────────
void loop() {
    server.handleClient();
}
