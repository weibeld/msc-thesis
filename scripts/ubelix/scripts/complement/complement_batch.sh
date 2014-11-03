#!/bin/bash
# Execute a batch of complementation experiments, each one in a subdirectory
# of the DIRECTORY argument. The complementation algorithm and options are
# deduced from the name of each subdirectory. This script is basically an
# automated repeated invocation of submit.sh with different arguments.
#
# dw-09.10.2014

set -e

usage() {
  echo "USAGE"
  echo "    $(basename $0) [SUBMIT_OPTIONS] DIRECTORY DATA"
  echo
  echo "DIRECTORY and DATA"
  echo "    If DIRECTORY contains subdir fribourg.r.m.X, then the script submits"
  echo "       complement.sh -a \"fribourg -r -m\" -d DATA from within fribourg.r.m.X"
  echo "       ...and so on for all the other subdirectories in DIRECTORY."
  echo "    The last token of a subdirectory name (X) is ignored but must be present."
  echo
  echo "SUBMIT_OPTIONS              [DEFAULT]"
  echo "    -m: Memory              [4G]"
  echo "    -c: CPU time            [01:00:00]"
  echo "    -o: STDOUT file         [stdout]"
  echo "    -e: STDERR file         [stderr]"
  echo "    -q: Queue               [all.q]"
  echo "    -s: Scratch             [off]        (set to e.g. [2G] to activate)"
  echo "    -p: Parallel env. SMP   [off]        (set to e.g. [2] slots to activate)"
  echo "    -n: E-mail notification [a]"
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

if [ $# -eq 0 ]; then usage; exit 0; fi

a=($@)                        # Put all command line args in array
size=${#a[@]}                 # Number of arguments
submit_opts=${a[@]:0:$size-2} # All args except the last two
dir=${a[$size-2]}             # Second-last argument
data=${a[$size-1]}            # Last argument

# dir=$(eval echo $dir)   # Enforce tilde expansion
# data=$(eval echo $data) # Enforce tilde expansion
dir=$(echo $dir | sed 's/\/$//')
test directory "$dir"
test file "$data"

subdirs=($(echo $dir/*/)) # Get all subdirs of $dir (with a trailing slash)
for sub in ${subdirs[@]}; do
  code=$(basename $sub) # $code of the form frib.r.m.foo
  IFS=. tokens=($code)
  algo=${tokens[0]}
  for t in ${tokens[@]:1:${#tokens[@]}-2}; do algo=$algo" -$t"; done
  submit_full="$submit_opts -d $sub"
  ~/submit.sh $submit_full ~/scripts/complement.sh -a "$algo" -d $data
done
