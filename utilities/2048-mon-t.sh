#!/bin/bash
# Program: Fetch the progress of training
# History: 2022/04/11 Hung Guei

# This tool fetches the results from summary blocks of a log file and display them
# The typical scenarios of using this tool are, when performing a long-term training with periodic evaluation,
# with all statistics are written into a single log file

res=$(grep summary -A 1 ${1%.x}.x | grep 'avg=' | cut -d':' -f2 | xargs -L1)
idx=$(($(<<<$res wc -l) - 1))
<<<$res nl -v0 -w${#idx} -s': ' -nrz
