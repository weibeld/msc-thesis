# GOAL script for testing correctness of complementation algorithm by the
# complementation-equivalence test. For changing the complementation algorithm
# and its options, this script file must be edited. The reason is that unfortu-
# nately, complemenation algorithm options cannot be passed in as variables.
#
# Usage: goal batch compl_equiv.gs <nb_tests> <states_min> <states_max> <sym_min> <sym_max>
#
# # dw-11.09.2014

$n = $1;     # Number of tests to do
$s_min = $2; # Minimum number of states of random automaton
$s_max = $3; # Maximum  "             "             "
$a_min = $4; # Minimum alphabet size of random automaton
$a_max = $5; # Maximum  "             "             "

$count = 1;
# Do this $n times
while "true" do

  echo $count + ".";
  echo `date`;
  $time_before = `date +%s`;

  # 1. Generate random automaton
  # ----------------------------
  echo "Random automaton:";
  $states =    `range $s_min $s_max`;
  $alph_size = `range $a_min $a_max`;
  $automaton = generate -t fsa -a nbw -m probability -A classical -n $alph_size -s $states;
  $transitions = stat -t $automaton;
  $acc_states = stat -a $automaton;
  echo "  States  ["+$s_min+"-"+$s_max+"]:    " + $states;
  echo "  Symbols ["+$a_min+"-"+$a_max+"]:    " + $alph_size; 
  echo "  Transitions:      " + $transitions;
  echo "  Accepting states: " + $acc_states;

  # 2. Complement random automaton by base algorithm
  # ------------------------------------------------
  echo -n "Complementing with 'piterman -eq -macc -sim -ro -r'... ";
  $result_base = complement -m piterman -eq -macc -sim -ro -r $automaton;
  echo "done";

  # 3. Complement random automaton by test algorithm
  # ------------------------------------------------
  echo -n "Complementing with 'fribourg'... ";
  $result_test = complement -m fribourg $automaton;
  echo "done";

  # 4. Check equivalence of results
  # -------------------------------
  echo -n "Checking equivalence... ";
  $equal = equivalence $result_base $result_test;
  echo $equal;
  if !$equal then
    echo $automaton;
  fi

  # Time measurement
  # ----------------
  $time_after = `date +%s`;
  echo "Elapsed time: " + `echo $(($time_after - $time_before))` + " sec.";
  echo;
  
  if $count == $n then break; fi
  $count = $count + 1;

done
