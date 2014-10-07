#!/bin/bash

set -e

# reps=$1 and if $1 not set, then reps=5
reps=${1-5}

# Limit Java heap size, otherwise Java uses more than the usually 4G granted to the job
export JVMARGS="-Xmx6G -Xms1G"

# Copy GOAL to local scratch
cp ~/bin/GOAL-20130711.tar.gz $TMP/tmp.tar.gz
goal_dir=$TMP/GOAL
mkdir $goal_dir
tar xzf $TMP/tmp.tar.gz -C $goal_dir --strip-components 1 && rm $TMP/tmp.tar.gz
goal=$goal_dir/goal

# Execute the compl_equiv GOAL script which must be already in the working dir
$goal batch compl_equiv.gs $reps
