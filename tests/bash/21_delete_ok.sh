#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

WEBROOT="${WEBROOT:-$ROOT_DIR/www}"
mkdir -p "$WEBROOT/t_del"
printf "bye\n" > "$WEBROOT/t_del/file.txt"
mkdir -p "$WEBROOT/t_del/dir"

echo "[21] DELETE existing file => 204 and file removed"
wire="$(
python3 - <<'PY' | send_raw
request = (
  "DELETE /t_del/file.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(request, end="")
PY
)"
ASSERT_STATUS 204 "$wire"
[[ ! -e "$WEBROOT/t_del/file.txt" ]] || die "expected file.txt to be removed"

echo "[21] DELETE missing => 404"
wire="$(
python3 - <<'PY' | send_raw
request = (
  "DELETE /t_del/missing.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(request, end="")
PY
)"
ASSERT_STATUS 404 "$wire"

echo "[21] DELETE directory => 403"
wire="$(
python3 - <<'PY' | send_raw
request = (
  "DELETE /t_del/dir HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(request, end="")
PY
)"
ASSERT_STATUS 403 "$wire"
