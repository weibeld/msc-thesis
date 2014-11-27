#!/bin/bash

timeouts=0

for file in $(ls "$1"/*.xml); do
#for file in $(ls "$2"/*.xml | tail -n "$1"); do
    filename=$(basename "$file")
    filename="${filename%.*}"
    timeout 10m efatool -i "$file" -a complementation.unifr3 -o out.dot -v time &> "$filename".txt
    if [ $? -eq 124 ]; then
      ((timeouts++))
    fi
    rm -f "$filename".dot
done

echo "$timeouts" >> out.txt
