#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server
wire="$(send_payload $'POST / HTTP/1.1\r\nHost: a\r\nTransfer-Encoding: chunked\r\nContent-Length: 5\r\n\r\nhello')"

ASSERT_STATUS 400 "$wire"
