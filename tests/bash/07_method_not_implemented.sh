#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server
wire="$(send_payload $'BREW / HTTP/1.1\r\nHost: a\r\n\r\n')"

ASSERT_STATUS 501 "$wire"
