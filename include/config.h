// config.h — WiFi credentials and hardware pin assignments.
// Keep this file out of version control if your repo is public.

#pragma once

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_SSID     "your-network-name"
#define WIFI_PASSWORD "your-network-password"

// ── SPI & control pins ────────────────────────────────────────────────────────
// SCK and MOSI use the ESP32's VSPI hardware SPI bus defaults.
// CS, DC, RST, and BUSY are software-controlled; any free GPIO works.
//
//  Display label │ Function              │ ESP32 GPIO
//  ──────────────┼───────────────────────┼──────────────────────────────
//  DIN           │ MOSI  (SPI data out)  │ 23   ← VSPI MOSI (fixed)
//  CLK           │ SCK   (SPI clock)     │ 18   ← VSPI CLK  (fixed)
//  CS            │ Chip select (active ↓)│  5   ← change freely
//  DC            │ Data / Command select │ 17   ← change freely
//  RST           │ Reset (active ↓)      │ 16   ← change freely
//  BUSY          │ Busy signal (active ↑)│  4   ← change freely
//  VCC           │ Power                 │ 3.3V
//  GND           │ Ground                │ GND

#define PIN_EPD_CS    5
#define PIN_EPD_DC   17
#define PIN_EPD_RST  16
#define PIN_EPD_BUSY  4
