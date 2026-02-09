#!/usr/bin/env bash
set -euo pipefail

# Path helpers
THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="${ROOT_DIR:-$(cd "$THIS_DIR/../.." && pwd)}"

# Defaults (can be overridden by env)
HOST="${HOST:-127.0.0.1}"
PORT="${PORT:-8080}"

# Where is the webserv binary?
# Try common names if WEBSERV_BIN not set.
if [[ -z "${WEBSERV_BIN:-}" ]]; then
  if [[ -x "$ROOT_DIR/webserv" ]]; then
    WEBSERV_BIN="$ROOT_DIR/webserv"
  elif [[ -x "$ROOT_DIR/Webserv" ]]; then
    WEBSERV_BIN="$ROOT_DIR/Webserv"
  elif [[ -x "./webserv" ]]; then
    WEBSERV_BIN="./webserv"
  elif [[ -x "./Webserv" ]]; then
    WEBSERV_BIN="./Webserv"
  else
    WEBSERV_BIN="./webserv"
  fi
fi

# Config file: allow caller override; otherwise use tests/bash/default.conf
WEBSERV_CONF="${WEBSERV_CONF:-$ROOT_DIR/tests/bash/default.conf}"

SERVER_LOG="${SERVER_LOG:-$ROOT_DIR/tests/bash/server.log}"
SERVER_PID_FILE="${SERVER_PID_FILE:-$ROOT_DIR/tests/bash/server.pid}"

die() { echo "[ERR] $*" >&2; exit 1; }

require_file() { [[ -f "$1" ]] || die "file not found: $1"; }
require_exec() { [[ -x "$1" ]] || die "executable not found: $1"; }

# Pick a timeout binary (GNU timeout on Linux, gtimeout on mac, fallback none)
if [[ -z "${TIMEOUT_BIN:-}" ]]; then
  if command -v timeout >/dev/null 2>&1; then
    TIMEOUT_BIN="timeout"
  elif command -v gtimeout >/dev/null 2>&1; then
    TIMEOUT_BIN="gtimeout"
  else
    TIMEOUT_BIN="" # no timeout available
  fi
fi

# Create a minimal config if missing (so tests don't all fail immediately).
# Adjust this template if your team's config grammar differs.
ensure_default_conf() {
  if [[ -f "$WEBSERV_CONF" ]]; then
    return 0
  fi

  mkdir -p "$(dirname "$WEBSERV_CONF")"
  cat >"$WEBSERV_CONF" <<EOF
events {
}
http {
  server {
    listen $PORT;
    root /tmp;
  }
}
EOF
}

# Wait until TCP port is open
wait_port() {
  local tries=60
  for _ in $(seq 1 "$tries"); do
    if nc -z "$HOST" "$PORT" >/dev/null 2>&1; then
      return 0
    fi
    sleep 0.05
  done
  return 1
}

start_server() {
  ensure_default_conf
  require_exec "$WEBSERV_BIN"
  require_file "$WEBSERV_CONF"

  stop_server || true

  : > "$SERVER_LOG"
  "$WEBSERV_BIN" "$WEBSERV_CONF" >"$SERVER_LOG" 2>&1 &
  echo $! > "$SERVER_PID_FILE"

  if ! wait_port; then
    echo "[ERR] server did not start; log:"
    tail -200 "$SERVER_LOG" || true
    stop_server || true
    exit 1
  fi
}

stop_server() {
  if [[ -f "$SERVER_PID_FILE" ]]; then
    local pid
    pid="$(cat "$SERVER_PID_FILE" || true)"
    rm -f "$SERVER_PID_FILE"
    if [[ -n "${pid:-}" ]]; then
      kill "$pid" >/dev/null 2>&1 || true
      wait "$pid" >/dev/null 2>&1 || true
    fi
  fi
}

cleanup() { stop_server || true; }
trap cleanup EXIT

# ------------------------------------------------------------------------------
# Sending helpers
# ------------------------------------------------------------------------------

# send_raw:
# - If called with an argument, it sends that payload.
# - If called without args, it reads from stdin (so you can pipe into it).
send_raw() {
  local payload="${1-}"

  # netcat variants:
  # -w 2 ensures connect/read timeout
  # -q 1 closes after stdin EOF (GNU netcat-openbsd supports -q)
  # If your nc doesn't support -q, change to: nc -w 2 "$HOST" "$PORT"
  if [[ -n "$payload" ]]; then
    if [[ -n "$TIMEOUT_BIN" ]]; then
      printf "%b" "$payload" | "$TIMEOUT_BIN" 3s nc -q 1 -w 2 "$HOST" "$PORT"
    else
      printf "%b" "$payload" | nc -q 1 -w 2 "$HOST" "$PORT"
    fi
  else
    if [[ -n "$TIMEOUT_BIN" ]]; then
      "$TIMEOUT_BIN" 3s nc -q 1 -w 2 "$HOST" "$PORT"
    else
      nc -q 1 -w 2 "$HOST" "$PORT"
    fi
  fi
}

send_payload() { send_raw "$1"; }

# ------------------------------------------------------------------------------
# Assertions / parsing response helpers
# ------------------------------------------------------------------------------

# first status line of the entire wire
status_line() { sed -n '1{/^HTTP\//p;}' <<<"$1"; }

status_code() { awk 'NR==1{print $2}' <<<"$(status_line "$1")"; }

count_responses() { grep -c '^HTTP/' <<<"$1" || true; }

# Extract Nth status line (1-based)
nth_status_line() {
  local n="$1" wire="$2"
  awk -v n="$n" '
    /^HTTP\/1\.[01] / {c++; if (c==n) {print; exit}}
  ' <<<"$wire"
}

ASSERT_STATUS() {
  local want="$1"
  local wire="$2"
  local line code

  line="$(status_line "$wire")"
  code="$(status_code "$wire")"

  if [[ -z "${line:-}" || -z "${code:-}" ]]; then
    echo "ASSERT_STATUS failed: expected $want"
    echo "status line:"
    echo "$line"
    echo "wire:"
    echo "$wire"
    return 1
  fi

  if [[ "$code" != "$want" ]]; then
    echo "ASSERT_STATUS failed: expected $want, got $code"
    echo "status line: $line"
    echo "wire:"
    echo "$wire"
    return 1
  fi
}

ASSERT_STATUS_N() {
  local n="$1" want="$2" wire="$3"
  local line code
  line="$(nth_status_line "$n" "$wire")"
  code="$(awk '{print $2}' <<<"$line")"

  if [[ -z "${line:-}" || -z "${code:-}" ]]; then
    echo "ASSERT_STATUS_N failed: response #$n expected $want"
    echo "line:"
    echo "$line"
    echo "wire:"
    echo "$wire"
    return 1
  fi

  if [[ "$code" != "$want" ]]; then
    echo "ASSERT_STATUS_N failed: response #$n expected $want, got $code"
    echo "line: $line"
    echo "wire:"
    echo "$wire"
    return 1
  fi
}

ASSERT_N_RESP() {
  local want="$1"
  local wire="$2"
  local got
  got="$(count_responses "$wire")"
  if [[ "$got" != "$want" ]]; then
    echo "Expected $want responses, got $got"
    echo "wire:"
    echo "$wire"
    return 1
  fi
}

contains() {
  local hay="$1" needle="$2"
  grep -Fq "$needle" <<<"$hay"
}
