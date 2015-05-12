#!./goal batch

if $1 == "" || $2 == "" then
  echo "Usage: complement_compare.gs ALGORITHM1 ALGORITHM2 [ 
         STATE_SIZE PROP_SIZE TRAN_DENSITY ACC_DENSITY ]";
  exit;
fi

echo "Compare two complementation algorithms.";
echo;

$alg1 = $1;
$alg2 = $2;
$state_size = $3;
$prop_size = $4;
$dt = $5;
$da = $6;
if $state_size == null || $prop_size == null || $dt == null || $da == null then
  $state_size = 6;
  $prop_size = 2;
  $dt = 1.0;
  $da = 0.5;
fi

$st1 = 0;
$tr1 = 0;
$st2 = 0;
$tr2 = 0;
$st_diff_accum = 0;
$tr_diff_accum = 0;
$count = 0;
while true do
  echo "#" + $count + ": ";
  echo -n "  Generating an automaton: ";
  $o = generate -t FSA -a NBW -m density -s $state_size -n $prop_size -dt $dt -da $da -r;
  ($s, $t) = stat $o;
  echo $s + ", " + $t;
  
  echo -n "  Complementing by " + $alg1 + ": ";
  $o1 = complement -m $alg1 $o;
  ($s1, $t1) = stat $o1;
  $st1 = $st1 + $s1;
  $tr1 = $tr1 + $t1;
  echo $s1 + ", " + $t1 + " (" + $st1 + ", " + $tr1 + ")";
  
  echo -n "  Complementing by " + $alg2 + ": ";
  $o2 = complement -m $alg2 $o;
  ($s2, $t2) = stat $o2;
  $st2 = $st2 + $s2;
  $tr2 = $tr2 + $t2;
  echo $s2 + ", " + $t2 + " (" + $st2 + ", " + $tr2 + ")";

  $st_diff = $s1 - $s2;
  $tr_diff = $t1 - $t2;
  $st_diff_accum = $st_diff_accum + $st_diff;
  $tr_diff_accum = $tr_diff_accum + $tr_diff;
  echo "  Diff: " + $st_diff + ", " + $tr_diff + 
       " (" + $st_diff_accum + ", " + $tr_diff_accum + ")";

  $count = $count + 1;
done
