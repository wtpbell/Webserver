#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server
wire="$(send_payload $'GARBAGE\r\n\r\n')"

ASSERT_STATUS 400 "$wire"
