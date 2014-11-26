#!/bin/bash

# Wrapper script for executing the GOAL Büchi complementation algorithms from
# the command line. The main difference to using GOAL directly is that this
# script measures the execution time and displays the number of states of the
# input and output automaton and the state growth. More statistics could be
# added.
#
# Daniel Weibel, 18.07.2014
#
# Usage: complement.sh <in.gff> <algorithm> [<opts>] [<out.gff>]


# Exit if any command returns a non-zero status (i.e. error)
set -e

# Some things have to be handled differently for Mac and Linux
if   [ $(uname -s) = "Darwin" ]; then OS="Mac"
elif [ $(uname -s) = "Linux" ];  then OS="Linux"
else OS="Linux"
fi

indent() {
  sed 's/^/    /'
}

if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ $# -lt 2 ]; then
  echo "Usage:"
  echo "$(basename $0) <in.gff> <algorithm> [<opts>] [<out.gff>]" | indent
  echo "Arguments:"
  echo -e "<in.gff>\t\t\tInput Büchi automaton" | indent
  echo -e "<algorithm>\t\t\tComplementation algorithm" | indent
  echo -e "* classic" | indent | indent
  echo -e "* deterministic" | indent | indent
  echo -e "* fribourg" | indent | indent
  echo -e "* kurshan" | indent | indent
  echo -e "* modifiedsafra" | indent | indent
  echo -e "* ms" | indent | indent
  echo -e "* piterman" | indent | indent
  echo -e "* ramsey" | indent | indent
  echo -e "* rank" | indent | indent
  echo -e "* safra" | indent | indent
  echo -e "* slice" | indent | indent
  echo -e "* waa" | indent | indent
  echo -e "* wapa" | indent | indent
  echo -e "[<opts>]\t\t\tOPTIONAL: algorithm-specific options, see" | indent
  echo -e "\t\t\t\t'goal help complement'" | indent
  echo -e "[<out.gff>]\t\t\tOPTIONAL: save output automaton to file" | indent
  echo "Example:"
  echo "$(basename $0) example.gff rank -r -tr -cp result.gff" | indent
  exit 0
fi

if [ "$(which goal)" == "" ]; then
  echo "ERROR: make sure 'which goal' points to the 'goal' script by, for example,"
  echo "adding the GOAL folder to the PATH"
  exit 1
fi

IN=$1     # Input file
ALGO=$2   # Algorithm
# The algorithm options are all the args following $2 and starting with a '-',
# until either the end of the argument list or until the first argument after $2
# not starting with a '-'. In the latter case, this last arg is the output file.
ARRAY=($@)
for ARG in ${ARRAY[@]:2}; do
  if [ "${ARG:0:1}" = "-" ]; then OPTS=$OPTS" "$ARG
  else OUT=$ARG; break; fi
done

# If no output file was specified, write output automaton to a temp file that is
# not seen by the user. This file will later be displayed to the screen.
if [ "$OUT" = "" ]; then
  if   [ $OS = "Mac" ];   then OUT=$(mktemp -t goal)
  elif [ $OS = "Linux" ]; then OUT=$(mktemp -t goal.XXXXXX); fi
  NO_SAVE="true"
fi

# The GOAL command we are going to execute
C="gc complement -m $ALGO $OPTS -o $OUT $IN"
# Execution of the command including measurement of the execution time
if [ $OS = "Linux" ]; then
  TIME_BEFORE=$(date +%s%N) # Current time in nanoseconds
  INFO=$($C 2>&1)
  TIME_AFTER=$(date +%s%N)  # Current time in nanoseconds
  TIME=$(printf "%.3f" $(echo "($TIME_AFTER-$TIME_BEFORE)/1000000000" | bc -l))
# On Mac, 'date' does only have seconds resolution. That's why we use a more
# awkward way to measure the time with 'time'
elif [ $OS = "Mac" ]; then
  TIME_FILE=$(mktemp -t goal)
  { time INFO=$($C 2>&1); } 2>$TIME_FILE
  TIME=$(cat $TIME_FILE | grep real | cut -d m -f 2 | tr -d s)
fi

# INFO contains the output of the GOAL command (without the output automaton).
# If the command executed properly, this is "null", if not, it is an error msg.
if [ "$INFO" != "null" ]; then
  echo "$INFO"
  exit 1
fi

# If no output file was specified, output automaton from temp file to the screen
if [ "$NO_SAVE" = "true" ]; then cat $OUT; echo; fi

# Get the number of states of an automaton in a GFF file
states() {
  grep -o "sid=" $1 | wc -l | tr -d ' '
}

# Execution time
echo -e "Execution time\t\t$TIME seconds"
# Size (i.e. number of states) of input automaton
echo "Input automaton"
N=$(states $IN)
echo -e "States (n)\t\t$N" | indent
# Size (i.e. number of states) of output automaton
echo "Output automaton"
M=$(states $OUT)
echo -e "States (m)\t\t$M" | indent
# State growth from the input (n) to the output (m) automaton
# We want to have n and m in a relation of the form m = (xn)^n. To find x, we
# have to calculate x = nth_root(m)/n. The nth root of m can be calculated by
# e^(ln(m)/n), thus x = e^(ln(m)/n)/n.
X=$(printf "%.3f" $(echo "e(l($M)/$N)/$N" | bc -l))
echo -e "State growth\tm = (${X}n)^n" | indent
