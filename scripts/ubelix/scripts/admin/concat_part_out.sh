#!/bin/bash
# Concatenate the out files in all the subdirectories of the current dir to a
# master out file in the current dir. The subdirectories are jobs each process-
# ing a partition of the whole test set.
#
# Daniel Weibel, 08.12.2014

dirs=($(echo */))

master_out=$(basename $(pwd)).out
echo "Creating $master_out"
touch $master_out

# $dir is a simple directory name with a trailing slash
for dir in ${dirs[@]}; do
  part_out=${dir}${dir%/}.out
  echo "Concatenating $part_out"
  cat $part_out >>$master_out
done