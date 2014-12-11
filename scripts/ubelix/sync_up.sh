#!/bin/bash
# Push directory ./scripts fo to ~/scripts on UBELIX. For safety, keep backup
# of ~/scripts on UBELIX in ~/.scripts. This script must be executed from the
# directory that contains the scripts folder to sync up.
#
# dw-04.11.2014

echo -n "Syncing UBELIX ~/scripts with local ./scripts directory... "

run.sh "rm -rf .scripts; mv scripts .scripts &>/dev/null"
put.sh scripts >/dev/null

echo done

