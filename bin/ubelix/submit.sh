#!/bin/bash
# dw-12.09.2014

usage() {
  echo "Usage"
  echo "    $0 [OPTONS] JOB_SCRIPT"
  echo
  echo "OPTIONS                     [Default]"
  echo "    -m: Memory              [4G]"
  echo "    -c: CPU time            [01:00:00]"
  echo "    -o: STDOUT file         [<job>.out]"
  echo "    -e: STDERR file         [<job>.err]"
  echo "    -s: Scratch size        [2G]         (set to 0 if no scratch desired)"
  echo "    -p: PE SMP slots        [2]          (set to 0 if no PE desired)"
  echo "    -n: E-mail notification [a]"
  echo "        b: E-mail is sent at beginning of job"
  echo "        e: E-mail is sent at end of job"
  echo "        a: E-mail is sent when job is aborted or rescheduled"
  echo "        s: E-mail is sent when job is suspended"
  echo "        n: No e-mails are sent"
}

while getopts ":m:c:r:o:e:s:p:n:" opt; do
  case $opt in
    m) memory=$OPTARG; ;;
    c) cpu_time=$OPTARG; ;;
    o) out_file=$OPTARG; ;;
    e) err_file=$OPTARG; ;;
    s) scratch_size=$OPTARG; ;;
    p) slots=$OPTARG; ;;
    n) notification=$OPTARG; ;;
    \?)
      echo "Error: invalid option: -$OPTARG"
      exit 1
      ;;
    :)
      echo "Error: option -$OPTARG requires an argument"
      exit 1
      ;;
  esac
done

shift $(($OPTIND - 1))
if [ $# -ne 1 ]; then
  usage
  exit 1
fi

script=$1
if [ ! -f $script ]; then echo "Error: $script is not a valid file"; exit 1; fi

job=$(basename $(dirname $script))

if [ -z $memory ]; then memory=4G; fi
if [ -z $cpu_time ]; then cpu_time=01:00:00; fi
if [ -z $out_file ]; then out_file=$job.out; fi
if [ -z $err_file ]; then err_file=$job.err; fi
if [ -z $notification ]; then notification=a; fi

if [ -z $scratch_size ]; then scratch_size=2G; fi
if [ $scratch_size == 0 ]; then scratch=""
else scratch="-l scratch=1,scratch_size=$scratch_size,scratch_files=1M "; fi

if [ -z $slots ]; then slots=2; fi
if [ $slots == 0 ]; then pe=""
else pe="-pe smp $slots -R y "; fi

email=daniel.weibel@unifr.ch

cd $(dirname $script)

cmd=\
"qsub "\
"-cwd "\
"-V "\
"-M $email "\
"-m $notification "\
"-l h_cpu=$cpu_time,h_vmem=$memory "\
"$scratch"\
"$pe"\
"-o $out_file "\
"-e $err_file "\
"-N $job "\
"$(basename $script)"

$cmd

echo $cmd >qsub
