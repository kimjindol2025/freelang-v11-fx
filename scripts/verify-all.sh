#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
mode="${1:-all}"
if [[ $# -gt 0 ]]; then shift; fi
result=0

run_child() {
  local script="$1"
  shift
  if FREELANG_VERIFY_ALL_CHILD=1 bash "$script" "$@"; then
    return 0
  else
    result=1
  fi
}

case "$mode" in
  all)
    [[ $# == 0 ]] || { echo 'usage: verify-all.sh [--fixpoint [CGC_MAIN] | --bootstrap]' >&2; exit 2; }
    run_child "$ROOT_DIR/verify-fixpoint.sh"
    run_child "$ROOT_DIR/scripts/verify-native-bootstrap.sh"
    ;;
  --fixpoint)
    run_child "$ROOT_DIR/verify-fixpoint.sh" "$@"
    ;;
  --bootstrap)
    run_child "$ROOT_DIR/scripts/verify-native-bootstrap.sh"
    ;;
  *)
    echo 'usage: verify-all.sh [--fixpoint [CGC_MAIN] | --bootstrap]' >&2
    exit 2
    ;;
esac

exit "$result"
