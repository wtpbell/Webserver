#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

WEBROOT="${WEBROOT:-$ROOT_DIR/www}"
TESTDIR="$WEBROOT/t_fs"
FIFO="$TESTDIR/notafile"
SECRET="$TESTDIR/secret.txt"

mkdir -p "$TESTDIR"
chmod 700 "$TESTDIR" 2>/dev/null || true

# Cleanup from previous runs.
# secret.txt may have been left at 000, so try to relax perms first.
chmod 600 "$SECRET" 2>/dev/null || true
rm -f "$FIFO" "$SECRET" 2>/dev/null || true

# Not-a-regular-file case: FIFO
mkfifo "$FIFO"

# Permission denied file (read)
printf "secret\n" > "$SECRET"
chmod 000 "$SECRET" || true

echo "[19] GET not-a-file (fifo) => 403"
wire="$(
python3 - <<'PY' | send_raw
request = (
  "GET /t_fs/notafile HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(request, end="")
PY
)"
ASSERT_STATUS 403 "$wire"

echo "[19] GET permission denied => 403 (may warn if running as root)"
wire="$(
python3 - <<'PY' | send_raw
request = (
  "GET /t_fs/secret.txt HTTP/1.1\r\n"
  "Host: localhost\r\n"
  "Connection: close\r\n"
  "\r\n"
)
print(request, end="")
PY
)"

code="$(status_code "$wire")"
if [[ "$code" != "403" ]]; then
  echo "[WARN] expected 403, got $code (permission tests may not work as root)"
else
  echo "[OK] 403 as expected"
fi

# Best-effort cleanup so reruns do not fail
chmod 600 "$SECRET" 2>/dev/null || true
rm -f "$FIFO" "$SECRET" 2>/dev/null || true
