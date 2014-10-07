#!/bin/bash
# Complement all the automata in a directory.
# dw-24.09.2014

# Defaults for command line parameters
default_algo="fribourg -r2ifc -m"
default_data=~/data/small.tar.gz

# Other frequently edited parameters
goal_archive=~/bin/GOAL-20130711.tar.gz
timeout=600 # Seconds
memory=1G

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-a algorithm] [-d data]"
  echo
  echo "OPTIONS                             [DEFAULT]"
  echo "    -a: Algorithm and options       [\"$default_algo\"]"
  echo "    -d: Data (absolute paths only)  [$default_data]"
}

if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then usage; exit 0; fi
if [ $# -ne 0 ] && [ $# -ne 2 ] && [ $# -ne 4 ]; then echo "Error: invalid number of command line arguments"; exit 1; fi

while getopts ":a:d:" opt; do
  case $opt in
    a) algo=$OPTARG; ;;
    d) data_archive=$OPTARG; ;;
    \?) echo "Error: invalid option: -$OPTARG"; exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done
if [ -z "$algo" ]; then algo=$default_algo; fi
if [ -z $data_archive ]; then data_archive=$default_data; fi

# Enforce tilde (~) expansion, in case the path was enclosed in quotes
data_archive=$(eval echo $data_archive)

# Test for absolute path and file existence
test_file() {
  path=$1
  if [ ! -f $path ]; then echo "Error: \"$path\" is not a valid file"; exit 1; fi
  if [ ${path:0:1} != "/" ]; then echo "Error: must specify absolute paths"; exit 1; fi
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
goal=$goal_dir/goal

# Set maximum and initial Java heap size. JVMARGS is read by the $goal script.
export JVMARGS="-Xmx$memory -Xms$memory"

# Test complementation algorithm and options validity
$goal generate -s 1 >$TMP/tmp.gff
err=$($goal complement -m $algo $TMP/tmp.gff 2>&1 1>/dev/null)
if [ -n "$err" ]; then echo "GOAL error: $err"; exit 1; fi
rm $TMP/tmp.gff

# Out file and log file
out=$TMP/$JOB_NAME.out
log=$TMP/$JOB_NAME.log

# Out file elements
na=NA    # Default string for R's NA ("not available")
y=Y; n=N # Signaling timeout and memory-out
s="\t"   # Column separator

# Nice date
d() { date "+%F %H:%M:%S"; }

# Out file header
cat >$out <<EOF
# Starting date:             $(d)
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME
# Cluster queue:             $QUEUE
# Complementation algorithm: $algo
# Data:                      $(basename $data_archive)
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        ${timeout}s
EOF
# Column titles (and order)
echo -e "states${s}zstates${s}t_out${s}m_out${s}real_t${s}tcpu_t${s}ucpu_t${s}scpu_t${s}dt${s}da${s}file" >>$out

compl=$TMP/complement.gff
compl_reduced=$TMP/reduced_complement.gff
stderr=$TMP/tmp.stderr
for filename in $data/*.gff; do
  file=$(basename $filename)

  # Complementation command execution
  # ----------------------------------------------------------------------------
  echo "$(d): $file" >>$log
  ulimit -S -t $timeout
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
  # Memory excess
  elif grep -q "java.lang.OutOfMemoryError" $stderr; then
    m_out=$y
    states=$na
    zstates=$na
    cat $stderr >>$log; echo "==> This was a memory excess" >>$log
  # Successful execution
  else
    states=$(grep "sid=" $compl | wc -l | tr -d ' ')
    $goal reduce $compl >$compl_reduced # Remove unreachable and dead states
    states_reduced=$(grep "sid=" $compl_reduced | wc -l | tr -d ' ')
    #states_reduced=X
    zstates=$(($states-$states_reduced))
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
  echo -e ${states}${s}${zstates}${s}${t_out}${s}${m_out}${s}${real}${s}${tot_cpu}${s}${user_cpu}${s}${sys_cpu}${s}${dt}${s}${da}${s}${file} >>$out
done

# Copy result files from local scratch to job directory
cp $out $log .
