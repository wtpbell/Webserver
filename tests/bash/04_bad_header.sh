#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

# Space before colon is illegal
start_server
wire="$(send_payload $'GET / HTTP/1.1\r\nHost : a\r\n\r\n')"

ASSERT_STATUS 400 "$wire"
