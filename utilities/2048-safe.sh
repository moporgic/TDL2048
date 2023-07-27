#!/bin/bash
# 2048-safe.sh: Check for training failures from the log file

# This tool extract score from the latest log (avg=*) and check whether it is below the threshold
# The score is extracted from the last summary block (if it presents); or from the last normal statistic block

score=$({ tail -n20 "${1:--}" | grep ops -A 1 || echo =-1; } | tail -n1 | cut -d'=' -f2 | cut -d' ' -f1)
(( score >= ${2:-100000} && score <= ${3:-2000000} ))
