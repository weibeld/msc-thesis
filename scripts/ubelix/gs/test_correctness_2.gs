# Performs the complementation-equivalence test on all the automata in the
# specified folder.
#
# $ goal batch compl_equiv_2.gs <folder>
#
# EDIT May 2015: no rewrite/cleaning done on this file yet
#
# dw-08.10.2014
#------------------------------------------------------------------------------#

$folder = $1;

for $in in `ls $folder/*.gff` do

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
    echo "Incorrect: " + $in;
  fi

  echo;
done