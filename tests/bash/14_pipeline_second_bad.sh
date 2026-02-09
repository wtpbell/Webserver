#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

payload=$'GET /a HTTP/1.1\r\nHost: localhost\r\n\r\nGET /b HTTP/1.1\r\nHosts: localhost\r\n\r\n'

start_server
wire="$(send_raw "$payload")"

ASSERT_N_RESP 2 "$wire"

# First status line:
first="$(printf "%s" "$wire" | awk '/^HTTP\/1\.[01] /{print; exit}')"

# Second status line (line number):
line2="$(printf "%s" "$wire" | grep -nE '^HTTP/1\.[01] ' | sed -n '2p' | cut -d: -f1)"
second=""
if [[ -n "$line2" ]]; then
  second="$(printf "%s" "$wire" | tail -n +"$line2")"
fi

ASSERT_STATUS 200 "$first"
ASSERT_STATUS 400 "$second"

stop_server
