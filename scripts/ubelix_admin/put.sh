#!/bin/bash
#
# Usage: put.sh FILE_OR_FOLDER [DESTINATION_FILE_OR_FOLDER]

if [[ $# -eq 0 ]] || [[ "$1" == -h ]]; then
  echo "USAGE:"
  echo "    $(basename $0) LOCAL [REMOTE]"
  echo "ARGUMENTS:"
  echo "    LOCAL    File or folder on local machine"
  echo "    REMOTE   File or folder on UBELIX, relative to HOME directory"
  exit
fi 

FILE_OR_FOLDER=$1
DEST="dw07r324@submit.unibe.ch:$2"
scp -r $FILE_OR_FOLDER $DEST
