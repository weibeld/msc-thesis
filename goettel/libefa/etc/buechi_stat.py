#!/usr/bin/env python

# This program parses the verbose time output of efatool collecting data about 
# the compilementation of automata.
# Author: Christian Goettel

import sys
import glob
import re

def usage():
  print "Usage: buechi_stat.py <dir>\n"
  print " <dir>   Directory having text files containing output of efatool"

argc = len(sys.argv)
if argc == 1 or argc > 2:
  usage()
  sys.exit(1)
count = 0
states = 0
sets = 0
tuples = 0
ns_time = 0
for f in glob.iglob(sys.argv[1] + "/*.txt"):
  for line in open(f, "r"):
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
print "Automata:", count
print "Avg states:", round(states / float(count), 2)
print "Avg sets:", round(sets / float(count), 2)
print "Avg tuples:", round(tuples / float(count), 2)
print "Avg time:", ns_time / float(count) / 1000000000, "s"
sys.exit(0)
