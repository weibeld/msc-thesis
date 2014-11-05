#!/bin/bash
# Replace ~/scripts on UBELIX with ./scripts on local machine. For security,
# keep backup of ~/scripts on UBELIX in ~/.scripts. The script must be executed
# from a directory that contains a folder named "scripts".
#
# dw-04.11.2014

echo "Copying"
echo "    $(pwd)/scripts"
echo "to"
echo "    ~/scripts"
echo "on UBELIX..."

run.sh "rm -rf .scripts; mv scripts .scripts"
put.sh scripts >/dev/null

echo Done

