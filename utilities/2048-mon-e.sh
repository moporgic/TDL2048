#!/bin/bash
# Program: Fetch the results of testing
# History: 2022/04/11 Hung Guei

# This tool fetches the results from summary block of log files and display them
# The typical scenarios of using this tool are, when performing testing with different networks or different depths

result() {
	grep summary -A20 | grep -E "^(total|${win:-32768})" | sed -E "s/${win:-32768}.+ / /g" | \
		sed -E "s/.+avg=/+/g" | sed -E "s/ max=.+//g" | xargs | tr '+' ';' | sed "s/ ;/;/g" | cut -b2- 2>/dev/null
}

if [[ $1 ]]; then # fetch from file(s)
	[[ $1 == *.x ]] && name="$1" || name="$1*.x"
	for x in $(find $(dirname ${1:-.}) -name "$name" | sort); do
		label="$(basename $x): "
		[[ $1 ]] && label=$(<<<${label#$1} sed -E "s/^[:. -]+//g")
		len=$((${#label} > len ? ${#label} : len))
		L+=("$label")
		R+=("$(result < $x)")
	done
else # fetch from stdin
	L=("")
	R=("$(result)")
	filter=${filter:-cat}
fi
for (( i=0; i<${#L[@]}; i++ )); do
	label=${L[$i]}
	blank=$(<<<$label tr '[:graph:]' ' ')
	<<<${R[$i]} tr ';' '\n' | ${filter:-tail -n1} | while IFS= read res; do
		printf "%-${len}s%6s%7s\n" "$label" $res
		label=$blank
	done
done
