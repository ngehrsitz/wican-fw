#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
B="$ROOT/build/olimex-esp32-evb-revk"
cd "$ROOT"

# The checkout may be owned by the runner's UID rather than the container's.
# This is scoped to the mounted repository and the disposable container.
git config --global --add safe.directory "$ROOT"

# GitHub job containers do not necessarily run the image's entrypoint.
source /opt/esp/idf/export.sh
idf.py --version

mkdir -p "$B"
if [[ ! -e "$B/dependencies.lock" ]]; then
    cp "$ROOT/dependencies.lock" "$B/dependencies.lock"
fi

idf.py -B "$B" \
    -DWICAN_BOARD=olimex-esp32-evb-revk \
    -DIDF_TARGET=esp32 \
    -DSDKCONFIG="$B/sdkconfig" \
    -DSDKCONFIG_DEFAULTS="$ROOT/boards/olimex-esp32-evb-revk/sdkconfig.defaults" \
    -DWICAN_DEPENDENCIES_LOCK="$B/dependencies.lock" \
    build

# Check the generated flash plan, not the checked-in ESP32-C3 configuration.
python - "$B" <<'PY'
import json
import shutil
import sys
from pathlib import Path

build = Path(sys.argv[1])
args = json.loads((build / "flasher_args.json").read_text())
if args["extra_esptool_args"]["chip"] != "esp32":
    raise SystemExit("Expected an ESP32 build")
if args["flash_settings"]["flash_size"] != "4MB":
    raise SystemExit("Expected 4MB flash metadata")

expected = {"bootloader": 0x1000, "partition-table": 0x8000,
            "otadata": 0xD000, "app": 0x10000}
for name, offset in expected.items():
    image = args[name]
    if int(image["offset"], 0) != offset:
        raise SystemExit(f"Unexpected {name} offset: {image['offset']}")
    path = build / image["file"]
    if not path.is_file() or path.stat().st_size == 0:
        raise SystemExit(f"Missing or empty image: {path}")
    if args["flash_files"].get(image["offset"]) != image["file"]:
        raise SystemExit(f"Image absent from flash plan: {name}")

shutil.copyfile(build / args["app"]["file"], build / "app-ota.bin")
PY

# An absolute output path avoids interpreting it relative to the build folder.
idf.py -B "$B" merge-bin \
    --format raw --flash-offset 0x0 --fill-flash-size 4MB \
    --output "$B/factory.bin"

python - "$B/factory.bin" <<'PY'
import sys
from pathlib import Path

if Path(sys.argv[1]).stat().st_size != 4 * 1024 * 1024:
    raise SystemExit("Factory image must be exactly 4 MiB")
PY

printf 'Factory image (flash at 0x0): %s\nOTA app image: %s\n' \
    "$B/factory.bin" "$B/app-ota.bin"
