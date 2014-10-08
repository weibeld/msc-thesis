# Performs the complementation-equivalence test on the automta in the folder
# from within the script is run.
#
# Run from within a folder containing GOAL automata (.gff files):
#
# $ goal batch compl_equiv_2.gs
#
# dw-08.10.2014

for $in in `ls *.gff` do

  echo "Automaton: " + $in;

  echo -n "Complementation with fribourg -m -m2... ";
  $frib = complement -m fribourg -m -m2 $in;
  echo "done";

  echo -n "Complementation with piterman -eq -macc -sim -ro -r... ";
  $pit = complement -m piterman -eq -macc -sim -ro -r $in;
  echo "done";

  echo -n "Checking equivalence... ";
  $equal = equivalence $frib $pit;
  echo $equal;
  if !$equal then
    echo $in;
  fi

  echo;
done