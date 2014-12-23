#!/bin/bash
#==============================================================================#
# SGE Job Script
# Test automata for universality. We use the "universal" command provided by
# the plugin ch.unifr.goal.util.
#
# For the size 20 test set, the required resources might be very high, depending
# on the complementation construction used for the "universal" command. Regard-
# ing memory, 4G is not enough (observed memouts with piterman 2G, fribourg 2G,
# slice 4G). Using something like 16G is recommendd. Regarding time, fribourg
# didn't finish in 96 hours real time (there's no timeout for individual tasks).
#
# Author: Daniel Weibel <daniel.weibel@unifr.ch>, Nov. 2014 - Dec. 2014
#==============================================================================#

goal_archive=~/bin/GOAL-20141117.tar.gz # GOAL executables
data_archive=~/data/20.tar.gz           # Default data (automata to test)
memory=16G                              # Default Java heap size
algo="piterman -eq -ro"                 # Default complementation construction

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-d data] [-m memory] [-a algo]"
  echo
  echo "ARGUMENTS                               [DEFAULT]"
  echo "    -d: Data (absolute path only)       [$data_archive]"
  echo "    -m: Java heap size                  [$memory]"
  echo "    -a: Complementation construction    [$algo]"
}
if [ "$1" == "-h" ]; then usage; exit; fi

while getopts ":d:m:a:" opt; do
  case $opt in
    d) data_archive=$OPTARG; ;;
    m) memory=$OPTARG;       ;;
    a) algo=$OPTARG;         ;;
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

# Result and log files on GPFS to see progress
out=$JOB_NAME.out
log=$JOB_NAME.log
touch I_AM_STILL_RUNNING  # Signalise current state

# Result file header
cat >$out <<EOF
# Starting date:             $(d)
# Cluster job name:          $JOB_NAME
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME
# Cluster queue:             $QUEUE
# Test method:               "universal" command of plugin ch.unifr.goal.util 
# Complementation algorithm: $algo
# Data:                      $(basename $data_archive)
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        No timeout
EOF
# Column headers
echo -e "univ\tfile" >>$out

# Set Java heap size
export JVMARGS="-Xmx$memory -Xms$memory"

# Files to capture stdout and stderr of GOAL "universal" command
stderr=$TMP/stderr
stdout=$TMP/stdout

# Initialise log file
job_start=$(date +%s)
echo "Start: $(d) ($job_start)" >$log
i=0  # Counter counting tasks

# Real work. Iterate through all the automata and do the universality test.
for aut in $data/*.gff; do
  echo "$((i+=1)). $(d): $(basename $aut)" >>$log
  $goal universal -m $algo $aut 1>$stdout 2>$stderr
  if [[ -s $stderr ]]; then cat $stderr >>$log; exit 1; fi
  if [[ $(cat $stdout) = true ]]
    then universal=Y 
    else universal=N
  fi
  echo -e "$universal\t$(basename $aut)" >>$out
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

mv I_AM_STILL_RUNNING I_AM_FINISHED
