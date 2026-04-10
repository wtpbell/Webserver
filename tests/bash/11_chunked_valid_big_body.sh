#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

wire="$(
  ${TIMEOUT_BIN:+$TIMEOUT_BIN 3s} bash -c '
    (
      printf "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n"
      printf "100000\r\n"
      head -c 1048576 < /dev/zero
      printf "\r\n0\r\n\r\n"
    ) | nc -q 1 -w 2 '"${HOST}"' '"${PORT}"' 2>/dev/null
  ' || true
)"

ASSERT_STATUS 400 "$wire"

stop_server
