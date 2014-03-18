#!/bin/bash

dest="test_set"
stylesheet="gff2faxml.xsl"

mkdir -p "$dest"

for file in $(ls "$1"/*.gff); do
    filename=$(basename "$file")
    filename="${filename%.*}"
    xsltproc --stringparam filename "$filename" -o "$dest"/"$filename".xml "$stylesheet" "$file"
done
