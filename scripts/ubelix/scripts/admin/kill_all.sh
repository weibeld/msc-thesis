#!/bin/bash
# Kills all running jobs.

a=($(qstat | tail -n +3 | awk '{print $1}'))

if [ ${#a[@]} -eq 0 ]; then echo "No jobs running."; exit 0; fi

qdel ${a[@]}
