#!/bin/bash
# Wrapper for the qsub command.
# dw-22.09.2014

set -e

usage() {
  echo "USAGE"
  echo "    $(basename $0) [OPTIONS] JOB_SCRIPT [ARGS]"
  echo
  echo "OPTIONS                     [DEFAULT]"
  echo "    -m: Memory              [4G]"
  echo "    -c: CPU time            [01:00:00]"
  echo "    -d: Execution directory [\$(pwd)]"
  echo "    -o: STDOUT file         [stdout]"
  echo "    -e: STDERR file         [stderr]"
  echo "    -q: Queue               [short.q]"
  echo "    -s: Scratch             [1G]       (set to 'off' to disable)"
  echo "    -p: Parallel env. SMP   [4]        (set to 'off' to disable)"
  echo "    -n: E-mail notification [n]"
  echo "        b: E-mail is sent at beginning of job"
  echo "        e: E-mail is sent at end of job"
  echo "        a: E-mail is sent when job is aborted or rescheduled"
  echo "        s: E-mail is sent when job is suspended"
  echo "        n: No e-mails are sent"
}

test() {
  kind=$1 # "file" or "directory"
  path=$2
  if [ ! -${kind:0:1} "$path" ]; then echo "Error: invalid $kind ($path)"; exit 1; fi
  if [ ${path:0:1} != "/" ]; then echo "Error: must specify absolute path ($path)"; exit 1; fi
}

if [ $# -eq 0 ] || [ "$1" = -h ]; then usage; exit 0; fi

while getopts ":m:c:d:o:e:q:s:p:n:" opt; do
  case $opt in
    m) memory=$OPTARG; ;;
    c) cpu_time=$OPTARG; ;;
    d) dir=$(sed 's/\/$//'<<<$OPTARG); ;;
    o) out_file=$OPTARG; ;;
    e) err_file=$OPTARG; ;;
    q) queue=$OPTARG; ;;
    s) scratch_size=$OPTARG; ;;
    p) slots=$OPTARG; ;;
    n) notification=$OPTARG; ;;
    \?) echo "Error: invalid option: -$OPTARG"; exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done

shift $(($OPTIND-1))
script=$(eval echo $1) # Enforece tilde expansion
test file "$script"

shift # $@ contains job script arguments (if any)

if [ -z $memory ]; then memory=4G; fi
if [ -z $cpu_time ]; then cpu_time=01:00:00; fi
if [ -z $dir ]; then dir=$(pwd); else dir=$(eval echo $dir); test directory $dir; fi
if [ -z $out_file ]; then out_file=stdout; fi
if [ -z $err_file ]; then err_file=stderr; fi
if [ -z $notification ]; then notification=n; fi
if [ -z $queue ]; then queue=short.q; fi

if [ -z $scratch_size ]; then scratch_size=1G; fi
if [ $scratch_size = off ]; then scratch=""
else scratch="-l scratch=1,scratch_size=$scratch_size,scratch_files=1M "; fi

if [ -z $slots ]; then slots=4; fi
if [ $slots = off ]; then pe=""
else pe="-pe smp $slots -R y "; fi

job=$(basename $dir)
email=daniel.weibel@unifr.ch

cmd=\
"qsub "\
"-wd $dir "\
"-V "\
"-M $email "\
"-m $notification "\
"-l h_cpu=$cpu_time,h_vmem=$memory "\
"$scratch"\
"$pe"\
"-q $queue "\
"-o $out_file "\
"-e $err_file "\
"-N $job "\
"$script"

# Qsub execution with $@ args to the job script
$cmd "$@"

# Save issued qsub command
args=("$@")
for a in "${args[@]}"; do
  # Enclose multi-word args in \"
  if [ $(wc -w <<< $a) -gt 1 ]; then a=$(sed -e 's/^/\"/' -e 's/$/\"/' <<< $a); fi
  arg_string=$arg_string" "$a
done
echo $cmd $arg_string >$dir/qsub
