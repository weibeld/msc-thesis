#!./goal batch
# The #! would allow to just execute $ ./complement_equiv.gs instead of
# $ goal batch complement_equiv.gs.
# The #! with a relative path does not work on Mac. It work on Linux, but even
# there it's of limited use, because it requires that the GOAL script is
# executed in the same directory as the goal script is located (or somehow
# relative from there).
# So, it's maybe best to not use the #! at all, or to use an absolute path
# of the goal script (putting a symlink to the goal script in e.g. /usr/bin
# doesn't work).
# dw-18.08.2014

# if $1 == "" || $2 == "" then
#   echo "Usage: complement_equiv.gs BASE_ALGORITHM TEST_ALGORITHM [ 
#          STATE_SIZE PROP_SIZE TRAN_DENSITY ACC_DENSITY ]";
#   exit;
# fi

echo "Check the correctness of a complementation algorithm.";
echo;

$base_alg = "piterman";
$test_alg = "fribourg";
$n = $1;
$state_size = $3;
$prop_size = $4;
$dt = $5;
$da = $6;
if $state_size == null || $prop_size == null || $dt == null || $da == null then
  $state_size = 4;
  $prop_size = 2;
  $dt = 1.0;
  $da = 0.5;
fi

$count = 1;
$eq = 1;
while $eq do
  echo "#" + $count + ": ";
  echo -n "  Generating an automaton: ";
  $o = generate -t FSA -a NBW -m density -s $state_size -n $prop_size -dt $dt -da $da -r;
  ($s, $t) = stat $o;
  echo $s + ", " + $t;
  
  echo -n "  Complementing by " + $base_alg + ": ";
  $o1 = complement -m $base_alg -eq -macc -sim -sp -ro -r $o;
  ($s, $t) = stat $o1;
  echo $s + ", " + $t;
  
  echo -n "  Complementing by " + $test_alg + ": ";
  $o2 = complement -m $test_alg -c -r2ifc -m $o;
  ($s, $t) = stat $o2;
  echo $s + ", " + $t;
  
  echo -n "  Checking equivalence: ";
  $equiv = equivalence $o1 $o2;
  echo $equiv;

  if !$equiv then
    $eq = 0;
    echo "Counterexample found!";
    echo "";
    echo $o;
  fi

  if $count == $n then break; fi
  $count = $count + 1;
done
