# Compares the performance of fribourg and piterman with and without the
# -macc option (maximising acceptance set of input automaton).
#
# Run from within a folder containing GOAL automata (.gff files):
# $ goal batch macc.gs
#
# dw-06.10.2014

for $in in `ls *.gff` do
  #-----------------------------------------------------------------------------
  # Automaton
  #-----------------------------------------------------------------------------
  echo -n "Automaton: " + $in + ":    ";
  ($s,$t,$a) = stat $in;
  # $s: states, $t: transitions, $a: accepting states
  echo $s + " " + $t + " " + $a;
  echo "---------------------------------------------------------------";

  #-----------------------------------------------------------------------------
  # fribourg
  #-----------------------------------------------------------------------------
  echo -n "Complementation with fribourg:       ";
  $out = complement -m fribourg $in;
  ($s1,$t1,$a1) = stat $out;
  echo $s1 + " " + $t1 + " " + $a1;
  echo -n "Complementation with fribourg -macc: ";
  $out = complement -m fribourg -macc $in;
  ($s2,$t2,$a2) = stat $out;
  echo -n $s2 + " " + $t2 + " " + $a2;
  # Difference of states, transitions, and accepting states with and without -macc
  $s_diff = `echo $(($s2-$s1))`;
  $s_diff = `if [ $s_diff -gt 0 ]; then echo +$s_diff; else echo $s_diff; fi`;
  $t_diff = `echo $(($t2-$t1))`;
  $t_diff = `if [ $t_diff -gt 0 ]; then echo +$t_diff; else echo $t_diff; fi`;
  $a_diff = `echo $(($a2-$a1))`;
  $a_diff = `if [ $a_diff -gt 0 ]; then echo +$a_diff; else echo $a_diff; fi`;
  echo "  " + $s_diff + " " + $t_diff + " " + $a_diff;

  #-----------------------------------------------------------------------------
  # piterman
  #-----------------------------------------------------------------------------
  echo -n "Complementation with piterman:       ";
  $out = complement -m piterman $in;
  ($s1,$t1,$a1) = stat $out;
  echo $s1 + " " + $t1 + " " + $a1;
  echo -n "Complementation with piterman -macc: ";
  $out = complement -m piterman -macc $in;
  ($s2,$t2,$a2) = stat $out;
  echo -n $s2 + " " + $t2 + " " + $a2;
  # Difference of states, transitions, and accepting states with and without -macc
  $s_diff = `echo $(($s2-$s1))`;
  $s_diff = `if [ $s_diff -gt 0 ]; then echo +$s_diff; else echo $s_diff; fi`;
  $t_diff = `echo $(($t2-$t1))`;
  $t_diff = `if [ $t_diff -gt 0 ]; then echo +$t_diff; else echo $t_diff; fi`;
  $a_diff = `echo $(($a2-$a1))`;
  $a_diff = `if [ $a_diff -gt 0 ]; then echo +$a_diff; else echo $a_diff; fi`;
  echo "  " + $s_diff + " " + $t_diff + " " + $a_diff;

  echo;
done