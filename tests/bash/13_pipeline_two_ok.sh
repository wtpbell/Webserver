#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

payload=$'GET /a HTTP/1.1\r\nHost: localhost\r\n\r\nGET /b HTTP/1.1\r\nHost: localhost\r\n\r\n'
wire="$(send_payload "$payload")"

ASSERT_N_RESP 2 "$wire"
# You can also check both are 200:
# (simple check: two "200" status lines)
count_200="$(grep -c '^HTTP/1\.1 200' <<<"$wire" || true)"
[[ "$count_200" -eq 2 ]] || { echo "Expected two 200 responses"; echo "$wire"; exit 1; }
