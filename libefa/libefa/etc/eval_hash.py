#!/usr/bin/env python

# This program parses the verbose time output of efatool collecting data about 
# the compilementation of automata.
# Author: Christian Goettel

import os
import sys
import re
from subprocess import call

def usage():
  print "Usage: eval_hash.py <dir> <file>\n"
  print " <dir>   Directory holding files of efatool output"
  print " <file>  File name (without suffix) with output of efatool to average"

argc = len(sys.argv)
if argc < 3 or argc > 3:
  usage()
  sys.exit(1)
count = 0
states = 0
sets = 0
tuples = 0
ns_time = 0
for i in range(1,5):
  file_name = sys.argv[2] + str(i) + ".txt"
  for line in open(sys.argv[1] + "/" + file_name, "r"):
	 m = re.search("DOM parsing", line)
	 if m:
		count += 1
	 m = re.search("of states: (\d+)", line)
	 if m:
		states += int(m.group(1))
	 m = re.search("mixed sets: (\d+)", line)
	 if m:
		sets += int(m.group(1))
	 m = re.search("of tuples: (\d+)", line)
	 if m:
		tuples += int(m.group(1))
	 m = re.search("automaton '" + sys.argv[2] + "': (\d+) s (\d+) ns", line)
	 if m:
		ns_time += int(m.group(1)) * 1000000000
		ns_time += int(m.group(2))
print "Automata:" + str(count)
print "States:  " + str(round(states / float(count), 0))
print "Sets:    " + str(round(sets / float(count), 2))
print "Tuples:  " + str(round(tuples / float(count), 2))
print "Time:    " + str(ns_time / float(count) / 1000000000) + " s"
sys.exit(0)
