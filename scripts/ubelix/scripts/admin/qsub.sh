#!/bin/bash

cat $1/qsub | sed "s/$1/$2/g" > $2/qsub

