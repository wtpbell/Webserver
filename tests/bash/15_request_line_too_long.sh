#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

# kMaxRequestLine = 8000
wire="$(
python3 - <<'PY' | send_raw
path = "/" + ("a" * 9000)
req = f"GET {path} HTTP/1.1\r\nHost: localhost\r\n\r\n"
print(req, end="")
PY
)"

# depending on your design: 414 (URI Too Long) or 400; your validator suggests URITooLong => 414
ASSERT_STATUS 414 "$wire"
