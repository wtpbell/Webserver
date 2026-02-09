#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

# kMaxBodySize = 1024 * 1024
start_server

wire="$(
  { printf "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 1048577\r\n\r\n"; } \
  | nc -q 1 "$HOST" "$PORT" \
  2>/dev/null || true
)"

ASSERT_STATUS 413 "$wire"

