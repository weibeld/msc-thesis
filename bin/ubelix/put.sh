#!/bin/bash

FILE_OR_FOLDER=$1

DEST='dw07r324@submit.unibe.ch:~'

if [ -f $FILE_OR_FOLDER ] || [ -d $FILE_OR_FOLDER ]; then
  scp -r $FILE_OR_FOLDER $DEST
else
  echo "Error: $FILE_OR_FOLDER is not a valid file or folder."
fi
