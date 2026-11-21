# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Keep generated board dependencies separate from the upstream C3 lock file.
if(WICAN_DEPENDENCIES_LOCK)
    idf_build_set_property(DEPENDENCIES_LOCK "${WICAN_DEPENDENCIES_LOCK}")
endif()

set(WICAN_BOARD "" CACHE STRING "Board profile (empty keeps the upstream hardware selection)")

set(WICAN_V210 1)
set(WICAN_V300 2)
set(WICAN_USB_V100 3)
set(WICAN_PRO 4)
set(OLIMEX_ESP32_EVB_REVK 5)

set(WICAN_V210_STR "OBD")
set(WICAN_V300_STR "OBD")
set(WICAN_USB_V100_STR "USB")
set(WICAN_PRO_STR "OBD-PRO")
set(OLIMEX_ESP32_EVB_REVK_STR "EVB-K")

# Set these without quotes, as they're string literals
set(WICAN_V210_PREF "obd")
set(WICAN_V300_PREF "obd")
set(WICAN_USB_V100_PREF "usb")
set(WICAN_PRO_PREF "obd_pro")
set(OLIMEX_ESP32_EVB_REVK_PREF "evb_k")

add_definitions(-DWICAN_V210=${WICAN_V210})
add_definitions(-DWICAN_V300=${WICAN_V300})
add_definitions(-DWICAN_USB_V100=${WICAN_USB_V100})
add_definitions(-DWICAN_PRO=${WICAN_PRO})
add_definitions(-DOLIMEX_ESP32_EVB_REVK=${OLIMEX_ESP32_EVB_REVK})

add_definitions(-DWICAN_V210_STR="${WICAN_V210_STR}")
add_definitions(-DWICAN_V300_STR="${WICAN_V300_STR}")
add_definitions(-DWICAN_USB_V100_STR="${WICAN_USB_V100_STR}")
add_definitions(-DWICAN_PRO_STR="${WICAN_PRO_STR}")

execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(GIT_SHA1 "${GIT_SHA}")
string(REPLACE "." "" GIT_SHA "${GIT_SHA}")

add_definitions(-DGIT_SHA="${GIT_SHA1}")

# Existing numeric HARDWARE_VER overrides and the V300 default remain supported.
if(WICAN_BOARD STREQUAL "olimex-esp32-evb-revk")
    set(HARDWARE_VER ${OLIMEX_ESP32_EVB_REVK})
elseif(NOT WICAN_BOARD STREQUAL "")
    message(FATAL_ERROR "Unknown WICAN_BOARD: ${WICAN_BOARD}")
elseif(NOT DEFINED HARDWARE_VER)
    set(HARDWARE_VER ${WICAN_V300})
endif()

if(HARDWARE_VER EQUAL ${OLIMEX_ESP32_EVB_REVK})
    if(DEFINED IDF_TARGET AND NOT IDF_TARGET STREQUAL "esp32")
        message(FATAL_ERROR "Olimex ESP32-EVB Rev K requires IDF_TARGET=esp32")
    endif()
    if(DEFINED ENV{IDF_TARGET} AND NOT "$ENV{IDF_TARGET}" STREQUAL "esp32")
        message(FATAL_ERROR "Olimex ESP32-EVB Rev K requires IDF_TARGET=esp32")
    endif()
    set(IDF_TARGET "esp32" CACHE STRING "ESP-IDF target")
endif()

add_definitions(-DHARDWARE_VER=${HARDWARE_VER})

set(PROJECT_PREF "wican-fw")

# Set HW_PREF based on HARDWARE_VER
if (HARDWARE_VER EQUAL ${OLIMEX_ESP32_EVB_REVK})
    set(HW_PREF "${OLIMEX_ESP32_EVB_REVK_PREF}")
    set(HW_VER "${OLIMEX_ESP32_EVB_REVK_STR}")
elseif (HARDWARE_VER EQUAL ${WICAN_PRO})
    set(HW_PREF "${WICAN_PRO_PREF}")
    set(HW_VER "${WICAN_PRO_STR}")
elseif (HARDWARE_VER EQUAL ${WICAN_USB_V100})
    set(HW_PREF "${WICAN_USB_V100_PREF}")
    set(HW_VER "${WICAN_USB_V100_STR}")
elseif (HARDWARE_VER EQUAL ${WICAN_V210})
    set(HW_PREF "${WICAN_V210_PREF}")
    set(HW_VER "${WICAN_V210_STR}")
else()
    set(HW_PREF "${WICAN_V300_PREF}")
    set(HW_VER "${WICAN_V300_STR}")
endif()

add_definitions(-DHARDWARE_VERSION="${HW_VER}")

# Form the project binary name
set(PRJ_BIN "${PROJECT_PREF}_${HW_PREF}_${GIT_SHA}")
project("${PRJ_BIN}")

if(HARDWARE_VER EQUAL ${OLIMEX_ESP32_EVB_REVK} AND NOT IDF_TARGET STREQUAL "esp32")
    message(FATAL_ERROR "Olimex ESP32-EVB Rev K requires IDF_TARGET=esp32")
endif()
