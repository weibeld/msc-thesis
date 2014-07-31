#!/bin/bash

# Reverts the changes made by nailgun_on.sh. In particular, stops the Nailgun
# server and reverts the goal script to the original one.
#
# Daniel Weibel, 23.07.2014
#
# Usage: ./nailgun_off.sh


if [ $# -gt 0 ]; then
  echo "Usage:"
  echo "    $(basename $0)"
  echo " Description:"
  echo "    Reverts the changes made by nailgun_on.sh. In particular, stops the Nailgun"
  echo "    server, and reverts the goal script ('which goal') to the original one."
  exit 0
fi

# If either the GOAL folder isn't in the PATH, or the goal script doesn't exist
if [ "$(which goal)" = "" ]; then
  echo "There seems to be no 'goal' script"
  exit 1
fi

GOAL_DIR=$(dirname $(which goal))
BACKUP=goal.original

# If no Nailgun server is running
if [ "$(ps -e | grep NGServe[r])" = "" ]; then
  echo "No Nailgun server is running"
else
  echo "Stopping Nailgun server..."
  ng ng-stop  # Returns a non-zero exit value
fi

# Restore backup of original goal script to file 'goal'
if [ -f $GOAL_DIR/$BACKUP ]; then
  echo "Restoring original goal script ('which goal')..."
  mv $GOAL_DIR/$BACKUP $GOAL_DIR/goal
fi
