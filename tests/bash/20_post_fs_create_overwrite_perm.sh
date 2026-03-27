#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

WEBROOT="${WEBROOT:-$ROOT_DIR/www}"
mkdir -p "$WEBROOT/t_fs"
mkdir -p "$WEBROOT/t_fs/dir"
mkdir -p "$WEBROOT/t_fs/nowrite"
chmod 555 "$WEBROOT/t_fs/nowrite" || true

rm -f "$WEBROOT/t_fs/upload.txt"

echo "[20] POST create new file => 201"
wire="$(
python3 - <<'PY' | send_raw
body = "abc"
req = (
  "POST /t_fs/upload.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  f"Content-Length: {len(body)}\r\n"
  "Connection: close\r\n"
  "\r\n"
  + body
)
print(req, end="")
PY
)"
ASSERT_STATUS 201 "$wire"
grep -Fq "abc" "$WEBROOT/t_fs/upload.txt" || die "upload.txt content mismatch"

echo "[20] POST overwrite existing => 204"
wire="$(
python3 - <<'PY' | send_raw
body = "zzz"
req = (
  "POST /t_fs/upload.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  f"Content-Length: {len(body)}\r\n"
  "Connection: close\r\n"
  "\r\n"
  + body
)
print(req, end="")
PY
)"
ASSERT_STATUS 204 "$wire"
grep -Fq "zzz" "$WEBROOT/t_fs/upload.txt" || die "upload.txt overwrite mismatch"

echo "[20] POST to directory path => 400"
wire="$(
python3 - <<'PY' | send_raw
body = "x"
req = (
  "POST /t_fs/dir HTTP/1.1\r\n"
  "Host: localhost\r\n"
  f"Content-Length: {len(body)}\r\n"
  "Connection: close\r\n"
  "\r\n"
  + body
)
print(req, end="")
PY
)"
ASSERT_STATUS 400 "$wire"

echo "[20] POST parent missing => 404"
wire="$(
python3 - <<'PY' | send_raw
body = "x"
req = (
  "POST /t_fs/no_such_dir/file.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  f"Content-Length: {len(body)}\r\n"
  "Connection: close\r\n"
  "\r\n"
  + body
)
print(req, end="")
PY
)"
ASSERT_STATUS 404 "$wire"

echo "[20] POST permission denied (parent not writable) => 403 (may warn if running as root)"
wire="$(
python3 - <<'PY' | send_raw
body = "x"
req = (
  "POST /t_fs/nowrite/file.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  f"Content-Length: {len(body)}\r\n"
  "Connection: close\r\n"
  "\r\n"
  + body
)
print(req, end="")
PY
)"

code="$(status_code "$wire")"
if [[ "$code" != "403" ]]; then
  echo "[WARN] expected 403, got $code (permission tests may not work as root)"
else
  echo "[OK] 403 as expected"
fi
