#!/bin/bash
# Sun Grid Engine writes this seven character long sequence 033 [ H 033 [ 2 J
# to the end of the stdout files if the job script contains an exit command.

sed -i 's/\o033\[H\o033\[2J//' $1
echo "Fixed $1"

