#!/bin/bash

PLUGIN_NAME=$1

PREFIX_1=ch
PREFIX_2=unifr
PREFIX_3=goal
PACKAGE=$PREFIX_1.$PREFIX_2.$PREFIX_3.$PLUGIN_NAME

mkdir $PACKAGE

cd $PACKAGE

mkdir -p sources/$PREFIX_1/$PREFIX_2/$PREFIX_3/$PLUGIN_NAME

mkdir classes

touch plugin.xml

echo '<?xml version="1.0" ?>' >plugin.xml
echo '<!DOCTYPE plugin PUBLIC "-//JPF//Java Plug-in Manifest 1.0" "http://jpf.sourceforge.net/plugin_1_0.dtd">' >>plugin.xml
echo "<plugin id=\"$PACKAGE\">" >>plugin.xml
echo '<requires>' >>plugin.xml

echo '</requires>' >>plugin.xml
echo '</plugin>' >>plugin.xml

