#!/bin/bash
# 2048-fetch.sh: Fetch the results from summary blocks

# This tool fetches the results from summary blocks of log files and display them.
# The typical scenarios of using this tool are,
# (1) monitor the training progress of a long-term training with periodic evaluations.
# (2) monitor different testing results for different settings such as networks or depths.

# Usage:
# (1) pipe raw logs as stdin: (raw log) | ./2048-fetch.sh [OPT]...
# (2) specify log name/label: ./2048-fetch.sh [OPT]... [LOG]...

while [[ $1 == -* ]]; do
	case "$1" in
		-o)    shift; ;& # select output columns: e.g., "avg,max,8192,16384,32768"
		-o=*)  columns=${1#*=}; ;;
		-a|-A) filter="cat"; ;; # always print all summary blocks
		-1|-L) filter="tail -n1"; ;; # always print only the last summary blocks
		-i)    index=1; ;; # always show index numbers
		-i*)   index=$(<<<${1:2} tr -d '='); ;; # set index minimal display digits (0 to disable)
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
win=$(<<<$columns sed -E "s/[a-z]+,?//g" | tr ',' '|')
format+=$(<<<$win sed -E "s/[0-9]\||[0-9]$/_/g" | tr -d '[:digit:]' | sed "s/_/%8s/g")
columns=($(<<<$columns sed -E "s/[^a-z0-9]/ /g"))

# fetch labels
(( $# )) && src=() || src=("|-") # fetch from stdin if no args
while (( $# )); do # fetch from file labels...
	[[ $1 == *.x ]] && name="${1##*/}" || name="${1##*/}*.x"
	for x in $(find $(dirname ${1:-.}) -maxdepth 1 -name "$name" | sort); do
		label="$(basename $x):  "
		[[ $1 && $1 != *.x ]] && label=$(<<<${label#${1##*/}} sed -E "s/^[:. -]+//g")
		len=$((${#label} > len ? ${#label} : len))
		src+=("${label}|${x}")
	done
	shift
done

# fetch results and print
(( ${#src[@]} == 1 )) && filter=${filter:-cat} || filter=${filter:-tail -n1}
for (( i=0; i<${#src[@]}; i++ )); do
	label=${src[$i]%|*}
	result=$(grep summary -A20 "${src[$i]#*|}" | \
		grep -E "^(total${win:+|}$win)" | sed -E "s/ [0-9. ]+% +/|/g" | \
		sed -E "s/$avg/+/g" | sed -E "s/($max)|( tile=.+)//g" | \
		xargs | sed -e "s/^\+//g" | tr '+' '\n' | sed -e "s/^ //g")
	width=$(($(<<<$result wc -l) - 1))
	fmt=$format
	[ "${index}${filter}" == cat ] && idx=$width || idx=$index
	if (( ${idx:-0} )); then # print results with index
		width=$((${#width} > index ? ${#width} : index))
		result="$(<<<$result nl -v0 -w$width -s': ' -nrz)"
		fmt="%-$((width + 3))s$format"
	fi
	fmt=%-${len}s${fmt}
	<<<$result $filter | while IFS= read -r res; do
		res=$(echo $res)
		if [[ $res ]]; then
			chk=($res)
			if (( ${#chk[@]} < ${#columns[@]}+(${idx:-0}?1:0) )); then # fix missing win rates
				bound=$(<<< "$res" sed -E "s/[0-9]+:? |\|[0-9.]+%//g" | sed -E "s/ [0-9 ]+ / /")
				for tile in ${win//|/ }; do
					(( $tile < ${bound% *} )) && res=${res/100.00% /100.00% 100.00% }
					(( $tile > ${bound#* } )) && res=${res/%/ 0%}
				done
			fi
			res=$(<<<$res sed -E "s/[0-9]+\|//g")
			printf "${fmt}\n" "${label}" ${res}
		fi
		label=
	done
done
