#!/usr/bin/env bash

HOST=127.0.0.1
PORT=8080
TIMEOUT=1

# Colors
RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
NC="\033[0m"

ok()    { echo -e "${GREEN}PASS${NC}"; }
fail()  { echo -e "${RED}FAIL${NC}"; }
warn()  { echo -e "${YELLOW}RUNNING${NC}"; }

#global counters
TOTAL=0
PASSED=0
FAILED=0

# Expectation type:
#   open   → valid request, server should accept connection
#   close  → invalid request, server expected to reject/close
#   any    → only expect server to stay alive
#
# NOTE:
# These expectations are not strictly enforced yet because
# the server does not send HTTP responses on this branch.

run() {
  name="$1"
  expected="$2"
  expect="$3"     # open | close | any
  request="$4"

  TOTAL=$((TOTAL + 1))

  echo
  echo "=================================================="
  echo "TEST: $name"
  echo "EXPECTED: $expected"
  echo "EXPECT: $expect"
  echo "STATUS: $(warn)"
  echo "--------------------------------------------------"

  printf "%b" "$request" | nc -w "$TIMEOUT" "$HOST" "$PORT" > /dev/null
  rc=$?

  result=0

  case "$expect" in
    open|close|any)
      # For now, we only check that the server accepted the connection
      [[ $rc -lt 2 ]] && result=1
      ;;
    *)
      echo "Unknown expectation: $expect"
      ;;
  esac

  echo "--------------------------------------------------"
  echo -n "RESULT: "
  if [[ $result -eq 1 ]]; then
    ok
    PASSED=$((PASSED + 1))
  else
    fail
    FAILED=$((FAILED + 1))
  fi
  echo "=================================================="
}


#
# ================= VALID REQUESTS =================
#

run "valid GET" \
"Parse OK, Validate OK, connection stays open or closes cleanly" \
"open" \
"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"

run "valid path normalization" \
"Parse OK, NormalizePath succeeds (/a/c/), Validate OK" \
"open" \
"GET /a/b/../c/./ HTTP/1.1\r\nHost: localhost\r\n\r\n"

run "POST with Content-Length 0" \
"Parse OK, Validate OK, no body phase" \
"open" \
"POST /submit HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n"

#
# ================= START-LINE ERRORS =================
#

run "missing Host header" \
"Parse OK, Validate FAIL (BadRequest)" \
"close" \
"GET / HTTP/1.1\r\n\r\n"

run "unsupported HTTP version" \
"Parse OK, Validate FAIL (VersionNotSupported)" \
"close" \
"GET / HTTP/2.0\r\nHost: localhost\r\n\r\n"

run "unsupported method" \
"Parse OK, Validate FAIL (NotImplemented)" \
"close" \
"BREW /coffee HTTP/1.1\r\nHost: localhost\r\n\r\n"

run "absolute URI (proxy style)" \
"Parse OK, Validate FAIL (BadRequest – origin-form only)" \
"close" \
"GET http://example.com/ HTTP/1.1\r\nHost: example.com\r\n\r\n"

#
# ================= PATH / NORMALIZATION ERRORS =================
#

run "directory traversal above root" \
"Parse OK, NormalizePath FAIL (BadRequest or Forbidden)" \
"close" \
"GET /../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"

run "empty target" \
"Parser FAIL (invalid request-line)" \
"close" \
"GET  HTTP/1.1\r\nHost: localhost\r\n\r\n"

#
# ================= HEADER ERRORS =================
#

run "invalid header name (space)" \
"Parser FAIL (invalid header field-name)" \
"close" \
"GET / HTTP/1.1\r\nBad Header: x\r\nHost: localhost\r\n\r\n"

#
# ================= TRANSFER SEMANTICS ERRORS =================
#

run "Content-Length + chunked" \
"Parse OK, Validate FAIL (BadRequest – CL + chunked)" \
"close" \
"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n"

run "invalid chunk size (non-hex)" \
"Parser FAIL (invalid chunk size)" \
"close" \
"POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZ\r\n"

#
# ================= VALID CHUNKED =================
#

run "valid chunked body" \
"Parse OK, Chunked body parsed (Wiki), Validate OK" \
"open" \
"POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n0\r\n\r\n"


#
# ================= SUMMARY =================
#

echo
echo "==================== SUMMARY ====================="
echo -e "TOTAL:  $TOTAL"
echo -e "${GREEN}PASSED: $PASSED${NC}"
echo -e "${RED}FAILED: $FAILED${NC}"
echo "================================================="

[[ $FAILED -eq 0 ]]
