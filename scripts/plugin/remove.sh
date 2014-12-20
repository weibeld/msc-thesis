#!/bin/bash

# Remove an installed plugin from GOAL.
#
# Daniel Weibel, 20 Dec. 2014


set -e

# Default value
goal_dir=$th/goal/GOAL.app/Contents/Resources/Java

usage() {
  echo "USAGE"
  echo "    $(basename $0) -p plugin [-g GOAL]"
  echo
  echo "DESCRIPTION"
  echo "    Removes the specified plugin from GOAL."
  echo
  echo "ARGUMENTS"
  echo "    -p  Plugin name    (e.g. ch.unifr.goal.util)"
  echo "    -g  GOAL directory (DEFAULT: $goal_dir)"
}
if [[ $# -eq 0 ]] || [[ "$1" = -h ]]; then usage; exit; fi

while getopts ":p:g:" opt; do
  case $opt in
    p) plugin_name=${OPTARG}; ;;
    g) goal_dir=${OPTARG%/};  ;;
    \?) echo "Error: invalid option: -$OPTARG";             exit 1 ;;
    :)  echo "Error: option -$OPTARG requires an argument"; exit 1 ;;
  esac
done

if [[ "$plugin_name" = "" ]]; then
  echo "Error: option -p is mandatory"
  exit 1
fi

if [[ ! -d "$goal_dir" ]]; then
  echo "Error: $goal_dir is not a valid directory"
  exit 1
fi

plugin_dir=$goal_dir/plugins/$plugin_name

if [[ ! -d "$plugin_dir" ]]; then
  echo "Error: plugin $plugin_name does not exist"
  exit 1
fi

rm -rf "$plugin_dir"

echo "Removed plugin $plugin_name"
