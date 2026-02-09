#!/usr/bin/env bash
set -euo pipefail

DIR="$(cd "$(dirname "$0")" && pwd)"

fail=0
for t in "$DIR"/[0-9][0-9]_*.sh; do
  echo "===== $(basename "$t") ====="
  if bash "$t"; then
    echo "[OK] $(basename "$t")"
  else
    echo "[FAIL] $(basename "$t")"
    fail=1
  fi
  echo
done

exit $fail
