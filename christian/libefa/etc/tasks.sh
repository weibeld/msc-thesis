#!/bin/bash

TEST_SET="example1.xml example2.xml example3.xml example4.xml example5.xml \
          michel1.xml michel2.xml michel3.xml symbol2.xml symbol3.xml \
          symbol4.xml symbol5.xml"
HASHES="kazlib bernstein crc general-crc murmur1 murmur1a murmur2 murmur2a \
        murmurn2 murmura2 murmur3 one-at-a-time paul-hsieh rotatingv2"
REPETITIONS=5

rm -fR *.txt *.dot *.ps "$HASHES"

for hashf in $HASHES; do
  mkdir -p "$hashf"
  for automaton in $TEST_SET; do
	 efatool -i "$automaton" -o out.dot
	 automaton_name="${automaton%.*}"
	 dot -Tps -o "${automaton_name}i.ps" "${automaton_name}.dot"
	 rm "${automaton_name}.dot"
	 for iteration in `seq 1 $REPETITIONS`; do
		efatool -i "$automaton" -a complementation.unifr -H "$hashf" -o out.dot -v time &> "$hashf/$automaton_name${iteration}.txt"
		if [ $iteration -ne 5 ]; then
		  rm "${automaton_name}.dot"
		fi
	 done
  done
done
