#!/bin/bash
# Rename GOAL test automata (the 11'000) to shorter filenames.
# dw-14.09.2014

# Exit on error
set -e

# Remove possible trailing slash from folder (command line arg)
folder=$(sed 's/\/$//' <<< $1)

for filename in $folder/*.gff; do
  file=$(basename $filename)
  dir=$(dirname $filename)

  # Old filename => new-s-15-r-1.00-f-0.10--1-of-100.gff

  # Remove double dash
  file_new=$(echo $file | sed 's/--/-/')

  # Tokenize and read into array
  old_ifs=$IFS; IFS=-
  t=($file_new)
  IFS=$old_ifs
  # t[0]: new
  # t[1]: s
  # t[2]: 15
  # t[3]: r
  # t[4]: 1.00
  # t[5]: f
  # t[6]: 0.10
  # t[7]: 1
  # t[8]: of
  # t[9]: 100.gff

  # New filename => s15_dt1.0_da0.1_001.gff
  file_new=${t[1]}${t[2]}"_t"$(printf %.1f ${t[4]})"_a"$(printf %.1f ${t[6]})"_"$(printf %03d ${t[7]})".gff"

  # Rename
  mv $filename $dir/$file_new
done
