#!/bin/bash

# Compiles and installs a JPF (Java Plugin Framework) plugin for GOAL. Can be 
# executed from anywhere. Assumes that the name of the plugin directory is equal
# to the Java package name of the plugin.
#
# A typical JPF plugin has the structure:
# ch.unifr.goal.myplugin/   # Plugin direcory
#     plugin.xml            # JPF plugin manifest
#     sources/
#         ch/unifr/goal/myplugin/<.java files>
#     classes/
#         ch/unifr/goal/myplugin/<.class files>
#
# Installing the plugin is simply done by copying the plugin directory
# (ch.unifr.goal.myplugin/ in the above example) to <GOAL>/plugins.
#
# Usage: ./plugin_build.sh <plugin_dir> [compileonly]
#
# Daniel Weibel, 26.05.2014


# Exit if any command returns a non-zero status (i.e. error)
set -e

if [ "$1" = "-h" ] || [ "$1" = "help" ] || [ "$1" = "--help" ] || [ $# -lt 1 ] || [ $# -gt 2 ]; then
  echo "USAGE:"
  echo "    $(basename $0) PLUGIN_DIR [-c]"
  echo "DESCRIPTION:"
  echo "    Compiles and installs the GOAL plugin in PLUGIN_DIR. Installing means"
  echo "    simply copying PLUGIN_DIR to <GOAL>/plugins. With the option -c, the"
  echo "    plugin is only compiled but not installed."
  echo "NOTE:"
  echo "    Compiler warning \"bootstrap class path not set in conjunction with"
  echo "    -source 1.6\" can be removed with \"javac -Xbootclasspath:\" in script."
  exit 0
fi

if [ "$(which goal)" == "" ]; then
  echo "ERROR: make sure 'which goal' points to the 'goal' script by, for example,"
  echo "adding the GOAL folder to the PATH"
  exit 1
fi

PLUGIN_DIR=$(echo $1 | sed 's/\/$//') # Remove possible trailing /

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
WITH_XBOOTCLASSPATH=true
# No compiler warning
if [ "$WITH_XBOOTCLASSPATH" = true ]; then
  # Adapt value of -Xbootclasspath:
  javac -classpath $CLASSPATH \
    -Xbootclasspath:/Library/Java/JavaVirtualMachines/jdk1.7.0_45.jdk/Contents/Home/jre/lib/rt.jar \
    -source 1.6 -target 1.6 \
    -d $PLUGIN_DIR/classes $PLUGIN_DIR/$SOURCE_PATH/*.java
# Provokes a compiler warning
else
  echo
  javac -classpath $CLASSPATH \
    -source 1.6 -target 1.6 \
    -d $PLUGIN_DIR/classes $PLUGIN_DIR/$SOURCE_PATH/*.java
  echo "-> For fixing the \"bootstrap class path\" warning, set -Xbootclasspath in script"
fi
echo "Done"

if [ "$2" == -c ]; then exit 0; fi

# Installation
# ------------
echo "Installing plugin at $GOAL_DIR/plugins"
cp -r $PLUGIN_DIR $GOAL_DIR/plugins
