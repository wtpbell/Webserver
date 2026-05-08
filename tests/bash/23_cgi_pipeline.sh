#!/usr/bin/env bash

set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

request=$'GET /cgi-bin/ping.sh HTTP/1.1\r\nHost: localhost\r\n\r\n'
payload="${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}${request}"

wire=$(send_payload "$payload")

ASSERT_N_RESP 16 "$wire"

for i in {1..16}; do
	ASSERT_STATUS_N $i 200 "$wire"
done
