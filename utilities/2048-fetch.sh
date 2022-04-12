#!/bin/bash
# Program: Fetch the results from summary blocks
# History: 2022/04/11 Hung Guei

# This tool fetches the results from summary blocks of log files and display them.
# The typical scenarios of using this tool are, 
# (1) monitor the training progress of a long-term training with periodic evaluations.
# (2) monitor different testing results for different settings such as networks or depths.

# Usage:
# (1) pipe raw logs as stdin: (raw log) | ./2048-fetch.sh [OPTS]
# (2) specify log name/label: ./2048-fetch.sh [OPTS] NAME|LABEL

while [[ $1 == -* ]]; do
	case "$1" in
		-o)    shift; ;& # select output columns: e.g., "avg,max,8192,16384,32768"
		-o=*)  columns=${1#*=}; ;;
		-a|-A) filter="cat"; ;; # always print all summary blocks
		-l|-1) filter="tail -n1"; ;; # always print only the last summary blocks
		-i)    index=1; ;; # always show index numbers
	esac
	shift
done

# parse output columns
columns=,${columns,,},
declare -A repl=([a]=avg [av]=avg [m]=max [x]=max [mx]=max \
	[1k]=1024 [2k]=2048 [4k]=4096 [8k]=8192 [16k]=16384 [32k]=32768 [64k]=65536 \
	[1]=1024 [2]=2048 [4]=4096 [8]=8192 [16]=16384 [32]=32768 [64]=65536)
for n in ${!repl[@]}; do
	columns=${columns/,$n,/,${repl[$n]},}
done
columns=${columns:1}; columns=${columns::-1}
columns=${columns:-"avg,max,16384,32768"}

# generate formats
format=
[[ $columns == *avg* ]] && { avg=".+avg="; format+="%6s"; } || avg=".+avg=[0-9]+ "
[[ $columns == *max* ]] && { max="max="; format+="%8s"; } || max="max=[0-9]+"
win=$(<<<$columns sed -E "s/[a-z]+,//g" | tr ',' '|')
format+=$(<<<$win sed -E "s/[0-9]\||[0-9]$/_/g" | tr -d '[:digit:]' | sed "s/_/%8s/g")

# fetch labels
src=()
if [[ $1 ]]; then # fetch from file(s)
	[[ $1 == *.x ]] && name="$1" || name="$1*.x"
	for x in $(find $(dirname ${1:-.}) -name "$name" | sort); do
		label="$(basename $x):  "
		[[ $1 ]] && label=$(<<<${label#$1} sed -E "s/^[:. -]+//g")
		len=$((${#label} > len ? ${#label} : len))
		src+=("${label}|${x}")
	done
else # fetch from stdin
	src=("|")
fi

# fetch results and print
(( ${#src[@]} == 1 )) && filter=${filter:-cat} || filter=${filter:-tail -n1}
for (( i=0; i<${#src[@]}; i++ )); do
	token=${src[$i]}
	label=${token%%|*}
	blank=$(<<<$label tr '[:graph:]' ' ')
	result=$(grep summary -A20 ${token##*|} | \
		grep -E "^(total${win:+|}$win)" | sed -E "s/^[0-9].+% +/ /g" | \
        sed -E "s/$avg/+/g" | sed -E "s/($max)|( tile=.+)//g" | \
		xargs | sed -e "s/^\+//g" | tr '+' '\n' | sed -e "s/^ //g")
	width=$(($(<<<$result wc -l) - 1))
	fmt=$format
	[ "${index}${filter}" == cat ] && idx=$width || idx=$index
	if (( ${idx:-0} )); then
		result="$(<<<$result nl -v0 -w${#width} -s': ' -ba -nrz)"
		fmt="%-5s$format"
	fi
	fmt=%-${len}s${fmt}
	<<<$result $filter | while IFS= read res; do
		printf "${fmt}\n" "$label" $res
		label=$blank
	done
done
