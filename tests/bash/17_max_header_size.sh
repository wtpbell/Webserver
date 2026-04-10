#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

# kMaxHeaderSize = 8192
# header-too-large to PayloadTooLarge => 413

EXPECTED=413

wire="$(
python3 - <<'PY' | send_raw
# Make a huge header value so the *header section* exceeds kMaxHeaderSize.
# Use 9000 > 8192.
big = "a" * 9000

request = (
  "GET / HTTP/1.1\r\n"
  "Host: localhost\r\n"
  f"X-Big: {big}\r\n"
  "\r\n"
)
print(request, end="")
PY
)"

ASSERT_STATUS "$EXPECTED" "$wire"
