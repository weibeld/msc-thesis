#!/bin/bash
# dw-14.09.2014

data=~/data/test_set_s15
#data=~/Desktop/automata
goal=~/bin/GOAL-20140808/goal
#goal=goal
out=out
log=log
timeout=600 # Seconds
memory=1G
log_stdout=false
algo=fribourg
opts="-r2ifc -m"

# For security
if [ ! -d $data ]; then echo "Error: $data is not a valid directory"; exit 1; fi
if [ ! -f $goal ]; then echo "Error: $goal is not a valid file"; exit 1; fi

# Nice date
d() { date "+%F %H:%M:%S"; }

# GNU time
time=~/bin/gnu_time/bin/time
#time=~/Desktop/time/bin/time
time_format="real: %e\nuser_cpu: %U\nsys_cpu: %S\n"

# Out file format
na=NA  # Default string for R's NA ("not available")
sep="\t"

# Out file initialisation
echo    "# $(d)" >>$out
echo -e "# Complementation:\t\t$algo $opts" >>$out
echo -e "# Memory limit (Java heap):\t$memory" >>$out
echo -e "# Timeout (CPU time):\t\t${timeout}s" >>$out
echo -e "states${sep}t_out${sep}m_out${sep}real_t${sep}cpu_t${sep}file" >>$out

# Log to log file, or log file and screen
log() {
  if [ $log_stdout == true ]; then msg "$@" | tee -a $log
  else msg "$@" >>$log; fi
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

tmp=tmp.out
for filename in $data/*.gff; do
  file=$(basename $filename)

  # Complementation
  log "$(d): Complementing $file... " n
  ulimit -S -t $timeout
  { $time -f "$time_format" $goal complement -m $algo $opts $filename; } &>$tmp
  #timeout ${timeout}s $goal complement -m fribourg $filename &>$tmp # Exit code 124
  ulimit -S -t unlimited

  t_out=0
  m_out=0
  # Timeout
  if grep -q "time limit exceeded" $tmp; then
    log "Timeout"
    t_out=1
    states=$na
  # Memory excess
  elif grep -q "java.lang.OutOfMemoryError" $tmp; then
    log "Memory excess"
    m_out=1
    states=$na
  # Successful execution
  else
    log "Success"
    states=$(grep sid= $tmp | wc -l | tr -d ' ')
  fi

  # Times
  wallclock=$(grep '^real' $tmp | cut -d " " -f 2)
  user_cpu=$(grep '^user' $tmp | cut -d " " -f 2)
  sys_cpu=$(grep '^sys' $tmp | cut -d " " -f 2)
  total_cpu=$(bc <<< "$user_cpu + $sys_cpu")

  # Write out file
  echo -e ${states}${sep}${t_out}${sep}${m_out}${sep}${wallclock}${sep}${total_cpu}${sep}${file} >>$out

done
rm $tmp
