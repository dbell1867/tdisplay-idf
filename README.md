# T-Display S3 Pro — the ESP-IDF (C) edition

A companion to the [C++/Arduino bring-up](../tdisplay) of the **LilyGO T-Display
S3 Pro**, redone at a lower level: **pure C on Espressif's native ESP-IDF
framework**. Same board, same curriculum, but this time nothing is hidden — no
`setup()`/`loop()`, no Arduino wrapper. You meet FreeRTOS, components, and
`menuconfig` directly.

The goal is understanding, not just working code — and specifically to see **what
Arduino was doing for you** underneath. Where useful, each step is contrasted with
the Arduino version.

---

## The board

**LilyGO T-Display S3 Pro**

- **MCU:** ESP32-S3R8 — dual-core LX7, WiFi + BLE, **16 MB flash, 8 MB PSRAM**
- **Display:** 2.33" IPS TFT, **222 × 480**, **ST7796** driver (SPI)
- **Touch:** capacitive **CST226SE** (I²C) — later lesson
- **USB:** native USB-C (no separate serial chip) → console rides the built-in
  **USB-Serial/JTAG**

### Pin map (from LilyGO's `utilities.h`)

| Function | GPIO | Function | GPIO |
|---|---|---|---|
| SPI SCLK | 18 | TFT CS | 39 |
| SPI MOSI | 17 | TFT DC | 9 |
| SPI MISO | 8  | TFT RST | 47 |
| I²C SDA  | 5  | TFT backlight | 48 |
| I²C SCL  | 6  | Touch RST | 13 |
| Touch IRQ | 21 | | |

---

## Current state

| Stage | Status | What it does |
|---|---|---|
| 1 — Hello serial | 🚧 scaffolded | `app_main` logs a boot line + a 1 Hz tick over USB-Serial/JTAG |

**The firmware in `main/main.c` right now** is the Stage 1 serial proof: log a
boot banner, then a `tick N (uptime …)` line once per second. It builds once
ESP-IDF is installed — see the lesson below.

---

## Repository layout

```
CMakeLists.txt        root ESP-IDF project file (names the project)
sdkconfig.defaults    build defaults (USB-Serial/JTAG console, target = esp32s3)
main/
  CMakeLists.txt      registers the `main` component
  main.c              app_main() — the Stage 1 hello-serial code
docs/
  lesson-01-first-light.md   install ESP-IDF from scratch → project model → serial
```

`sdkconfig` (the generated config) is intentionally **not** committed; the
reproducible bits live in `sdkconfig.defaults`.

---

## Toolchain

- **[ESP-IDF](https://docs.espressif.com/projects/esp-idf/)** — Espressif's
  official C framework (FreeRTOS + drivers + CMake build), driven by `idf.py`.
- Installed from source under `~/esp/esp-idf` (see the lesson). No PlatformIO
  here — this is the native toolchain on purpose.

---

## Build & flash (after installing ESP-IDF — see the lesson)

```bash
get_idf                       # activate ESP-IDF in this shell (alias set up in the lesson)
idf.py set-target esp32s3     # once per project
idf.py build                  # compile
idf.py -p /dev/ttyACM0 flash monitor   # flash + open the console (Ctrl-] to quit)
```

**Find the port:** don't assume `/dev/ttyACM0` — with the board unplugged, run
`sudo dmesg -W` (follow new kernel messages only), then connect the USB-C cable and
read the device the kernel creates (native-USB boards like this appear as
`/dev/ttyACMx`). Drop the `-p` flag and `idf.py` auto-detects. See
[Lesson 01, Module 1](docs/lesson-01-first-light.md) for detail.

---

## Lessons

- **[Lesson 01 — From Zero, the ESP-IDF Way](docs/lesson-01-first-light.md):**
  installing ESP-IDF from scratch, the CMake/component project model, `app_main`
  vs `setup()`/`loop()`, FreeRTOS delays, and the USB-Serial/JTAG console — the
  same "prove the pipeline before the display" milestone as the Arduino Lesson 1.
