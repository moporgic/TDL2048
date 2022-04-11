#!/bin/bash
# Program: Check for network corruption from a log file
# History: 2022/04/11 Hung Guei

# This tool extract score from the latest log (avg=*) and check whether it is below the threshold
# The score is extracted from the last summary block (if it presents); or from the last normal statistic block

{ tail -n20 | grep ops -A 1 || echo =0; } | tail -n1 | cut -d'=' -f2 | cut -d' ' -f1 | xargs -I{} test {} -ge ${1:-10000}
