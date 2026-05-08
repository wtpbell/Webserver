#!/bin/bash

echo -ne "Status: 200 OK\n"
echo -ne "Content-Type: text/plain; charset=ISO-8859-1\n"
echo -ne "\n"
echo -ne "Hello from $(whoami)!\n"
