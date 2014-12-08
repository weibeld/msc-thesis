#!/bin/bash
# Delete all core dump files in all the subdirectories. Subdirectories
# relative to either the dir given as argument, or the current dir if no
# argument given.

if [ -n "$1" ]; then prefix=$(sed 's/\/$//' <<< $1)
else prefix="."; fi

dirs=($(echo $prefix/*/))

# $dir is dirname with trailing slash
for dir in ${dirs[@]}; do
  echo "Cleaning $dir"
  rm -f ${dir}core.*
done

