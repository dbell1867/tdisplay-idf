# Lesson 01 — From Zero, the ESP-IDF Way

**Project:** Bringing up a LilyGO T-Display S3 Pro in **C** on **ESP-IDF**
**Goal of this lesson:** Go from *no toolchain at all* to your own C code running
on the board and logging over USB — understanding the ESP-IDF project model and,
along the way, seeing **what Arduino was doing for you underneath**.

> Audience note: this is the companion to the [C++/Arduino Lesson 01](../../tdisplay/docs/lesson-01-first-light.md).
> It assumes you've done that one, so it teaches by **contrast** — where ESP-IDF
> differs from Arduino, and where plain C differs from the C++ you saw there.

---

## Learning objectives

By the end you can:

1. Explain why "C on the ESP32" means **ESP-IDF**, and how it differs from Arduino.
2. Install ESP-IDF from scratch and activate it in a shell.
3. Describe the ESP-IDF **project model**: CMake, components, `app_main`, `sdkconfig`.
4. Build, flash, and read the console with `idf.py`.
5. Explain `app_main` + FreeRTOS vs Arduino's `setup()`/`loop()`.

---

## Design decisions (made before any code)

### Why ESP-IDF / C

Arduino (the other project) is a friendly **C++ wrapper**. ESP-IDF is the layer
*underneath* it — Espressif's official framework, written in and for **C**, built
on the **FreeRTOS** real-time operating system. Choosing it is choosing to see the
machinery Arduino hides:

| | Arduino (the C++ project) | ESP-IDF (this project) |
|---|---|---|
| Language | C++ (a friendly subset) | C |
| Entry point | `setup()` + `loop()` | `app_main()` (one function) |
| The loop | framework writes it for you | **you** write it (or spawn tasks) |
| OS | hidden | FreeRTOS, front and centre |
| Build | `platformio.ini` | CMake + `sdkconfig` (`menuconfig`) |
| Config | a few `build_flags` | a Kconfig menu of hundreds of options |
| Toolchain | PlatformIO fetches it | you install ESP-IDF yourself |

None of this is harder to *program* — it's the same board — but there's more of
the iceberg above the waterline. That visibility is the point.

### What "first light" means here

Same philosophy as the Arduino lesson: **prove the pipeline before the display.**
This lesson's concrete milestone is **Stage 1 — hello serial**: compile C → flash
→ read a log line back. The display (ST7796 via ESP-IDF's `esp_lcd`) is the next
lesson, once the pipeline is trusted.

---

## Module 1 — Permission to talk to the board

Identical to the Arduino lesson — this is a Linux/USB thing, not a framework thing.

**Find the port** (don't assume it): with the board unplugged, follow the kernel
log, then connect the USB-C cable and watch the device it creates:

```bash
sudo dmesg -W        # follow NEW kernel messages only (skips buffer history); Ctrl-C to stop
# ... cdc_acm 1-1:1.0: ttyACM0: USB ACM device   <- this machine: /dev/ttyACM0
```

Native-USB boards like this one appear as `/dev/ttyACMx`; boards with a separate
serial chip appear as `/dev/ttyUSBx`. (Quick alternative: `ls /dev/ttyACM*` before
vs. after plugging in — the new entry is your board.)

**Surgical option** — `udevadm monitor` watches the device manager and can be
filtered to serial devices only, so `dmesg`'s general chatter is gone:

```bash
udevadm monitor --udev --subsystem-match=tty   # then plug in; only your tty event shows
```

`--udev` reports the event after the `/dev` node exists; add `--property` to also
see the device's `ID_VENDOR_ID` / `ID_SERIAL` identity.

**Grant access** — the device is owned by group `uucp` on Arch, so add yourself:

```bash
sudo usermod -aG uucp $USER   # then log out and back in
getent group uucp             # verify membership
```

If you already did this for the C++ project, you're done — same user, same group.
See the [Arduino Lesson 01, Module 1](../../tdisplay/docs/lesson-01-first-light.md)
for the full explanation of the permissions and the re-login gotcha.

---

## Module 2 — Install ESP-IDF (from scratch)

This is the big difference from PlatformIO: **you install the framework yourself**,
once, into `~/esp/esp-idf`. It's a bigger download (a full GCC cross-toolchain +
FreeRTOS + drivers), but it's the real thing.

### 2.1 — Host prerequisites (Arch / Omarchy)

ESP-IDF brings its own Python tools, but it needs a few system packages to
bootstrap the build:

```bash
sudo pacman -S --needed git cmake ninja ccache dfu-util libusb python
```

(`git` and `python` you already have; `cmake`, `ninja`, `ccache`, `dfu-util` were
missing on this machine.)

### 2.2 — Clone the framework

Clone a **stable release branch** (not `master`) so your toolchain is a known,
supported version. Check <https://github.com/espressif/esp-idf/releases> for the
latest `vX.Y`, then:

```bash
mkdir -p ~/esp
git clone -b release/v5.4 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf
```

- `-b release/v5.4` — pick the newest stable release branch you saw on the
  releases page (this is just an example version).
- `--recursive` — ESP-IDF pulls in many git submodules; this fetches them too.
  If you forget it: `cd ~/esp/esp-idf && git submodule update --init --recursive`.

### 2.3 — Install the toolchain

```bash
cd ~/esp/esp-idf
./install.sh esp32s3
```

This downloads the ESP32-S3 cross-compiler and support tools into `~/.espressif`
and creates a private Python virtualenv (same idea as PlatformIO's `penv` — an
isolated environment so nothing touches your system Python). It does **not** touch
your project yet.

### 2.4 — Activate it in your shell

ESP-IDF works by *exporting environment variables* (`IDF_PATH`, `PATH`, the Python
venv) into your current shell. You do this **once per terminal session**:

```bash
. ~/esp/esp-idf/export.sh
```

Typing that every time is tedious, so add an **alias** to `~/.bashrc`:

```bash
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

Then a new shell just needs `get_idf`. (Espressif deliberately recommends an alias
over auto-sourcing in your profile — `export.sh` is slow enough that running it in
*every* shell would drag down terminals that have nothing to do with ESP32 work.)

Verify:

```bash
get_idf
idf.py --version
```

---

## Module 3 — The project model

Open the four files in this repo. Together they *are* an ESP-IDF project — and
they map onto things you already met in the Arduino project.

### `CMakeLists.txt` (root) — the project declaration

```cmake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tdisplay-idf)
```

ESP-IDF is a **CMake** build system. This root file pulls in IDF's build
machinery (via `$IDF_PATH`, set by `export.sh`) and names the project. You rarely
touch it. *Rough analogy:* it plays the role `platformio.ini`'s top matter did —
"this is a project, here's its name" — but the how-to-build logic lives in ESP-IDF
rather than in a config file you edit.

### `main/` — your first **component**

ESP-IDF organises code into **components**: a folder with sources + a
`CMakeLists.txt` that registers them. Every project has a `main` component — the
one the framework calls into. As this project grows, the display driver, touch,
etc. each become their own component (this is how IDF does the job Arduino's
`lib_deps` did — reusable units of code, but first-class in the build).

`main/CMakeLists.txt`:

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")
```

- `SRCS` — the C files to compile.
- `INCLUDE_DIRS` — folders added to the header search path.

### `sdkconfig.defaults` — configuration (the `build_flags` analogue)

Arduino tuned the build with a handful of `-D` flags. ESP-IDF has **hundreds** of
options, organised in a Kconfig menu you open with `idf.py menuconfig`. Your
choices are written to a generated `sdkconfig` file. Anything in
`sdkconfig.defaults` seeds that file on first generation, so a fresh checkout
builds the same way:

```ini
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y   # console over the native USB-C
CONFIG_ESP_CONSOLE_UART_DEFAULT=n
CONFIG_IDF_TARGET="esp32s3"
```

That first line is the direct descendant of the Arduino project's
`-DARDUINO_USB_MODE=1 / -DARDUINO_USB_CDC_ON_BOOT=1`: it routes the console
(`printf`, `ESP_LOG…`) onto the chip's built-in USB, so the one USB-C cable both
powers the board and carries its logs. (This board has *no* separate serial chip.)

> We commit `sdkconfig.defaults` and **git-ignore** the generated `sdkconfig` —
> the defaults file is the reproducible source of truth.

### `main/main.c` — the code

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "tdisplay";

void app_main(void)
{
    ESP_LOGI(TAG, "=== T-Display S3 Pro is alive! (ESP-IDF / C) ===");

    int counter = 0;
    while (1) {
        ESP_LOGI(TAG, "tick %d  (uptime %lld ms)",
                 ++counter, esp_timer_get_time() / 1000);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

**Read it against the Arduino version:**

- **`app_main()` replaces `setup()` + `loop()`.** ESP-IDF calls `app_main` once,
  on a FreeRTOS task. There is no automatic forever-loop — the `while (1)` is
  *ours*. (In Arduino the framework's hidden `main()` was calling `loop()` in a
  `while` for you; here you see and own that loop.)
- **`ESP_LOGI(TAG, ...)` replaces `Serial.printf`.** It's `printf`-style but adds
  a level (I = info) and a tag, and the monitor colours it. `Serial` doesn't exist
  in C/ESP-IDF — it was an Arduino C++ object.
- **`vTaskDelay(pdMS_TO_TICKS(1000))` replaces `delay(1000)`.** This one matters:
  it doesn't busy-wait, it *yields the CPU back to FreeRTOS* for a second so other
  tasks can run. `pdMS_TO_TICKS` converts milliseconds into the OS's tick units.
- **C, not C++:** no objects or `->`; `static const char *TAG` is a plain C
  string pointer. The `%lld` is because `esp_timer_get_time()` returns a 64-bit
  microsecond count.

> **Editor errors before install are normal.** Until ESP-IDF is installed and
> you've run one build, your editor can't find `freertos/FreeRTOS.h` etc. and will
> underline them. The first `idf.py build` generates a `compile_commands.json`
> that teaches the editor where the headers live.

---

## Module 4 — Stage 1: build, flash, read

With ESP-IDF active (`get_idf`) and you in the project folder:

```bash
idf.py set-target esp32s3   # once: applies sdkconfig.defaults, picks the chip
idf.py build                # compile (first build is SLOW — it builds all of IDF)
idf.py -p /dev/ttyACM0 flash monitor   # flash, then open the console
```

- **`set-target`** generates `sdkconfig` from your defaults and locks the chip.
  Run it once per project (or after `fullclean`).
- **First `build` is minutes**, like PlatformIO's first run — it compiles the
  whole framework once, then caches it. Later builds are fast.
- **`flash monitor`** chains two steps: flash the binary, then attach the serial
  console. `-p /dev/ttyACM0` names the port (drop it and IDF auto-detects).
- **Quit the monitor with `Ctrl-]`** (not Ctrl-C).

**Success looks like** boot ROM output, then:

```
I (312) tdisplay: === T-Display S3 Pro is alive! (ESP-IDF / C) ===
I (1312) tdisplay: tick 1  (uptime 1312 ms)
I (2312) tdisplay: tick 2  (uptime 2312 ms)
```

The `I (…) tdisplay:` prefix is `ESP_LOGI` at work: level, timestamp (ms), tag.

> **Flashing gotcha.** If `flash` can't sync, put the board in download mode: hold
> **BOOT**, tap **RESET**, release **BOOT**, then re-run. Usually the auto-reset
> handles it and you won't need to.

---

## What you built and learned

- ✅ ESP-IDF installed from scratch (`~/esp/esp-idf`) and activated with `get_idf`
- ✅ The ESP-IDF project model: root CMake, the `main` component, `sdkconfig`
- ✅ `app_main` + FreeRTOS `vTaskDelay` vs Arduino `setup()`/`loop()`/`delay()`
- ✅ Console over native USB-Serial/JTAG (the ESP-IDF take on the Arduino USB flags)
- ✅ The `idf.py` build → flash → monitor loop

## Command cheat-sheet

```bash
get_idf                                 # activate ESP-IDF in this shell
idf.py set-target esp32s3               # once per project
idf.py build                            # compile
idf.py -p /dev/ttyACM0 flash monitor    # flash + console (Ctrl-] to quit)
idf.py menuconfig                        # open the Kconfig configuration menu
idf.py fullclean                         # wipe build/ (needed after some config changes)
```

## Glossary

- **ESP-IDF** — Espressif's official C framework (FreeRTOS + drivers + CMake build).
- **`idf.py`** — the command-line front end that wraps CMake/ninja/flashing.
- **Component** — ESP-IDF's unit of reusable code (a folder + `CMakeLists.txt`).
- **`app_main`** — your entry point; runs once on a FreeRTOS task.
- **FreeRTOS** — the real-time OS underneath everything; schedules tasks.
- **`sdkconfig` / `menuconfig`** — the generated build configuration and its menu.
- **USB-Serial/JTAG** — the ESP32-S3's built-in USB console peripheral (no chip).
- **`export.sh`** — the script that activates ESP-IDF in a shell (via `get_idf`).

## Next lesson

**Stage 2 — First light (the display):** drive the ST7796 over SPI using ESP-IDF's
`esp_lcd` component — the lower-level counterpart to Arduino_GFX — and put the
first pixels on the glass.
