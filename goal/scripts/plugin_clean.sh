#!/bin/bash
# dw-26.05.2014

# Removes everything from the $GOAL_DIR/plugins folder except the GOAL default
# plugins. Assumes that the 'goal' command is in the PATH.
#
# Usage: ./plugin_clear.sh

if [ "$(which goal)" == "" ]; then
  echo "Error: you must add the GOAL folder (containing plugins/) to your PATH."
  echo "Aborting"
  exit 0
fi

GOAL_DIR=$(dirname $(which goal))

# Select every file/folder that doesn't contain org.svvrl.goal.
for PLUGIN in $(ls $GOAL_DIR/plugins | sed '/org.svvrl.goal/d'); do
  rm -rf $GOAL_DIR/plugins/$PLUGIN
done

echo "Removed all custom plugins from $GOAL_DIR/plugins"