#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

WEBROOT="${WEBROOT:-$ROOT_DIR/www}"

mkdir -p "$WEBROOT/t_fs/dir"
printf "hello\n" > "$WEBROOT/t_fs/hello.txt"
printf "<h1>index</h1>\n" > "$WEBROOT/t_fs/dir/index.html"

echo "[18] GET existing file => 200"
wire="$(
python3 - <<'PY' | send_raw
req = (
  "GET /t_fs/hello.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(req, end="")
PY
)"
ASSERT_STATUS 200 "$wire"

echo "[18] GET missing => 404"
wire="$(
python3 - <<'PY' | send_raw
req = (
  "GET /t_fs/missing.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(req, end="")
PY
)"
ASSERT_STATUS 404 "$wire"

echo "[18] GET dir without trailing slash => 301"
wire="$(
python3 - <<'PY' | send_raw
req = (
  "GET /t_fs/dir HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(req, end="")
PY
)"
ASSERT_STATUS 301 "$wire"
