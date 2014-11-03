#!/bin/bash
# Compress the folder given as argument

dir=$(sed 's/\/$//' <<< $1)

echo "Creating $dir.tar.gz"
tar czf $dir.tar.gz $dir

