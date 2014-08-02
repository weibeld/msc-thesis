#!/bin/bash

# Compiles and installs a JPF plugin for GOAL. Can be executed from anywhere.
# Assumes that the name of the plugin folder is the package name of the Java
# package containing the plugin (this is a convention). Use plugin_generate.sh
# to generate a working example GOAL plugin. Assumes that the 'goal' command
# is in the PATH.
#
# A typical JPF (Java Plugin Framework) plugin has the structure:
# ch.unifr.goal.myplugin/
#     plugin.xml
#     sources/
#         ch/unifr/goal/myplugin/<.java files>
#     classes/
#         ch/unifr/goal/myplugin/<.class files>   (after compilation)
#
# Daniel Weibel, 26.05.2014
#
# Usage: ./plugin_build.sh <directory> [compileonly]


# Exit if any command returns a non-zero status (i.e. error)
set -e

if [ "$1" = "-h" ] || [ "$1" = "help" ] || [ $# -lt 1 ] || [ $# -gt 2 ]; then
  echo "Usage:"
  echo "    $(basename $0) <directory> [compileonly]"
  echo "Arguments:"
  echo -e "    <directory>\t\tRoot directory of the plugin to compile and install"
  echo -e "    [compileonly]\tOPTIONAL: if present, just compiles the source code,"
  echo -e "    \t\t\tbut doesn't install the plugin"
  echo "Description:"
  echo "    Compiles the GOAL plugin located in <directory> and installs it in the"
  echo "    GOAL system folder. The next run of GOAL will include the plugin. For"
  echo "    removing all user-added plugins from GOAL, you can use plugin_clean.sh."
  echo "Note:"
  echo "    If there is a compiler warning of the form \"bootstrap class path not"
  echo "    set in conjunction with -source 1.6\", you can get rid of it by setting"
  echo "    the -Xbootclasspath option for the javac command in the script."
  exit 0
fi

if [ "$(which goal)" == "" ]; then
  echo "ERROR: make sure 'which goal' points to the 'goal' script by, for example,"
  echo "adding the GOAL folder to the PATH"
  exit 1
fi

PLUGIN_DIR=$1

# Get path to the directory containing the source files, relative to the plugin
# directory. The plugin directory is per convention the package name of the
# plugin code. I.e. the plugin in directory ch.unifr.example has its source
# files in ch.unifr.example/sources/ch/unifr/example.
PLUGIN_NAME=$(basename $PLUGIN_DIR)
IFS='.' read -a ARRAY <<< $PLUGIN_NAME
SOURCE_PATH="sources"
for ELT in ${ARRAY[@]}; do SOURCE_PATH=$SOURCE_PATH/$ELT; done

# The standard GOAL plugins that must be added to the classpath for compilation
GOAL_DIR=$(dirname $(which goal))
CORE=$GOAL_DIR/plugins/org.svvrl.goal.core
CMD=$GOAL_DIR/plugins/org.svvrl.goal.cmd
GUI=$GOAL_DIR/plugins/org.svvrl.goal.gui

# The GOAL plugins are zipped by default, but must be present unzipped to be
# accessible by the Java compiler. Unzip them if they aren't yet.
if [ ! -d $CORE ]; then { unzip $CORE.zip -d $CORE; rm $CORE.zip; } >/dev/null; fi
if [ ! -d $CMD ]; then { unzip $CMD.zip -d $CMD; rm $CMD.zip; } >/dev/null; fi
if [ ! -d $GUI ]; then { unzip $GUI.zip -d $GUI; rm $GUI.zip; } >/dev/null; fi


# Compilation
# -----------
CLASSPATH=$CORE/classes:$CMD/classes:$GUI/classes
echo -n "Compiling sources... "
# The GOAL binaries, as obtained from the GOAL website, have been compiled for
# Java 1.6. This mean that the code for an additional plugin has to be compiled
# for Java 1.6 as well. (I encountered runtime problems when compiling the
# plugin for Java 1.7.) Compiling for a specific Java version can be done by
# setting the javac options -source and -target. Additionally, there is the
# option -Xbootclasspath: that has to be set to the rt.jar of a Java 1.6
# runtime environment. However, if the -Xbootclasspath: is omitted, only a
# compiler warning results, but the compilation still seems to work correctly.
# So, setting the -Xbootclasspath: can be seen as optional.
WITH_XBOOTLASSPATH=true
if [ "$WITH_XBOOTLASSPATH" = true ]; then # No compiler warning
  # Adapt this to the location of a Java 1.6 rt.jar on your system
  # Example for Mac:
  BOOTSTRAP_CLASSPATH=/Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home/jre/lib/rt.jar
  # Example for Linux/Debian:
  # BOOTSTRAP_CLASSPATH=/usr/lib/jvm/java-6-openjdk-i386/jre/lib/rt.jar
  javac -classpath $CLASSPATH \
    -Xbootclasspath:$BOOTSTRAP_CLASSPATH \
    -source 1.6 -target 1.6 \
    -d $PLUGIN_DIR/classes $PLUGIN_DIR/$SOURCE_PATH/*.java
else  # Provokes a compiler warning
  echo
  javac -classpath $CLASSPATH \
    -source 1.6 -target 1.6 \
    -d $PLUGIN_DIR/classes $PLUGIN_DIR/$SOURCE_PATH/*.java
  echo "-> For fixing the \"bootstrap class path\" warning, set -Xbootclasspath in script"
fi
echo "Done"

if [ "$2" = "compileonly" ]; then exit 0; fi


# Installation
# ------------
echo "Installing plugin at $GOAL_DIR/plugins"
# Remove possible slash (/) at end of PLUGIN_DIR. This is necessary for cp -r
# to copy the folder and not the content.
PLUGIN_DIR=$(dirname $PLUGIN_DIR)/$(basename $PLUGIN_DIR)
cp -r $PLUGIN_DIR $GOAL_DIR/plugins
