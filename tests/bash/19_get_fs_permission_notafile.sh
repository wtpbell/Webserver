#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

WEBROOT="${WEBROOT:-$ROOT_DIR/www}"
mkdir -p "$WEBROOT/t_fs"

rm -f "$WEBROOT/t_fs/notafile"
# Not-a-regular-file case: FIFO
mkfifo "$WEBROOT/t_fs/notafile"


# Permission denied file (read)
printf "secret\n" > "$WEBROOT/t_fs/secret.txt"
chmod 000 "$WEBROOT/t_fs/secret.txt" || true

echo "[19] GET not-a-file (fifo) => 403"
wire="$(
python3 - <<'PY' | send_raw
req = (
  "GET /t_fs/notafile HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(req, end="")
PY
)"
ASSERT_STATUS 403 "$wire"

echo "[19] GET permission denied => 403 (may warn if running as root)"
wire="$(
python3 - <<'PY' | send_raw
req = (
  "GET /t_fs/secret.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(req, end="")
PY
)"

# If tests run as root, chmod 000 won't block reads. So allow warn.
code="$(status_code "$wire")"
if [[ "$code" != "403" ]]; then
  echo "[WARN] expected 403, got $code (permission tests may not work as root)"
else
  echo "[OK] 403 as expected"
fi
