#!/bin/sh
#
# A 'yes'-like script with bounded output, and a signaling mechanism of writing
# to arg1.

yes | head -10

echo >"$1"

while : ; do
	sleep 10
done
