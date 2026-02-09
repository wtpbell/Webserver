#!/usr/bin/env bash
set -euo pipefail
source "$(dirname "$0")/_lib.sh"

start_server

curl -sS -D - "http://${HOST}:${PORT}/" -o /dev/null | sed -n '1,5p'

stop_server
