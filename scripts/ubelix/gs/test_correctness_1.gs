# Complement-equivalence tests on random automata. Complementation construction,
# number of tests to do, and dimensions of random automata are specified by
# command line arguments.
#
# Usage:
#     goal batch compleq.gs <algo>
#                           <options>
#                           <runs>
#                           <states_min>
#                           <states_max>
#                           <sym_min>
#                           <sym_max>
#
# EDIT May 2015: no rewrite/cleaning done on this file yet
#
# dw-11.09.2014
#------------------------------------------------------------------------------#

$algo = $1;  # Complementation construction
$opts = $2;  # Options to the construction (may be an empty string)
$n = $3;     # Number of tests to do
$s_min = $4; # Minimum number of states of random automaton
$s_max = $5; # Maximum  "             "             "
$a_min = $6; # Minimum alphabet size of random automaton
$a_max = $7; # Maximum  "             "             "

$count = 1;
# Do this $n times
while "true" do

  echo $count + ".";
  echo `date`;
  $time_before = `date +%s`;

  # 1. Generate random automaton
  # ----------------------------
  echo "Random automaton:";
  # Random numbers between $x_min (including) and $x_max (including)
  $states = `echo $(($(($RANDOM%$(($s_max-$s_min+1))))+$s_min))`;
  $syms =   `echo $(($(($RANDOM%$(($a_max-$a_min+1))))+$a_min))`;
  $aut = generate -t fsa -a nbw -m probability -A classical -n $syms -s $states;
  $trans = stat -t $aut;
  $acc = stat -a $aut;
  echo "  States  ["+$s_min+"-"+$s_max+"]:    " + $states;
  echo "  Symbols ["+$a_min+"-"+$a_max+"]:    " + $syms; 
  echo "  Transitions:      "                   + $trans;
  echo "  Accepting states: "                   + $acc;

  # 2. Complement random automaton by base algorithm
  # ------------------------------------------------
  echo -n "Complementing with 'piterman -eq -macc -sim -ro -r'... ";
  $result_base = complement -m piterman -eq -macc -sim -ro -r $aut;
  echo "done";

  # 3. Complement random automaton by test algorithm
  # ------------------------------------------------
  echo -n "Complementing with '" + $algo + $opts + "'... ";
  $result_test = complement -m $algo --option $opts $aut;
  echo "done";

  # 4. Check equivalence of results
  # -------------------------------
  echo -n "Checking equivalence... ";
  $equal = equivalence $result_base $result_test;
  echo $equal;
  if !$equal then
    echo $aut;
    $file = $count + ".gff";
    save $aut $file;
  fi

  # Time measurement
  # ----------------
  $time_after = `date +%s`;
  echo "Elapsed time: " + `echo $(($time_after - $time_before))` + " sec.";
  echo;
  
  if $count == $n then break; fi
  $count = $count + 1;

done
