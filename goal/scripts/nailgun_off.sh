#!/bin/bash

# Reverts the changes made by nailgun_on.sh. In particular, stops the Nailgun
# server and reverts the goal script to the original one.
#
# Daniel Weibel, 23.07.2014
#
# Usage: ./nailgun_off.sh


# Exit if any command returns a non-zero status (i.e. error)
set -e

if [ $# -gt 0 ]; then
  echo "Usage:"
  echo "    $(basename $0)"
  echo " Description:"
  echo "    Reverts the changes made by nailgun_on.sh. In particular, stops the Nailgun"
  echo "    server, and reverts the goal script ('which goal') to the original one."
  exit 0
fi

GOAL_DIR=$(dirname $(which goal))
BACKUP=goal.original

# If no Nailgun server is running
if [ "$(ps -e | grep NGServe[r])" = "" ]; then
  echo "No Nailgun server is running"
else
  echo "Stopping Nailgun server..."
  ng ng-stop
fi

if [ -f $GOAL_DIR/$BACKUP ]; then
  echo "Restoring original goal script ('which goal')..."
  mv $GOAL_DIR/$BACKUP $GOAL_DIR/goal
fi
