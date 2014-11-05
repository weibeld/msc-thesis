#!/bin/bash
# Install a plugin from the local machine at a GOAL installation on UBELIX.
# The local plugin has to be already compiled. The local plugin and the remote
# GOAL installation can be specifed with command line options. If they are
# absent, the script uses default values.
#
# dw-03.11.2014

set -e

# Default values
plugin=$(dirname $(which goal))/plugins/ch.unifr.goal.complement
goal_remote=bin/GOAL-20141017

usage() {
  echo "USAGE:"
  echo "    $(basename $0) [-p plugin] [-g GOAL]"
  echo
  echo "ARGUMENTS:                            [DEFAULT]"
  echo "    -p: Plugin directory              [<GOAL>/plugins/ch.unifr.goal.complement]"
  echo "    -g: GOAL directory on UBELIX      [$goal_remote]"
  echo
  echo "NOTE:"
  echo "    GOAL directory can be specified relative to UBELIX home directory"
}
if [ "$1" == -h ]; then usage; exit 0; fi

while getopts ":p:g:" opt; do
  case $opt in
    p) plugin=$OPTARG;      ;;
    g) goal_remote=$OPTARG; ;;
    \?) echo "Error: invalid option: -$OPTARG";             exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done

dest=$goal_remote/plugins

echo "Installing"
echo "    $plugin"
echo "at"
echo "    $dest"
echo "on UBELIX... "

# Pack plugin
plugin_parent=$(dirname $plugin)
plugin_name=$(basename $plugin)
tar=$(mktemp -t plugin)
tar -cz -f $tar -C $plugin_parent $plugin_name >/dev/null

# Copy packed plugin to UBELIX
tar_remote=$dest/$plugin_name.tar.gz
put.sh $tar $tar_remote >/dev/null

# Delete existing plugin on UBELIX. We assume that the plugin to replace has
# the same name as the plugin to install.
run.sh "rm -rf $dest/$plugin_name"

# Unpack new plugin
run.sh "tar -xz -f $tar_remote -C $dest; rm $tar_remote"

# Create new archive of the whole GOAL installation
goal_parent=$(dirname $goal_remote)
goal=$(basename $goal_remote)
run.sh "tar -cz -f $goal_parent/$goal.tar.gz -C $goal_parent $goal"

echo Done
