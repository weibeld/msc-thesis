#!/bin/bash
# dw-14.09.2014

# Frequently edited parameters
local=false
if [ "$local" == true ]; then data=~/Desktop/automata; else data=~/data/test; fi
goal=goal # Assumes 'goal' is in PATH on cluster node
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

# Result (out) and log file
out=out
log=log

# Out file format
na=NA  # Default string for R's NA ("not available")
yes=Y; no=N # Signaling timeout and memory-out
sep="\t"

# Nice date
d() { date "+%F %H:%M:%S"; }

# Out file initialisation
echo -e "# Starting date:\t\t$(d)" >>$TMP/$out
echo -e "# Cluster node:\t\t\t$(uname -n)" >>$TMP/$out   
echo -e "# Complementation algorithm:\t$algo $opts" >>$TMP/$out
echo -e "# Memory limit (Java heap):\t$memory" >>$TMP/$out
echo -e "# Timeout (CPU time):\t\t${timeout}s" >>$TMP/$out
echo -e "states${sep}t_out${sep}m_out${sep}real_t${sep}ucpu_t${sep}scpu_t${sep}tcpu_t${sep}dt${sep}da${sep}file" >>$TMP/$out

# Log to log file, or log file and screen
log() {
  if [ $TMP/$log_stdout == true ]; then msg "$@" | tee -a $TMP/$log
  else msg "$@" >>$TMP/$log; fi
}
msg() {
  if [ "$2" != "" ]; then echo -n "$1"
  else echo "$1"; fi
}

# Set maximum Java heap size
export JVMARGS="-Xmx$memory"

# Capture Ctrl-C
ctrl_c() { exit 1; }
trap ctrl_c SIGINT

# Pack folder holding data and move it to local scratch
tar -cz -f tmp.tar.gz -C $(dirname $data) $(basename $data)
mv tmp.tar.gz $TMP
tar -xz -f $TMP/tmp.tar.gz -C $TMP && rm $TMP/tmp.tar.gz
data=$TMP/$(basename $data)

tmp=$TMP/tmp.out
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
  t_out=$no
  m_out=$no
  # Timeout
  if grep -q "time limit exceeded" $tmp; then
    log "Timeout"
    t_out=$yes
    states=$na
  # Memory excess
  elif grep -q "java.lang.OutOfMemoryError" $tmp; then
    log "Memory excess"
    m_out=$yes
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
  echo -e ${states}${sep}${t_out}${sep}${m_out}${sep}${real}${sep}${user_cpu}${sep}${sys_cpu}${sep}${tot_cpu}${sep}${dt}${sep}${da}${sep}${file} >>$TMP/$out

done

# Copy result files to the global job directory
cp $TMP/$out $TMP/$log .
