# ============================================================================
#  Root CMake file for an ESP-IDF project.
#
#  This is the top of the build. ESP-IDF is a CMake-based build system: this
#  file pulls in the IDF build machinery, then names the project. The real code
#  lives in components (here, the `main/` component). You rarely edit this file.
# ============================================================================
cmake_minimum_required(VERSION 3.16)

# Pull in ESP-IDF's CMake plumbing. $ENV{IDF_PATH} is set when you run
# `. $HOME/esp/esp-idf/export.sh` (or the `get_idf` alias) — see docs/lesson-01.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# The project name becomes the firmware binary name (tdisplay-idf.bin).
project(tdisplay-idf)
