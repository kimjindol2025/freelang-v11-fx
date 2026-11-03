#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to the native compiler}"
exec env FX_NATIVE_COMPILER="$compiler" bash "$root/tests/native-bootstrap/raw-tcp-review-probes.sh"
