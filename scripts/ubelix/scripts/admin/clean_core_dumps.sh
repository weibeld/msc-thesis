#!/bin/bash
# Empty all sub-directories of the directory given as argument.

if [ -n "$1" ]; then prefix=$(sed 's/\/$//' <<< $1)
else prefix="."; fi

dirs=($(echo $prefix/*/))

# $dir is dirname with trailing slash
for dir in ${dirs[@]}; do
  echo "Cleaning $dir"
  rm -f ${dir}core.*
done

