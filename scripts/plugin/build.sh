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
# Daniel Weibel, 26.05.2014


set -e # Exit if any command returns a non-zero status (i.e. error)

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "USAGE:"
  echo "    $(basename $0) PLUGIN_DIR GOAL_DIR [-c]"
  echo "DESCRIPTION:"
  echo "    Compiles the GOAL plugin in PLUGIN_DIR and installs it under GOAL_DIR."
  echo "    With the -c option, the plugin is only compiled but not installed."
  echo "NOTE:"
  echo "    Compiler warning \"bootstrap class path not set in conjunction with"
  echo "    -source 1.6\" can be removed with \"javac -Xbootclasspath:\" in script."
  exit 0
fi

test_dir() {
  if [ ! -d "$1" ]; then echo "Error: $1 is not a valid directory"; exit 1; fi
}

# Root directory of the plugin to install
PLUGIN_DIR=$(sed 's/\/$//' <<<"$1")
test_dir "$PLUGIN_DIR"

# GOAL installation directory
GOAL_DIR=$(sed 's/\/$//' <<<"$2")
test_dir "$GOAL_DIR"

# Get path to the directory containing the source files, relative to the plugin
# directory. The plugin directory is per convention the package name of the
# plugin code. I.e. the plugin in directory ch.unifr.example has its source
# files in ch.unifr.example/sources/ch/unifr/example.
PLUGIN_NAME=$(basename $PLUGIN_DIR)
IFS='.' read -a ARRAY <<<$PLUGIN_NAME
SOURCE_PATH="sources"
for ELT in ${ARRAY[@]}; do SOURCE_PATH=$SOURCE_PATH/$ELT; done

# The standard GOAL plugins that must be added to the classpath for compilation
CORE=$GOAL_DIR/plugins/org.svvrl.goal.core
CMD=$GOAL_DIR/plugins/org.svvrl.goal.cmd
GUI=$GOAL_DIR/plugins/org.svvrl.goal.gui

# The GOAL plugins are zipped by default, but must be present unzipped to be
# accessible by the Java compiler. Unzip them if they aren't yet.
[ ! -d $CORE ] && unzip $CORE.zip -d $CORE >/dev/null && rm $CORE.zip
[ ! -d $CMD ]  && unzip $CMD.zip -d  $CMD  >/dev/null && rm $CMD.zip
[ ! -d $GUI ]  && unzip $GUI.zip -d  $GUI  >/dev/null && rm $GUI.zip

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

if [ "$3" == -c ]; then exit 0; fi

# Installation
# ------------
echo "Installing plugin at $GOAL_DIR/plugins"
cp -r $PLUGIN_DIR $GOAL_DIR/plugins
