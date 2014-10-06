#!./goal batch

# Apparently used by Ming-Hsien and his team for generating the test set of
# 11'000 random automata. Note that the generated automata may contain unreach-
# able and dead states because the -r option of generate is not set.

$size = $1;
$props = $2;
$num = $3;  # 100
$basedir = $4;

if $size == null || $props == null || $num == null || $basedir == null then
  echo "Usage: gen.gs STATE_SIZE ALPHABET_SIZE NUM_AUTOMATA BASEDIR";
  exit;
fi

$dts = "1.0 1.2 1.4 1.6 1.8 2.0 2.2 2.4 2.6 2.8 3.0"; # 11
$das = "0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0";     # 10

for $dt in $dts do # 11
  for $da in $das do # 10
    for $i in `seq 1 $num` do # 100
      $file = $basedir +  "/aut-" + $size + "-" + $props + "-" + $dt + "-" + $da + "-" + $i + ".gff";
      echo "Generating " + $file;
      $o = generate -t fsa -a nbw -A classical -m density -s $size -n $props -dt $dt -da $da;
      save $o $file;
    done
  done
done
