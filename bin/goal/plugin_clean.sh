#!/bin/bash

# Removes everything from the $GOAL_DIR/plugins folder except the GOAL default
# plugins, which are org.svvrl.goal.core, org.svvrl.goal.gui, and
# org.svvrl.goal.cmd. Assumes that the 'goal' command is in the PATH.
#
# Daniel Weibel, 26.05.2014
#
# Usage: ./plugin_clean.sh


# Exit if any command returns a non-zero status (i.e. error)
set -e

if [ $# -gt 0 ]; then
  echo "Usage:"
  echo "    $(basename $0)"
  echo "Description:"
  echo "    Deletes all the user-added plugins from GOAL."
  exit 0
fi 

if [ "$(which goal)" == "" ]; then
  echo "ERROR: make sure 'which goal' points to the GOAL launching script by, for"
  echo "example, adding the GOAL system folder to the PATH"
  exit 1
fi

GOAL_DIR=$(dirname $(which goal))

# Select every file/folder that doesn't contain org.svvrl.goal.
for PLUGIN in $(ls $GOAL_DIR/plugins | sed '/org.svvrl.goal/d'); do
  rm -rf $GOAL_DIR/plugins/$PLUGIN
done

echo "Removed all custom plugins from $GOAL_DIR/plugins"
