#!/bin/bash
# dw-14.09.2014

# Algorithm and options can be specified as command line arguments
if [ -n "$1" ]; then algo="$*"
else algo="fribourg -r2ifc -m"; fi

# Other frequently edited parameters
data_archive=~/data/test_set_s15.tar.gz
goal_archive=~/bin/GOAL-20140808.tar.gz
timeout=600 # Seconds
memory=1G
log_stdout=false

test() {
  if [ ! -f "$1" ]; then echo "Error: \"$1\" is not a valid file"; exit 1; fi
}
test $data_archive
test $goal_archive

# Out file and log file
out=out
log=log

# Out file elements
na=NA    # Default string for R's NA ("not available")
y=Y; n=N # Signaling timeout and memory-out
s="\t"   # Column separator

# Nice date
d() { date "+%F %H:%M:%S"; }

# Out file header
cat >$TMP/$out <<EOF
# Starting date:             $(d)
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME
# Cluster queue:             $QUEUE
# Complementation algorithm: $algo
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        ${timeout}s
EOF
# Column titles (and order)
echo -e "states${s}t_out${s}m_out${s}real_t${s}tcpu_t${s}ucpu_t${s}scpu_t${s}dt${s}da${s}file" >>$TMP/$out

# Log to log file, or log file and screen
log() {
  if [ $log_stdout == true ]; then msg "$@" | tee -a $TMP/$log
  else msg "$@" >>$TMP/$log; fi
}
msg() {
  if [ "$2" != "" ]; then echo -n "$1"
  else echo "$1"; fi
}

# Set maximum and initial Java heap size
export JVMARGS="-Xmx$memory -Xms$memory"

# Capture Ctrl-C
ctrl_c() { exit 1; }
trap ctrl_c SIGINT

# Copy data to scratch
cp $data_archive $TMP/tmp.tar.gz
data=$TMP/data
mkdir $data
tar xzf $TMP/tmp.tar.gz -C $data --strip-components 1 && rm $TMP/tmp.tar.gz

# Copy GOAL to scratch
cp $goal_archive $TMP/tmp.tar.gz
goal_dir=$TMP/GOAL
mkdir $goal_dir
tar xzf $TMP/tmp.tar.gz -C $goal_dir --strip-components 1 && rm $TMP/tmp.tar.gz
goal=$goal_dir/goal

tmp=$TMP/tmp.out
for filename in $data/*.gff; do
  file=$(basename $filename)

  # Complementation command execution
  # ----------------------------------------------------------------------------
  log "$(d): Complementing $file... "
  ulimit -S -t $timeout
  { time $goal complement -m $algo $filename; } &>$tmp
  if [ $? -ne 0 ]; then cat $tmp >>$TMP/$log; fi
  #timeout ${timeout}s $goal complement -m fribourg $filename &>$tmp # Exit code 124
  ulimit -S -t unlimited

  # States, timeout, memory-out
  # ----------------------------------------------------------------------------
  t_out=$n
  m_out=$n
  # Timeout
  if grep -q "time limit exceeded" $tmp; then
    log "Timeout"
    t_out=$y
    states=$na
  # Memory excess
  elif grep -q "java.lang.OutOfMemoryError" $tmp; then
    log "Memory excess"
    m_out=$y
    states=$na
  # Successful execution
  else
    log "Success"
    states=$(grep sid= $tmp | wc -l | tr -d ' ')
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
  real=$(seconds "$(grep '^real' $tmp)")
  user_cpu=$(seconds "$(grep '^user' $tmp)")
  sys_cpu=$(seconds "$(grep '^sys' $tmp)")
  tot_cpu=$(bc <<< "$user_cpu + $sys_cpu")

  # Transition and acceptance density
  # ----------------------------------------------------------------------------
  # Assumes filename of the form "s15_t1.0_a0.1_001.gff"
  dt=$(echo $file | egrep -o "t[0-9].[0-9]" | sed 's/^t//')
  da=$(echo $file | egrep -o "a[0-9].[0-9]" | sed 's/^a//')

  # Write out file
  echo -e ${states}${s}${t_out}${s}${m_out}${s}${real}${s}${tot_cpu}${s}${user_cpu}${s}${sys_cpu}${s}${dt}${s}${da}${s}${file} >>$TMP/$out

done

# Copy result files from local scratch to job directory
cp $TMP/$out $TMP/$log .
