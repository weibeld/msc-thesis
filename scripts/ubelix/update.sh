#!/bin/bash
# Install the GOAL plugin from the local machine on UBELIX.
# dw-07.10.2014

set -e

# Plugin dirs on local machin and on UBELIX
plug_local=$(dirname $(which goal))/plugins
plug_remote=bin/GOAL-20130711/plugins # Relative to $HOME

# Run any command on UBELIX
run_remote() {
  ssh dw07r324@submit.unibe.ch "$1"
}

# Pack local plugin and copy to UBELIX
echo "Copying new plugin to UBELIX..."
packed=$plug_local/plugin.tar.gz
tar -cz -f $packed -C $plug_local ch.unifr.goal.complement
put.sh $packed $plug_remote >/dev/null
rm $packed

# Delete old plugin on UBELIX
echo "Deleting old plugin on UBELIX..."
run_remote "rm -rf $plug_remote/ch.unifr.goal.complement"

# Unpack new plugin
echo "Installing new plugin on UBELIX..."
run_remote "tar -xz -f $plug_remote/plugin.tar.gz -C $plug_remote; rm $plug_remote/plugin.tar.gz"

# Create new archive of the whole GOAL application
echo "Creating updated GOAL archive on UBELIX..."
run_remote "tar -cz -f bin/GOAL-20130711.tar.gz -C bin GOAL-20130711"
