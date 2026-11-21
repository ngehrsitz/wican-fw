# Olimex ESP32-EVB Rev K: build and flash images

This is an experimental port of the original WiCAN firmware to the classic
ESP32 on the Olimex ESP32-EVB Rev K. It is not WiCAN PRO firmware. SD-card
support is excluded. Building successfully does not establish that the board,
Ethernet, CAN, or OTA updates have been tested on hardware.

The existing web UI, configuration, and protocol behavior are retained. Native
SLCAN still allows transmission: this port does not add a listen-only policy or
a transmit interlock. Start with CAN disconnected and use an isolated test bus
before connecting other equipment.

## Build on Windows with Docker

Use Docker Desktop with Linux containers enabled. Run from this repository:

```powershell
.\tools\build-docker.ps1
```

The wrapper mounts the repository as `/project`, including paths with spaces,
and runs `tools/ci/build-firmware.sh`. It uses the official ESP-IDF **5.5.2**
Linux amd64 image pinned to:

```text
espressif/idf@sha256:88a65f412e730fdbe134cd7427dd840eb1a6923479499c9c233699f804a3096b
```

No Windows ESP-IDF toolchain or USB access inside Docker is needed for building.
The first run downloads the image and managed components, so it needs Internet
access. The wrapper does not flash the board.

For a Linux host, the equivalent command from the repository root is:

```bash
docker run --rm --platform linux/amd64 \
  --mount "type=bind,source=$(pwd),target=/project" \
  --workdir /project --env IDF_GIT_SAFE_DIR=/project \
  espressif/idf@sha256:88a65f412e730fdbe134cd7427dd840eb1a6923479499c9c233699f804a3096b \
  bash tools/ci/build-firmware.sh
```

The build selects `WICAN_BOARD=olimex-esp32-evb-revk` and `IDF_TARGET=esp32`,
using `boards/olimex-esp32-evb-revk/sdkconfig.defaults` and the existing
`wican_partitions_table.csv`. Its generated configuration, copied/resolved
dependency lock, and outputs stay under `build/olimex-esp32-evb-revk/`.
The tracked root ESP32-C3 `sdkconfig` and `dependencies.lock` are not overwritten.
Other existing build directories are not cleaned or changed.

## Outputs and checks

| File | Intended use |
| --- | --- |
| `build/olimex-esp32-evb-revk/factory.bin` | Full 4 MiB factory image, written at **`0x0`** over USB. |
| `build/olimex-esp32-evb-revk/app-ota.bin` | App-only image for the existing firmware upload UI on this same board/partition layout. |
| `build/olimex-esp32-evb-revk/flasher_args.json` | Generated offsets/settings for inspection; not a standalone flashing package. |

ESP-IDF checks the app against the OTA partition capacity during the build.
The script also checks the generated ESP32/4MB metadata, bootloader, partition
table, OTA-data and app entries, then checks the merged file is exactly
4,194,304 bytes. It uses IDF's built-in `merge-bin`, not a custom merger.
The ESP32 bootloader is inside the factory image at `0x1000`; the **merged file
itself must be written at `0x0`**.

There is no separate web-UI filesystem image: `homepage_full.html` is embedded
in the app. Internal storage is left blank in the factory image and initialized
as LittleFS by the firmware on first boot. No SD card or preloaded filesystem
image is required.

## GitHub Actions

`.github/workflows/firmware-build.yml` uses the same build script and pinned
container. It runs on relevant pushes to `olimex-esp32-evb-revk` and supports
`workflow_dispatch`. It only uploads the three files above as an artifact; it
does not commit, push, publish releases, deploy pages, or flash hardware.
Existing workflows are unchanged.

Check that Actions is enabled and the workflow is registered in the fork before
expecting a run. A missing workflow listing alone does not identify a particular
disabled setting. GitHub's manual **Run workflow** button normally requires the
workflow file to exist on the default branch; a relevant push to the port branch
is the initial trigger when the file is only on that branch.

## First USB flash: destructive

Keep **CAN disconnected**. Confirm the connected device is a classic ESP32 and
that its detected flash is **4MB** before writing this image. Do not flash these
images to an ESP32-C3 WiCAN or a WiCAN PRO.

Use the standalone esptool USB flasher on Windows; installing the ESP-IDF
toolchain is not necessary. Replace `COM5` with the board's actual USB serial
port. First inspect the flash (the commands below use the esptool 4.x spelling;
newer esptool versions also accept these aliases):

```powershell
python -m esptool --chip esp32 --port COM5 flash_id
```

**Writing `factory.bin` erases the previous configuration, internal files, and
OTA state.** Only after checking the device/flash size and accepting that loss:

```powershell
python -m esptool --chip esp32 --port COM5 write_flash --flash_size 4MB 0x0 build/olimex-esp32-evb-revk/factory.bin
```

These commands are instructions, not actions performed by the build script or
CI. USB recovery should remain available while testing the first boot. Verify
boot logs, internal-storage initialization, Ethernet DHCP and the web UI before
testing CAN. Hardware behavior and OTA/rollback still need bench validation.

For later updates on the **same Olimex port and partition layout**, upload only
`app-ota.bin` through the existing web UI. App-only OTA does not write the
partition table, OTA-data initialization image, NVS, or storage; it leaves the
existing configuration/files in place. Do not upload the merged factory image
through the OTA UI. Legacy SPIFFS-to-LittleFS initialization can reset stored
files, so back up settings before changing from older filesystem formats.
