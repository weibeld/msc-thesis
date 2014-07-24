#!/bin/bash
# dw-26.05.2014

# Compiles and installs a JPF plugin for GOAL. Can be executed from anywhere.
# Assumes that the name of the plugin folder is the package name of the Java
# package containing the plugin (this is a convention). Use plugin_generate.sh
# to generate a working example GOAL plugin. Assumes that the 'goal' command
# is in the PATH.
#
# A typical plugin has the structure:
# ch.unifr.goal.myplugin/
#     plugin.xml
#     sources/
#         ch/unifr/goal/myplugin/<.java files>
#     classes/
#         ch/unifr/goal/myplugin/<.class files>   (after compilation)
#
# Usage: ./plugin_build.sh <directory>


# Exit if any command returns a non-zero status (i.e. error)
set -e

if [ $# -ne 1 ]; then
  echo "Usage:"
  echo "    $(basename $0) <directory>"
  echo "Notes:"
  echo "    <directory> is the directory of the GOAL plugin to be compiled and"
  echo "    installed."
  exit 0
fi

if [ "$(which goal)" == "" ]; then
  echo "Error: you must add the GOAL folder (containing plugins/) to your PATH."
  echo "Aborting"
  exit 0
fi

PLUGIN_DIR=$1
cd $PLUGIN_DIR

# Get path to the directory containing the source files, relative to the plugin
# directory. The plugin directory is per convention the package name of the
# plugin code. I.e. the plugin in directory ch.unifr.example has its source
# files in sources/ch/unifr/example (relative to ch.unifr.example).
PLUGIN_NAME=$(basename $(pwd))
IFS='.' read -a ARRAY <<< $PLUGIN_NAME
SOURCE_PATH='sources'
for ELT in ${ARRAY[@]}; do SOURCE_PATH=$SOURCE_PATH/$ELT; done

# The standard GOAL plugins that must be added to the classpath for compilation
GOAL_DIR=$(dirname $(which goal))
CORE=$GOAL_DIR/plugins/org.svvrl.goal.core
CMD=$GOAL_DIR/plugins/org.svvrl.goal.cmd
GUI=$GOAL_DIR/plugins/org.svvrl.goal.gui

# The GOAL plugins are zipped by default, but must be present in unzipped to be
# accessible by the Java compiler. Unzip them if they aren't yet.
if [ ! -d $CORE ]; then unzip ${CORE}.zip -d $CORE >/dev/null; fi
if [ ! -d $CMD ]; then unzip ${CMD}.zip -d $CMD >/dev/null; fi
if [ ! -d $GUI ]; then unzip ${GUI}.zip -d $GUI >/dev/null; fi


# Compilation
# -----------
CLASSPATH=$CORE/classes:$CMD/classes:$GUI/classes

# The existing GOAL classes seem to have been compiled with and for Java 1.6.
# That means that new plugin code has to be compiled for Java 1.6 too
# (I encountered runtime problems when compiling a plugin for Java 1.7). This
# requires setting -source and -target and the bootstrap classpath. The
# bootstrap classpath should probably actually be an rt.jar of a 1.6 JDK.
# However, I just have a 1.7 JDK, and using it helps at leat omit a warning.
# ADAPT THIS (according to your system):
BOOTSTRAP_CLASSPATH=/Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home/jre/lib/rt.jar

echo "Compiling sources..."
javac -classpath $CLASSPATH \
  -Xbootclasspath:$BOOTSTRAP_CLASSPATH -source 1.6 -target 1.6 \
  -d classes $SOURCE_PATH/*.java
echo "Done"


# Installation
# ------------
echo "Installing plugin at $GOAL_DIR/plugins"
# Remove possible slash (/) at end of PLUGIN_DIR. This is necessary for cp -r
# to copy the folder and not the content.
PLUGIN_DIR=$(dirname $PLUGIN_DIR)/$(basename $PLUGIN_DIR)
cp -r $PLUGIN_DIR $GOAL_DIR/plugins




