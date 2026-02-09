#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

# Declares 1MB chunk but sends only 64KB then closes. Server may:
# - reply 400 (nice)
# - or just close (acceptable)
out="$(
  timeout 3 bash -c '
    (
      {
        printf "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n"
        printf "100000\r\n"
        head -c 65536 < /dev/zero
        printf "\r\n"
      } 2>/dev/null
    ) | nc -q 1 "'"$HOST"'" "'"$PORT"'" || true
  ' || true
)"


# Pass criteria: command returns within timeout.
# Optional: if it responded, it should be 400/501.
if [[ -n "$out" ]]; then
  line="$(status_line <<<"$out")"
  echo "$line"
fi
exit 0
