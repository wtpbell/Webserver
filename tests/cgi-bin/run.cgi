#!/bin/bash

if [ -z "$CONTENT_LENGTH" ] || [ $CONTENT_LENGTH -le 0 ]; then
	exit 1
fi

LENGTH=$(head -c $CONTENT_LENGTH | wc -c)
if [ $LENGTH -ne $CONTENT_LENGTH ]; then
	exit 1
fi

echo "Content-Type: text/plain"
echo "Status: 204 VERY OK"
echo ""

env

exit 0
