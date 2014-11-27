#!/usr/bin/env python

# This program parses the verbose time output of efatool collecting data about 
# the compilementation of automata.
# Author: Christian Goettel

import os
import sys
import re
from subprocess import call

def usage():
  print "Usage: eval_stat.py <dir> <int> <file>\n"
  print " <dir>   Directory having text files containing output of efatool"
  print " <int>   automaton size"
  print " <file>  File holding data to plot"

argc = len(sys.argv)
if argc < 4 or argc > 4:
  usage()
  sys.exit(1)
count = 0
states = 0
sets = 0
tuples = 0
ns_time = 0
td = ['1.00', '1.20', '1.40', '1.60', '1.80', '2.00', '2.20', '2.40', '2.60', '2.80', '3.00']
ad = ['0.10', '0.20', '0.30', '0.40', '0.50', '0.60', '0.70', '0.80', '0.90', '1.00']
f = open(sys.argv[3], 'w')
for t in range(len(td)):
  for a in range(len(ad)):
	 for i in range(1,100):
		file_name = "new-s-" + sys.argv[2] + "-r-" + td[t] + "-f-" + ad[a] + "--" + str(i) + "-of-100.txt"
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
		  m = re.search(": (\d+) s (\d+) ns", line)
		  if m:
			 ns_time += int(m.group(1)) * 1000000000
			 ns_time += int(m.group(2))
	 f.write(td[t] + "\t" + ad[a] + "\t")
	 s = str(round(sets / float(count), 2))
	 f.write(s + "\t")
	 s = str(round(tuples / float(count), 2))
	 f.write(s + "\t")
	 s = str(ns_time / float(count) / 1000000000)
	 f.write(s + "\n")
	 count = 0
	 states = 0
	 sets = 0
	 tuples = 0
	 ns_time = 0
f.close()
sys.exit(0)
