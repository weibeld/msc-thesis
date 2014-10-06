#!/bin/bash

aut=aut.gff
aut_red=aut_red.gff

echo "Renaming $1 to $aut"
mv $1 $aut

echo "Generating reduced version of $aut"
goal reduce -o $aut_red $aut &>/dev/null

echo "$aut:"
goal stat $aut
echo

echo "$aut_red:"
goal stat $aut_red 
echo

echo "Complementing $aut and $aut_red with 'fribourg -r2ifc -m -r'"
frib=frib.gff
frib_red=frib_red.gff
goal complement -m fribourg -r2ifc -m -r -o $frib $aut &>/dev/null
goal complement -m fribourg -r2ifc -m -r -o $frib_red $aut_red &>/dev/null

echo "$frib:"
goal stat $frib
echo

echo "$frib_red:"
goal stat $frib_red
echo

echo "Complementing $aut and $aut_red with 'piterman -eq -sim -ro'"
pit=pit.gff
pit_red=pit_red.gff
goal complement -m piterman -eq -sim -ro -r -o $pit $aut &>/dev/null
goal complement -m piterman -eq -sim -ro -r -o $pit_red $aut_red &>/dev/null

echo "$pit:"
goal stat $pit
echo

echo "$pit_red:"
goal stat $pit_red
echo
