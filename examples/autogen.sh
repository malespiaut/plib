#!/bin/sh

echo "Running aclocal"
aclocal
echo "Running automake"
automake --add-missing
echo "Running autoconf"
autoconf

echo "======================================"
echo "Now you are ready to run './configure'"
echo "======================================"
