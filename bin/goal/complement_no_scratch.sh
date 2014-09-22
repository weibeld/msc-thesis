#!/bin/bash
# dw-14.09.2014

# Frequently edited parameters
local=false
if [ "$local" == true ]; then data=~/Desktop/automata; else data=~/data/test_set_s15; fi
goal=goal # GOAL executable
algo=fribourg
opts="-r2ifc -m"
timeout=600 # Seconds
memory=1G
log_stdout=false

# For security
if [ ! -d $data ]; then
  echo "Error: $data is not a valid directory"; exit 1; fi
if [ ! -f $goal ] && [ -z "$(which $goal)" ]; then
  echo "Error: $goal is not a valid file"; exit 1; fi

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
cat >$out <<EOF
# Starting date:             $(d)
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME  
# Complementation algorithm: $algo $opts
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        ${timeout}s
EOF
# Column titles (and order)
echo -e "states${s}t_out${s}m_out${s}real_t${s}tcpu_t${s}ucpu_t${s}scpu_t${s}dt${s}da${s}file" >>$out

# Log to log file, or log file and screen
log() {
  if [ $log_stdout == true ]; then msg "$@" | tee -a $log
  else msg "$@" >>$log; fi
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

# Pack folder with data, move it to local scratch, unpack, work with it
# tar -cz -f tmp.tar.gz -C $(dirname $data) $(basename $data)
# mv tmp.tar.gz $TMP
# tar -xz -f $TMP/tmp.tar.gz -C $TMP && rm $TMP/tmp.tar.gz
# data=$TMP/$(basename $data)

tmp=tmp.out
for filename in $data/*.gff; do
  file=$(basename $filename)

  # Complementation command execution
  # ----------------------------------------------------------------------------
  log "$(d): Complementing $file... " n
  ulimit -S -t $timeout
  { time $goal complement -m $algo $opts $filename; } &>$tmp
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
  echo -e ${states}${s}${t_out}${s}${m_out}${s}${real}${s}${tot_cpu}${s}${user_cpu}${s}${sys_cpu}${s}${dt}${s}${da}${s}${file} >>$out

done

# Copy result files from local scratch to job directory
#cp $TMP/$out $TMP/$log .
