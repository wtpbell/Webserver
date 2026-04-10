#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

WEBROOT="${WEBROOT:-$ROOT_DIR/www}"
mkdir -p "$WEBROOT/t_del_perm/nowrite"

# Create the file first, then remove write permission on the parent directory.
printf "cant delete me\n" > "$WEBROOT/t_del_perm/nowrite/file.txt"
chmod 555 "$WEBROOT/t_del_perm/nowrite" || true

echo "[22] DELETE permission denied (parent not writable) => 403 (may warn if root)"
wire="$(
python3 - <<'PY' | send_raw
request = (
  "DELETE /t_del_perm/nowrite/file.txt HTTP/1.1\r\n"
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

# If not root, file should still exist.
if [[ "$code" == "403" ]]; then
  [[ -e "$WEBROOT/t_del_perm/nowrite/file.txt" ]] || die "file.txt should still exist after forbidden delete"
fi

# Cleanup (restore perms first, then remove)
chmod 755 "$WEBROOT/t_del_perm/nowrite" 2>/dev/null || true
chmod 644 "$WEBROOT/t_del_perm/nowrite/file.txt" 2>/dev/null || true
rm -rf "$WEBROOT/t_del_perm" 2>/dev/null || true
