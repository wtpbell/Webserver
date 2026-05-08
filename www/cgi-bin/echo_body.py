#!/usr/bin/env python3
import os
import sys

length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
body = sys.stdin.buffer.read(length)

print("Content-Type: text/plain")
print()
print("METHOD=" + os.environ.get("REQUEST_METHOD", ""))
print("QUERY_STRING=" + os.environ.get("QUERY_STRING", ""))
print("CONTENT_TYPE=" + os.environ.get("CONTENT_TYPE", ""))
print("CONTENT_LENGTH=" + str(length))
print("READ=" + str(len(body)))
print(body[:200].decode("utf-8", errors="replace"))
