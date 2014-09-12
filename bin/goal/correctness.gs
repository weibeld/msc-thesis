# GOAL script for testing correctness of complementation algorithm.
# dw-11.09.2014
# Usage: goal batch correctness.gs fribourg piterman 100

# if $# != 3 then
#   echo "Usage:";
#   echo;
#   echo "    goal batch " + $0 + " <test algo> <base algo> <repetitions>";
#   echo;
#   echo "Generates a random automaton, complements it with the base algorithm (ground-";
#   echo "truth) and the test algorithm, and test the equivalence of the two complements.";
#   echo "Repeats this <repetitions> times.";
#   echo;
#   echo ">> IMPORTANT: algorithm options must be set INSIDE the script.";
#   exit;
# fi

$test_algo = "fribourg";
$base_algo = "piterman";
$n = $1;

$alphabets[0] = "classical";
$alphabets[1] = "propositional";

$count = 1;
while "true" do

  echo $count + ".";
  echo `date`;
  $time_before = `date +%s`;

  # 1. Generate random automaton
  # ----------------------------
  echo "Generating random automaton";
  $states =               `ruby -e 'print "%d" % (rand(9)+2)'`; # [2..10]
  $alph_type = $alphabets[`ruby -e 'print "%d" % rand(2)'`];    # [0..1]
  if $alph_type == "propositional" then
    $alph_size =          `ruby -e 'print "%d" % (rand(3)+1)'`; # [1..3]
  else
    $alph_size =          `ruby -e 'print "%d" % (rand(5)+1)'`; # [1..5]
  fi
  $automaton = generate -t fsa -a nbw -m probability -A $alph_type -n $alph_size -s $states -r;
  echo "  Alphabet:      " + $alph_type;
  echo "  Alphabet size: " + $alph_size;
  echo "  States:        " + $states;
  $trans = stat -t $automaton;
  echo "  Transitions:   " + $trans;
  $acc = stat -a $automaton;
  echo "  Acc states:    " + $acc;

  # 2. Complement random automaton by base algorithm
  # ------------------------------------------------
  echo -n "Complementing with " + $base_algo + "... ";
  $result_base = complement -m $base_algo -eq -macc -sim -sp -ro -r $automaton;
  echo "done";

  # 3. Complement random automaton by test algorithm
  # ------------------------------------------------
  echo -n "Complementing with " + $test_algo + "... ";
  $result_test = complement -m $test_algo -c -r2ifc -m $automaton;
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
  $time_diff = `echo "$time_after - $time_before" | bc`;
  # if $time_diff >= 60 then
  #   $min = `echo "$time_diff / 60" | bc`;
  #   $sec = `echo "$time_diff % 60" | bc`;
  #   $time_string = $min + " min. " + $sec + " sec.";
  # else
  #   $time_string = $time_diff + " sec.";
  # fi
  echo "Elapsed time: " + $time_diff + " sec.";
  echo;
  
  if $count == $n then break; fi
  $count = $count + 1;

done


