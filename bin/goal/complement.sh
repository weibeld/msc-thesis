#!/bin/bash

data=~/data/test
goal=~/bin/GOAL-20140808/goal
out=out
log=log
timeout=5   # Seconds
memory=1G
log_screen=true

# Out file format
t=T
m=M
delim="\t"

# Nice date
d() { date "+%F %H:%M:%S"; }

# GNU time
time=~/bin/gnu_time/bin/time
time_format="real: %e\nuser_cpu: %U\nsys_cpu: %S\n"

# Out file initialisation
echo "# $(d)" >>$out
echo -e "# Memory limit (Java heap):\t$memory" >>$out
echo -e "# Timeout (CPU time):\t\t${timeout}s" >>$out
echo -e "States${delim}Sec. R${delim}Sec. C${delim}File" >>$out

# log() {
#   if [ $log_screen == true ]; then
#     if [ "$2" != "" ]; then echo -n "$1" | tee -a $log
#     else echo "$1" | tee -a $log; fi
#   else
#     if [ "$2" != "" ]; then echo -n "$1" >>$log
#     else echo "$1" >>$log; fi
#   fi
# }

log() {
  if [ "$2" != "" ]; then msg="echo -n $1"
  else msg="echo $1"; fi
  if [ $log_screen == true ]; then $msg | tee -a $log
  else $msg >>$log
  fi
}

# Set maximum Java heap size
export JVMARGS="-Xmx$memory"

# Capture Ctrl-C
#trap some_function SIGINT

tmp=tmp.out
for filename in $data/*.gff; do
  file=$(basename $filename)

  # Complementation
  log "$(d): Complementing $file... " n
  ulimit -S -t $timeout
  # { $time -f $time_format $goal complement -m fribourg $filename; } &>$tmp
  { $time -f "$time_format" $goal complement -m fribourg $filename; } &>$tmp
  #timeout ${timeout}s $goal complement -m fribourg $filename &>$tmp # Exit code 124
  ulimit -S -t unlimited

  # Timeout
  if grep -q "time limit exceeded" $tmp; then
    log "Timeout"
    c1=$t
  # Memory excess
  elif grep -q "java.lang.OutOfMemoryError" $tmp; then
    log "Memory excess"
    c1=$m
  # Successful execution
  else
    states=$(grep sid= $tmp | wc -l | tr -d ' ')
    log "Success"
    c1=$states
  fi

  # Times
  wallclock=$(grep '^real' $tmp | cut -d " " -f 2)
  user_cpu=$(grep '^user' $tmp | cut -d " " -f 2)
  sys_cpu=$(grep '^sys' $tmp | cut -d " " -f 2)
  total_cpu=$(bc <<< "$user_cpu + $sys_cpu")

  # Write out file
  echo -e ${c1}${delim}${wallclock}${delim}${total_cpu}${delim}${file} >>$out

done
rm $tmp
