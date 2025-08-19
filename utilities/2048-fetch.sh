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
		-i)    index=2; ;; # always show index numbers
		-i*)   index=$(<<<${1:2} tr -d '='); ;; # set index minimal display digits (0 to disable)
		-H)    index=${index:-2}; headings=yes; ;; # print headings
		-t)    output="sed -E s/\\s+/\\t/g"; ;; # use tabs instead of spaces
	esac
	shift
done

# parse output columns
columns=,${columns,,},
declare -A repl=([a]=avg [av]=avg [m]=max [x]=max [mx]=max
	[1k]=1024 [2k]=2048 [4k]=4096 [8k]=8192 [16k]=16384 [32k]=32768 [64k]=65536
	[1]=1024 [2]=2048 [4]=4096 [8]=8192 [16]=16384 [32]=32768 [64]=65536
	[full]=avg,max,2048,4096,8192,16384,32768,65536)
for n in ${!repl[@]}; do
	columns=${columns/,$n,/,${repl[$n]},}
done
columns=${columns:1}; columns=${columns::-1}
columns=${columns:-"avg,max,16384,32768"}
chk=${columns},; chk=${chk#avg,}; chk=${chk#max,}
[[ $chk != *avg* && $chk != *max* ]] || exit $?
[[ $chk == $(sort -n <<< ${chk//,/$'\n'} | xargs | tr ' ' ,), || ! $chk ]] || exit $?

# generate formats
format=
[[ $columns == *avg* ]] && { avg=".+avg="; format+="%6s"; } || avg=".+avg=[0-9]+ "
[[ $columns == *max* ]] && { max="max="; format+="%8s"; } || max="max=[0-9]+"
win=$(<<<$columns sed -E "s/[a-z]+,?//g" | tr ',' '|')
format+=$(<<<$win sed -E "s/[0-9]\||[0-9]$/_/g" | tr -d '[:digit:]' | sed "s/_/%8s/g")
columns=($(<<<$columns sed -E "s/[^a-z0-9]/ /g"))
output=${output:-cat}

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
if [[ $headings ]]; then
	if [[ $output != cat &&
		! $(printf "%s\n" "${src[@]}" | sed -E "s/\|.+$//" | xargs) && $index == 0 ]]; then
		output="sed -E s/^\\s+//;s/\\s+/\\t/g" # also trim beginning spaces
	fi
	printf "%-$((len+(index?index+3:0)))s${format}\n" "" ${columns[@]^^} | $output
fi
(( ${#src[@]} == 1 )) && filter=${filter:-cat} || filter=${filter:-tail -n1}
for (( i=0; i<${#src[@]}; i++ )); do
	label=${src[$i]%|*}
	res=$(grep summary -A20 "${src[$i]#*|}" |
		while IFS= read -r stat; do # fix missing win rates
			if [[ $prev ]]; then
				if [[ $stat =~ ^([0-9]+)\ .+\ [0-9.]+%$ ]]; then
					for (( tile=${prev%% *}<<1; tile<${BASH_REMATCH[1]}; tile<<=1 )); do
						echo $tile 0 0 0 0% ${prev##* }
					done
					prev=$stat
				elif [[ ! $stat ]]; then
					for (( tile=${prev%% *}<<1; tile<=65536; tile<<=1 )); do
						echo $tile 0 0 0 0% 0%
					done
					prev=$stat
				fi
				echo $stat
			elif [[ $stat == summary* ]]; then
				prev="1 0 0 0 0% 100.00%"
			fi
		done |
		grep -E "^(total${win:+|}$win)" | sed -E "s/ [0-9. ]+% +/|/g" |
		sed -E "s/$avg/+/g" | sed -E "s/($max)|( tile=.+)//g" |
		xargs | sed -e "s/^\+//g" | tr '+' '\n' | sed -e "s/^ //g")
	width=$(($(<<<$res wc -l) - 1))
	fmt=$format
	[[ ${index}${filter} == cat ]] && idx=$width || idx=$index
	if (( ${idx:-0} )); then # prefix res with index
		width=$((${#width} > ${index:-2} ? ${#width} : ${index:-2}))
		res=$(<<<$res nl -v0 -w$width -s': ' -nrz)
		fmt=%-$((width + 3))s$format
	fi
	fmt=%-${len}s${fmt}
	<<<$res $filter | while IFS= read -r res; do
		[[ $res ]] || continue
		res=$(<<<$res sed -E "s/[0-9]+\|//g")
		printf "${fmt}\n" "${label}" ${res}
		label=
	done
done | stdbuf -o0 $output
