#!/bin/bash
#
# Run 'rank -tr -ro' on 10 partitions of the GOAl test set
#
# Dependencies:
#   - Partitioned version of the GOAL test set in ~/data/goal_splitted
#
# Daniel Weibel <daniel.weibel@unifr.ch> May 2015
#------------------------------------------------------------------------------#

# Number of partitions
parts=10

# Size of a partition
size=1100

# Pad a number with 0s to the number of digits of 'parts'
pad() {
  local d=$(($(echo -n "$parts" | wc -c)))
  printf "%0${d}d" "$1"
}

n=0
for d in */; do
  # Subdirectory name
  d=${d%/}

  # Number of current partition (padded)
  p=$(pad $(($n + 1)))

  # Calculate at which number to start with 'id' in each partition
  i0=$(($n * $size + 1))

  # First partition: print CSV header in CSV file (no -n)
  if [[ $n -eq 0 ]]; then
    submit -q mpi.q -m 4G -c 72:00:00 -d "$(pwd)/$d" -s 10G -p 4 -n ea \
      ~/submit/complement \
        -a "rank -tr -ro" \
        -d ~/data/goal_splitted/goal.${p}_of_${parts}.tar.gz \
        -t 600 \
        -m 1G \
        -i "$i0"

  # Remaining partitions: do NOT print CSV header in CSV file (-n)
  else
    submit -q mpi.q -m 4G -c 72:00:00 -d "$(pwd)/$d" -s 10G -p 4 -n ea \
      ~/submit/complement \
        -a "rank -tr -ro" \
        -d ~/data/goal_splitted/goal.${p}_of_${parts}.tar.gz \
        -t 600 \
        -m 1G \
        -i "$i0" \
        -n
  fi

  n=$(($n + 1))
done

