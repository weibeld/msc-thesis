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
  echo "    $(basename $0) [SUBMIT_OPTIONS] DATA TIMEOUT MEMORY"
  echo
  echo "DESCRIPTION"
  echo "    Executes complement.sh for each subdirectory of current directory."
  echo "    Subdirectories must have the form, e.g., fribourg.m1.m2.15. The first"
  echo "    token is the complementation algorithm, and the following except the"
  echo "    last token are its options. The last token can be arbitrary, but must"
  echo "    be present."
  echo
  echo "ARGUMENTS"
  echo "    DATA, TIMEOUT, and MEMORY are passed to complement.sh, SUBMIT_OPTIONS"
  echo "    are passed to submit.sh."
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

[ $# -eq 0 ] && usage && exit

a=($@)                        # Put all command line args in array
size=${#a[@]}                 # Number of arguments
submit_opts=${a[@]:0:$size-3} # All args except the last three
data=${a[$size-3]}            # Third-last argument
timeout=${a[$size-2]}         # Second-last argument
memory=${a[$size-1]}          # Last argument

test file "$data"

subdirs=($(echo $(pwd)/*/)) # Get all subdirs of current dir (with a trailing slash)
for sub in ${subdirs[@]}; do
  code=$(basename $sub) # $code of the form frib.r.m.foo
  ifs=$IFS; IFS=.; tokens=($code); IFS=$ifs
  algo=${tokens[0]}
  for t in ${tokens[@]:1:${#tokens[@]}-2}; do algo=$algo" -$t"; done
  submit.sh $submit_opts -d $sub ~/scripts/complement/complement.sh -a "$algo" -d $data -t $timeout -m $memory
done
