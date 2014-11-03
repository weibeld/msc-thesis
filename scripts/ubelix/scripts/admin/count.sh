#!/bin/bash

if [ $# -lt 2 ]; then
  echo "USAGE:"
  echo "    $(basename $0) STRING FILE"
  echo "Counts the number of lines containing STRING in FILE."
  exit 0
fi

string=$1
file=$2

cat $file | grep "$string" | wc -l

