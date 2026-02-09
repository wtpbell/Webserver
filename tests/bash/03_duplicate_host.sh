#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

wire="$(send_payload "GET / HTTP/1.1\r\nHost: a\r\nHost: b\r\n\r\n")"
ASSERT_STATUS 400 "$wire"
