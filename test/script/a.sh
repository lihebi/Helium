#!/bin/bash

echo "This should be in STDOUT"
# 1>&2 echo -n "This should be in STDERR"

echo "This should be in STDOUT"
echo "This should be in STDOUT"
echo "This should be in STDOUT"

# 1>&2 echo -n "This should be in STDERR"
# 1>&2 echo -n "This should be in STDERR"

sleep 1

echo "output after sleep"

sleep 2

exit 1
