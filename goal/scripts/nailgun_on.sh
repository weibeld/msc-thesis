#!/bin/bash

# Backs up and replaces the GOAL launching script (the one from 'which goal')
# with a new script that starts GOAL on the Nailgun server. Then starts the
# Nailgun server, set up for the execution of GOAL.
#
# The idea is that then starting GOAL, either from the command line or via the
# GUI, is done with Nailgun so that the Java VM startup delay is dispensed with.
# However, in this case a command like 'goal complement automaton.gff' yields an
# error, which apparently has to do with the parsing of the GFF file. Also
# starting GOAL by clicking on the application icon doesn't seem to be faster.
#
# Always call nailgun_off.sh to revert the changes done by nailgun_on.sh.
#
# Usage: ./nailgun_on.sh
#
# Daniel Weibel, 22.07.2014

if [ $# -gt 0 ]; then
  echo "Usage:"
  echo "    $(basename $0)"
  echo " Description:"
  echo "    Backs up and replaces the GOAL launching script ('which goal') with a new"
  echo "    script that starts GOAL on the Nailgun server. Then launches the Nailgun"
  echo "    server. Call nailgun_off.sh to revert the changes made by this script."
  exit 0
fi

# If the Nailgun client (ng) is not installed
if [ "$(which ng)" = "" ]; then
  echo "ERROR: You don't seem to have Nailgun installed"
  echo "Get and install it from https://github.com/martylamb/nailgun"
  exit 1
fi

# If a Nailgun server is already running
if [ "$(ps -e | grep NGServe[r])" != "" ]; then
  echo "ERROR: A Nailgun server seems to be already running"
  echo "See 'ps -e | grep NGServer', and try to stop this server first"
  exit 1
fi

# Directory and file names
GOAL_DIR=$(dirname $(which goal))
GOAL_JAR=$GOAL_DIR/lib/jpf-boot.jar
# ADAPT THIS TO THE LOCATION OF THE NAILGUN SERVER JAR ON YOUR SYSTEM
NAILGUN_SERVER=$HOME/bin/nailgun-server-0.9.2-SNAPSHOT.jar

# The filename for the backup of the original goal script
BACKUP=goal.original

# If the backup of the original goal script is present without a Nailgun server
# running. This can happen if the computer has been shut down without calling
# nailgun_off.sh beforehand, or if the server has been stopped with ng ng-stop
# instead of with nailgun_off.sh. 
if [ -f $GOAL_DIR/$BACKUP ]; then
  mv $GOAL_DIR/$BACKUP $GOAL_DIR/goal
fi

# Backup the original goal script and replace it with a new one that uses the
# Nailgun client (ng) to start GOAL on the Nailgun server
echo "Backing up and replacing original goal script ('which goal')..."
mv $GOAL_DIR/goal $GOAL_DIR/$BACKUP
echo '#!/bin/bash' > $GOAL_DIR/goal
echo 'ng org.java.plugin.boot.Boot "$@"' >> $GOAL_DIR/goal
chmod +x $GOAL_DIR/goal

# Start the Nailgun server (class NGServer) with all the Java Virtual Machine
# options that are needed for the execution of GOAL. The options are the same
# as in the original goal script.
echo "Starting Nailgun server..."
java \
-DapplicationRoot="$GOAL_DIR" \
-Dorg.java.plugin.boot.pluginsRepositories="$GOAL_DIR/plugins" \
-Djpf.boot.config="$GOAL_DIR/boot.properties" \
-Dorg.apache.commons.logging.Log=org.apache.commons.logging.impl.SimpleLog \
-Dorg.apache.commons.logging.simplelog.log.org.java.plugin=fatal \
-classpath $NAILGUN_SERVER:$GOAL_JAR \
com.martiansoftware.nailgun.NGServer >/dev/null &
#-classpath $NAILGUN_SERVER:$GOAL_JAR:$GOAL_DIR:$GOAL_DIR/plugins:$GOAL_DIR/lib \

echo "---> Run nailgun_off.sh to revert the changes made by this script"
