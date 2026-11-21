$ErrorActionPreference = 'Stop'

$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
# Official espressif/idf:v5.5.2, Linux amd64; no Windows toolchain is used.
$image = 'espressif/idf@sha256:88a65f412e730fdbe134cd7427dd840eb1a6923479499c9c233699f804a3096b'

& docker run --rm --platform linux/amd64 `
    --mount "type=bind,source=$projectRoot,target=/project" `
    --workdir /project `
    --env IDF_GIT_SAFE_DIR=/project `
    $image bash tools/ci/build-firmware.sh

if ($LASTEXITCODE -ne 0) {
    throw "Docker firmware build failed (exit code $LASTEXITCODE)."
}

Write-Host "Images: $(Join-Path $projectRoot 'build\olimex-esp32-evb-revk')"
