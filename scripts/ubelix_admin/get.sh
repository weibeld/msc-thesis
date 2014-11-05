#!/bin/bash

FILE_OR_FOLDER=$1

ORIGIN="dw07r324@submit.unibe.ch:$FILE_OR_FOLDER"
DEST=~/Desktop

scp -r $ORIGIN $DEST
