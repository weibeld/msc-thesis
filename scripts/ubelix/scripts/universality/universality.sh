#!/bin/bash
# SGE Job Script
# Test all the automata in a tar.gz archive for universality. An automaton is
# universal if it accepts every word over a given alphabet. The test is done
# by complementation and testing for emptiness. If the complement of an auto-
# maton A is empty, then A itself is universal.
# As the complementation algorithm we take fribourg -m1 -r2c -macc -r -rr,
# because we know from other experiments that firbourg -m1 can complement both
# the 15 and the 20 test set with 1G Java heap without memory outs. Before, we
# tried already piterman with 1G and slice with 4G Java heap, but they all had
# memory outs with the 20 test set. We don't set a timeout because we want all
# tasks to finish.
# To be on the safe side, allocate 2G Java heap and run the job with 5G memory.
#
# dw-28.11.2014

goal_archive=~/bin/GOAL-20141117.tar.gz # GOAL executables
data_archive=~/data/20.tar.gz           # Default data
memory=2G                               # Default Java heap size
algo="fribourg -m1 -r2c -macc -r -rr"   # Complementation construction

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-d data]"
  echo
  echo "ARGUMENTS                               [DEFAULT]"
  echo "    -d: Data (absolute path only)       [$data_archive]"
  echo "    -m: Java heap size             [$memory]"
}
if [ "$1" == "-h" ]; then usage; exit; fi

while getopts ":d:m:" opt; do
  case $opt in
    d) data_archive=$OPTARG; ;;
    m) memory=$OPTARG;       ;;
    \?) echo "Error: invalid option: -$OPTARG";             exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done

# Test for absolute path and file existence
test_file() {
  path=$1
  if [ ! -f $path ]; then echo "Error: \"$path\" is not a valid file"; exit 1; fi
  if [ ${path:0:1} != "/" ]; then echo "Error: must specify absolute paths"; exit 1; fi
}
test_file $data_archive

# Copy data to local scratch
cp $data_archive $TMP/tmp.tar.gz
data=$TMP/data
mkdir $data
tar xzf $TMP/tmp.tar.gz -C $data --strip-components 1 && rm $TMP/tmp.tar.gz

# Copy GOAL to local scratch
cp $goal_archive $TMP/tmp.tar.gz
goal_dir=$TMP/GOAL
mkdir $goal_dir
tar xzf $TMP/tmp.tar.gz -C $goal_dir --strip-components 1 && rm $TMP/tmp.tar.gz
goal=$goal_dir/gc

# Nice date
d() { date "+%F %H:%M:%S"; }

# Result and log files (in working directory on network file system)
out=$JOB_NAME.out
log=$JOB_NAME.log
touch I_AM_STILL_RUNNING

# Result file header
cat >$out <<EOF
# Starting date:             $(d)
# Cluster job name:          $JOB_NAME
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME
# Cluster queue:             $QUEUE
# Test method:               complement(A) -> A'; emptiness(A')
# Complementation algorithm: $algo
# Data:                      $(basename $data_archive)
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        No timeout
EOF
# Column headers
echo -e "univ\tfile" >>$out

# Set Java heap size
export JVMARGS="-Xmx$memory -Xms$memory"

# File to write complement automaton to (to test for emptiness)
complement=$TMP/complement.gff
# File to write stderr of the GOAL command to (to test for memory out)
errors=$TMP/err

# Test if something has been written to file $errors (e.g. memory out)
test_errors() {
  if [ -s $errors ]; then cat $errors >>$log; exit 1; fi
}

# Initialise log file
job_start=$(date +%s)
echo "Start: $(d) ($job_start)" >$log
i=0  # Counter

# Real work. Iterate through all the automata and do the universality test.
for automaton in $data/*.gff; do
  echo "$((i+=1)). $(d): $(basename $automaton)" >>$log
  $goal complement -m $algo $automaton >$complement 2>$errors
  test_errors
  if [ $($goal emptiness $complement 2>$errors) = true ]
  then test_errors; universal=Y
  else test_errors; universal=N; fi
  echo -e "$universal\t$(basename $automaton)" >>$out
done

# Finalize log file
job_end=$(date +%s)
echo "End: $(d) ($job_end)" >>$log
total_sec=$(($job_end-$job_start))
hours=$(($total_sec/3600))
rem_sec=$(($total_sec%3600))
min=$(($rem_sec/60))
sec=$(($rem_sec%60))
echo "Duration (wallclock): ${hours}h ${min}min ${sec}sec" >>$log
echo "                      ${total_sec}sec" >>$log

# Copy result file from local scratch to job directory
mv I_AM_STILL_RUNNING I_AM_FINISHED
