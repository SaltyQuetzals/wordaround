#!/bin/sh

echo "Combining lists..."
cat words/* | sort | uniq -u > words.txt
echo "Done"
