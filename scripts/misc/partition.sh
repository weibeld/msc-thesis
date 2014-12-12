#!/usr/bin/env bash
# Divide a GOAL test set (11'000 automata of size 15 or 20) into n partitions.
# n can be chosen by the user. If, for example, n=5, then there will be 5 part-
# itions of size 2200, with Partition 1 containing automata 1 to 2200,
# Partition 2 automata 2201 to 4400, etc. (based on the file listing of ls).
# Every partition is put in a folder, packed as a .tar.gz file.
#
# Daniel Weibel, 07.12.2014

set -e

# Command line argument defaults
n=5
indir=~/Desktop/thesis/goal/testset/15
outdir=~/Desktop

usage() {
  echo "USAGE"
  echo "    $(basename $0) [-n number] [-i dir] [-o dir]"
  echo
  echo "ARGUMENTS                             [DEFAULT VALUES]"
  echo "    -n: Number of partitions (>=2)    [$n]"
  echo "    -i: Directory of the test set     [$indir]"
  echo "    -o: Output directory              [$outdir]"
}
if [[ "$1" == -h ]]; then usage; exit; fi

while getopts ":n:i:o:" opt; do
  case $opt in
    n) n=$OPTARG;   ;;
    i) indir=$OPTARG;  ;;
    o) outdir=$OPTARG; ;;
    \?) echo "Error: invalid option: -$OPTARG";             exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done
if [[ "$n" -lt 2 ]]; then echo "Error: -n must be >= 2"; exit 1; fi

# Count files in test set
((total=$(ls $indir | wc -l)))
echo "Detected $total files in $indir"

# Number of files of one partiton, more precisely of the first n-1 partitions.
# If $total/$n is not an integer, then the decimal part is discarded and the
# resulting integer is the size of the first n-1 partitions. The last partition
# will contain all the remaining files. E.g. if total=11000 and n=3, then
# 11000/3=3666.6667, and thus P1=3666, P2=3666, P3=3668.
part_size=$(($total/$n))

cd $indir   # It's easier to do everything with basenames only
files=$(ls) # The basenames of all the files in the test set (file list)

for i in $(seq 1 $n); do
  # Padded version of i, e.g. if n=10 and i=9, then i_padded=09
  digits=$(($(echo -n $n | wc -c))) i_padded=$(printf "%0${digits}d" $i)
  echo -n "Creating partition $i_padded of $n... "
  # Directory containing the partition, will be zipped
  part_dir=$outdir/$(basename $indir).${i_padded}_of_$n
  mkdir $part_dir
  # The first n-1 partitions
  if [[ $i -lt $n ]]; then
    # Names of the files of this partition (first $part_size files of file list)
    part_files=$(echo "$files" | sed -n "1,${part_size}p")
    # Remove copied files from file list so we can always count from beginning
    files=$(echo "$files" | sed "1,${part_size}d")
    # Copy partition files to partition dir (see comment on xargs below)
    echo $part_files | xargs -J % cp % $part_dir
    size=$part_size
  # The last partition
  else
    echo $files | xargs -J % cp % $part_dir
    size=$(($(echo "$files" | wc -l)))
  fi
  # Zip the partition directory and clean up
  tar czf $part_dir.tar.gz -C $outdir ${part_dir##*/}
  rm -rf $part_dir
  echo "done ($size files in ${part_dir##*/}.tar.gz)"
done

# Note on xargs
# For copying the files in $part_files to $part dir, we could simply do
# cp $part_files $part_dir. However, there's the problem that if $part_file is
# very big, then the argument list of cp might exceed the maximally allowed
# argument list length for new processes (ARG_MAX). The error message "-bash:
# /bin/ls: Argument list too long" appears. getconf ARG_MAX shows this limit
# (in bytes). This limit only applies for *new processes* started by the shell.
# It is imposed by the exec() system call. echo is a shell builtin and thus no
# new process is started. That's why we can do "echo $part_files" but not
# "cp $part_files" (on ARG_MAX: http://www.in-ulm.de/~mascheck/various/argmax/).
# xargs reads from stdin and feeds chunks, that are smaller than ARG_MAX, to the 
# command supplied as argument (in our case cp). This solves the problem of
# exceeding ARG_MAX. The option -J defines a token (%) that will be replaced by
# every chunk from stdin in the following command string. In our case, every
# chunk of $part_files is put at the place of the % in "cp % $part_dir", and
# then cp is executed.
