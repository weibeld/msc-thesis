#!/bin/bash
# SGE Job Script
# Test all the automata in the specified .tar.gz file for completeness. Requires
# the ch.unifr.goal.util plugin providing the "complete" command line command.
# There is no timeout and the Java heap size is fixed to 1G because testing for
# completeness doesn't require much memory.
#
# dw-20.12.2014

memory=1G                               # Fixed memory
goal_archive=~/bin/GOAL-20141117.tar.gz # GOAL executables
data_archive=~/data/15.tar.gz           # Default data

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-d data]"
  echo
  echo "ARGUMENTS                               [DEFAULT]"
  echo "    -d: Data (absolute path only)       [$data_archive]"
}
if [ "$1" == "-h" ]; then usage; exit; fi

while getopts ":d:" opt; do
  case $opt in
    d) data_archive=$OPTARG; ;;
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
# Data:                      $(basename $data_archive)
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        No timeout
EOF
# Column headers
echo -e "complete\tfile" >>$out

# Set Java heap size
export JVMARGS="-Xmx$memory -Xms$memory"

# Files to capture stdout and stderr output of GOAL "complete" command
stdout=$TMP/stdout
stderr=$TMP/stderr

# Initialise log file
job_start=$(date +%s)
echo "Start: $(d) ($job_start)" >$log
i=0  # Counter

# Real work. Iterate through all the automata and do the completeness test.
for aut in $data/*.gff; do
  echo "$((i+=1)). $(d): $(basename $aut)" >>$log
  $goal complete $aut 1>$stdout 2>$stderr
  if [ -s $stderr ]; then cat $stderr >>$log; exit 1; fi
  if [ $(cat $stdout) = true ]
    then complete=Y 
    else complete=N
  fi
  echo -e "$cmpl\t$(basename $aut)" >>$out
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
