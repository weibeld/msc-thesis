#!/bin/bash
# SGE Job Script (use with 4G memory)
# Test for all the automata in a .tar.gz archive whether they are universal
# (accept all the possible words over an alphabet) or not. The test is done
# by complementation and testing of the complement for emptiness. If the
# complement is empty, the inital automaton is universal. As the complementation
# construction, we use piterman with all the optimisations. The results are used
# for the analysis of the test set of the 11'000 automata of Tsai et al.
# dw-28.11.2014

goal_archive=~/bin/GOAL-20141117.tar.gz # GOAL executables
data_archive=~/data/15.tar.gz           # Default data
memory=2G                               # Java heap size (must be enough!)

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-d data]"
  echo
  echo "ARGUMENTS                               [DEFAULT]"
  echo "    -d: Data (absolute path only)       [$data_archive]"  
}
if [ "$1" == "-h" ]; then usage; exit; fi

while getopts ":d:" opt; do
  case $opt in
    d) data_archive=$OPTARG; ;;
    \?) echo "Error: invalid option: -$OPTARG";             exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done

# Test for absolute path and file existence
test_file() {
  path=$1
  if [ ! -f $path ]; then echo "Error: \"$path\" is not a valid file"; exit 1; fi
  if [ ${path:0:1} != "/" ]; then echo "Error: must specify absolute paths"; exit 1; fi
}
test_file $data_archive

# Copy data to local scratch
cp $data_archive $TMP/tmp.tar.gz
data=$TMP/data
mkdir $data
tar xzf $TMP/tmp.tar.gz -C $data --strip-components 1 && rm $TMP/tmp.tar.gz

# Copy GOAL to local scratch
cp $goal_archive $TMP/tmp.tar.gz
goal_dir=$TMP/GOAL
mkdir $goal_dir
tar xzf $TMP/tmp.tar.gz -C $goal_dir --strip-components 1 && rm $TMP/tmp.tar.gz
goal=$goal_dir/gc

# Result file
out=$TMP/$JOB_NAME.out

# Result file header
cat >$out <<EOF
# Starting date:             $(date "+%F %H:%M:%S")
# Cluster job ID:            $JOB_ID
# Cluster node:              $HOSTNAME
# Cluster queue:             $QUEUE
# Data:                      $(basename $data_archive)
# Memory limit (Java heap):  $memory
# Timeout (CPU time):        No timeout
EOF
# Column headers
echo -e "univ\tfile" >>$out

# Set maximum and initial Java heap size. We don't set a timeout. We want all
# tasks to finish. This should be possible with piterman and our test set.
export JVMARGS="-Xmx$memory -Xms$memory"
algo="piterman -eq -macc -sim -ro -r"
complement=$TMP/complement.gff
errors=$TMP/err

# Test if something has been written to stderr (e.g. memory excess)
test_errors() {
  [ -s $errors ] && echo Error >>$out && exit 1
}

# Iterate through all the automata and do the universality test
for automaton in $data/*.gff; do
  $goal complement -m $algo $automaton >$complement 2>$errors
  test_errors
  if [ $($goal emptiness $complement 2>$errors) = true ]
  then test_errors; universal=Y
  else test_errors; universal=N; fi
  echo -e "$universal\t$(basename $automaton)" >>$out
done

# Copy result file from local scratch to job directory
cp $out .
