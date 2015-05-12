#!/bin/bash
# SGE Job Script
# Complement all the automata in a directory inside a .tar.gz archive with the
# complementation construction specified as argument. Write result statistics
# to a file that is to be processed by R.
#
# dw-24.09.2014

# GOAL version to use
goal_archive=~/bin/GOAL-20141117.tar.gz

# Defaults for command line parameters
algo="fribourg -m1"
data_archive=~/data/small.tar.gz
timeout=600 # Seconds
memory=1G
working_dir=true   # Out an log file in working dir which is usually on GPFS
print_header=true  # If partitioned test set: first part. "true", others "false"

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-a algorithm] [-d data] [-t timeout] [-m memory]"
  echo
  echo "OPTIONS                                 [DEFAULT VALUE]"
  echo "    -a: Algorithm (including options)   [\"$algo\"]"
  echo "    -d: Data (absolute path only)       [$data_archive]"
  echo "    -t: Timeout (seconds)               [$timeout]"
  echo "    -m: Max. Java heap size             [$memory]"
  echo "    -o: Out and log file in working dir [$working_dir]"
  echo "    -h: Print column header in out file [$print_header]"
}
if [[ "$1" = -h ]] && [[ $# -eq 1 ]]; then usage; exit; fi

while getopts ":a:d:t:m:o:h:" opt; do
  case $opt in
    a) algo=$OPTARG;         ;;
    d) data_archive=$OPTARG; ;;
    t) timeout=$OPTARG;      ;;
    m) memory=$OPTARG;       ;;
    o) working_dir=$OPTARG;  ;;
    h) print_header=$OPTARG; ;;
    \?) echo "Error: invalid option: -$OPTARG";             exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done

# Test for absolute path and file existence
test_file() {
  path=$1
  if [ ! -f $path ]; then echo "Error: \"$path\" is not a valid file"; exit 1; fi
  if [ ${path:0:1} != / ]; then echo "Error: must specify absolute paths"; exit 1; fi
}
test_file $data_archive
test_file $goal_archive

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

# Set maximum and initial Java heap size. JVMARGS is read by the $goal script.
export JVMARGS="-Xmx$memory -Xms$memory"

# Test complementation algorithm and options validity
$goal generate -s 1 >$TMP/tmp.gff
err=$($goal complement -m $algo $TMP/tmp.gff 2>&1 1>/dev/null)
if [ -n "$err" ]; then echo "GOAL error: $err"; exit 1; fi
rm $TMP/tmp.gff

# Out file and log file
if [ $working_dir = true ]; then
  out=$JOB_NAME.out # in current working dir (GPFS home)
  log=$JOB_NAME.log
  touch I_AM_STILL_RUNNING
else 
  out=$TMP/$JOB_NAME.out # in local scratch
  log=$TMP/$JOB_NAME.log
fi

# Out file elements
na=NA    # Default string for R's NA ("not available")
y=Y; n=N # Signaling timeout and memory-out
s=","    # Column separator

# Nice date
d() { date "+%F %H:%M:%S"; }

# Out file header
cat >$out <<EOF
# Starting date:             $(d)
# Cluster job name:          $JOB_NAME
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME
# Cluster queue:             $QUEUE
# Complementation algorithm: $algo
# Data:                      $(basename $data_archive)
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        ${timeout}s
EOF
# Column header (used by R)
if [[ "$print_header" = true ]]; then
echo -e "states${s}t_out${s}m_out${s}real_t${s}tcpu_t${s}ucpu_t${s}scpu_t${s}dt${s}da${s}file" >>$out
fi

compl=$TMP/complement.gff
compl_reduced=$TMP/reduced_complement.gff
stderr=$TMP/tmp.stderr

# Log inits
job_start=$(date +%s)
echo "Start: $(d) ($job_start)" >$log
i=0 # Counter

for filename in $data/*.gff; do
  file=$(basename $filename)
  ((i+=1))
  echo "$i. $(d): $file" >>$log
  # Complementation command execution
  # ----------------------------------------------------------------------------
  if [[ $timeout -gt 0 ]]; then ulimit -S -t $timeout; fi
  { time $goal complement -m $algo $filename; } 1>$compl 2>$stderr
  # If timeout:
  #   - exit code 152
  #   - to stderr: ...goal: line 19: <PID> CPU time limit exceeded (core dumped) java ${JVMARGS} ...
  #   - Java core dump file core.<PID> in working directory
  # If memory out:
  #   - exit code 0
  #   - to stderr: Exception in thread "Thread-1" java.lang.OutOfMemoryError: Java heap space ...
  ulimit -S -t unlimited

  # States, timeout, memory-out
  # ----------------------------------------------------------------------------
  t_out=$n
  m_out=$n
  # Timeout
  if grep -q "time limit exceeded" $stderr; then
    t_out=$y
    states=$na
    zstates=$na
    cat $stderr >>$log; echo "==> This was a timeout" >>$log
    # Remove core dump file to avoid that disk quota of UBELIX home is exceeded
    rm core.*
  # Memory excess
  elif grep -q "java.lang.OutOfMemoryError" $stderr; then
    m_out=$y
    states=$na
    zstates=$na
    cat $stderr >>$log; echo "==> This was a memory excess" >>$log
  # Successful execution
  else
    states=$(grep "sid=" $compl | wc -l | tr -d ' ')
    # $goal reduce $compl >$compl_reduced # Remove unreachable and dead states
    # states_reduced=$(grep "sid=" $compl_reduced | wc -l | tr -d ' ')
    # states_reduced=X
    # zstates=$(($states-$states_reduced))
  fi

  # Times
  # ----------------------------------------------------------------------------
  # Convert a line of the UNIX time output to seconds. The single argument must
  # be ONE STRING containig a substring of the form "4m54.234s".
  seconds() {
    m=$(echo "$1" | egrep -o '[0-9]*m' | sed 's/m$//')
    s=$(echo "$1" | egrep -o '[0-9][0-9]?.[0-9]*s' | sed 's/s$//')
    seconds=$(bc <<< "$m * 60 + $s")
    printf "%.3f" $seconds # printf performs rounding
  }
  real=$(seconds "$(grep '^real' $stderr)")
  user_cpu=$(seconds "$(grep '^user' $stderr)")
  sys_cpu=$(seconds "$(grep '^sys' $stderr)")
  tot_cpu=$(bc <<< "$user_cpu + $sys_cpu")

  # Transition and acceptance density
  # ----------------------------------------------------------------------------
  # Assumes filename of the form "s15_t1.0_a0.1_001.gff"
  dt=$(echo $file | egrep -o "t[0-9].[0-9]" | sed 's/^t//')
  da=$(echo $file | egrep -o "a[0-9].[0-9]" | sed 's/^a//')

  # Write line to out file
  echo -e ${states}${s}${t_out}${s}${m_out}${s}${real}${s}${tot_cpu}${s}${user_cpu}${s}${sys_cpu}${s}${dt}${s}${da}${s}${file} >>$out
done

# Log end
job_end=$(date +%s)
echo "End: $(d) ($job_end)" >>$log
total_sec=$(($job_end-$job_start))
hours=$(($total_sec/3600))
rem_sec=$(($total_sec%3600))
min=$(($rem_sec/60))
sec=$(($rem_sec%60))
echo "Duration (wallclock): ${hours}h ${min}min ${sec}sec" >>$log
echo "                      ${total_sec}sec" >>$log

# Copy result files from local scratch to job directory
if [ $working_dir = true ]
then mv I_AM_STILL_RUNNING I_AM_FINISHED
else cp $out $log .
fi
