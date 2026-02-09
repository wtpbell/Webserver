#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server
wire="$(send_payload $'GET / HTTP/1.5\r\nHost: a\r\n\r\n')"

ASSERT_STATUS 505 "$wire"
