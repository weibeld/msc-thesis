#!/bin/bash

# Wrapper script for executing the GOAL Büchi complementation algorithms in the
# command line mode. The main difference to using GOAL directly from the
# command line, is that this script outputs some statistics for each run of an
# algorithm.
#
# Daniel Weibel, 18.07.2014
#
# Usage: complement.sh <in.gff> <algorithm> [<out.gff>]

indent() {
  sed -l 's/^/    /'
}

if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "Usage:"
  echo "$(basename $0) <in.gff> <algorithm> [<out.gff>]" | indent
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
  echo -e "[<out.gff>]\t\t\tOPTIONAL: save output automaton to file" | indent
  echo "Notes:"
  echo "<algorithm> may include algorithm-specific arguments. In this case," | indent
  echo "enclose the whole parameter in quotes, e.g. \"ramsey -r\". For a description" | indent
  echo "of the algorithms and their arguments, see 'goal help complement'." | indent
  exit 0
fi

if [ "$(which goal)" = "" ]; then
  echo "You seem to not have GOAL installed"
  echo "Make sure 'which goal' points to the GOAL executable"
  exit 1
fi

IN=$1
ALGO=$2
OUT=$3

# If argument $3 was missing, write output automaton to a temp file that is not
# seen by the user. The contents of this file will be displayed to the screen.
if [ "$OUT" = "" ]; then
  OUT=$(mktemp -t goal)
  DISPLAY="true"
fi

# The GOAL command we are going to execute
C="goal complement -m $ALGO -o $OUT $IN"
# The time measurement with 'time' is a bit awkward and only used because on
# Mac, 'date' does just have second but no millisecond output. On Linux, one
# could measure the time before and after the run with 'date +%N'.
TIME_FILE=$(mktemp -t goal)
{ time INFO=$($C 2>&1); } 2>$TIME_FILE
TIME=$(cat $TIME_FILE | grep real | cut -d m -f 2 | tr -d s)


# INFO contains the output of the GOAL command (without the output automaton).
# If the command executed properly, this is "null", if not, it is an error msg.
if [ "$INFO" != "null" ]; then
  echo "$INFO"
  exit 1
fi

# If argument $3 was not given, then output the automaton to the screen
if [ $DISPLAY = "true" ]; then cat $OUT; echo; fi

# Get the number of states of an automaton in a GFF file
states() {
  grep sid= $1 | wc -l | tr -d ' '
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
printf -v X "%.3f" $(echo "e(l($M)/$N)/$N" | bc -l)
echo -e "State growth\tm = (${X}n)^n" | indent
