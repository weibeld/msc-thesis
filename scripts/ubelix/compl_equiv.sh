#!/bin/bash
# Test the correctness of a complementation algorithm by performing
# complementation-equivalence tests on random automata.
# dw-11.10.2014

set -e

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [ALGO] [RUNS] [MEM] [STATES] [SYMBOLS]"
  echo
  echo "ARGUMENTS:                                               [DEFAULT]"
  echo "    ALGO       Algorithm to test for correctness         [\"fribourg -m\"]"
  echo "    RUNS       Number of random automata to test         [5]"
  echo "    MEM        Max Java heap size                        [2G]"
  echo "    STATES     Min and max size of random automata       [4-4]"
  echo "    SYMBOLS    Min and max alphabet size of rand. aut.   [2-4]"
  echo
  echo "==> POSITIONAL ORDER OF ARGUMENTS MATTERS!"  
}
if [ "$1" == -h ]; then usage; exit 0; fi

# ${var-x}: x is the default value if var is not set
# Note: getopts would make sense here
algo=${1-fribourg -m}
n=${2-5}
mem=${3-2G}
s_range=${4-4-4}
a_range=${5-2-4}
# Extract min and max values
s_min=$(cut -d - -f 1 <<<"$s_range")
s_max=$(cut -d - -f 2 <<<"$s_range")
a_min=$(cut -d - -f 1 <<<"$a_range")
a_max=$(cut -d - -f 2 <<<"$a_range")

# Copy GOAL script to local scratch and customise it with the desired algorithm
batch=$TMP/batch.gs
cp ~/scripts/compl_equiv.gs $batch
sed -i "s/fribourg/$algo/" $batch

# Copy GOAL to local scratch
cp ~/bin/GOAL-20130711.tar.gz $TMP/tmp.tar.gz
goal_dir=$TMP/GOAL
mkdir $goal_dir
tar xzf $TMP/tmp.tar.gz -C $goal_dir --strip-components 1 && rm $TMP/tmp.tar.gz
goal=$goal_dir/goal

# Set Java heap size for GOAL. Otherwise, it may use more than the memory
# allocated to the job
export JVMARGS="-Xmx$mem -Xms$mem"

# Generate a random integer between $1 (including) and $2 (including). This
# function is used by the compl_equiv.gs script.
range() {
  from=$1 # including
  to=$2   # including
  size=$(($to-$from+1))
  echo $(($RANDOM%$size + $from))
}
export -f range # Bash way of exporting functions

# Execute the GOAL batch script
$goal batch $batch $n $s_min $s_max $a_min $a_max
