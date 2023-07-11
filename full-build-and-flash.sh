#!/usr/bin/env bash

cd -- "$(dirname "$0")"
set -x
west build -p auto -b "${1:-stamp_c3}" --sysbuild app
west flash

# west build -p auto -b "${1:-stamp_c3}" -t flash app