#!/bin/bash
#
# Usage: put.sh FILE_OR_FOLDER [DESTINATION_FILE_OR_FOLDER]

FILE_OR_FOLDER=$1

DEST='dw07r324@submit.unibe.ch:~'"/$2"

scp -r $FILE_OR_FOLDER $DEST
