#!/bin/bash
# History: 2019-06-15 moporgic
#          2022-07-17 moporgic
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

values=()
for i in {1..16}; do
	values+=($((2**i)))
done
values=${values[@]}

tiles=${@:-2048|4096|8192|16384|32768}
tiles=(${tiles//.../ ... })
tiles=${tiles[@]}
tiles=${tiles//,/|}
tiles=${tiles// /|}

tiles=$({
	egrep -o '[0-9]+' <<< "$tiles"
	for range in $(egrep -o '[0-9]+\|\.\.\.\|[0-9]+'); do
		[[ $range =~ ([0-9]+)[^0-9]+([0-9]+) ]]
		lower=${BASH_REMATCH[1]}
		upper=${BASH_REMATCH[2]}
		for tile in $values; do
			[ $tile -ge $lower ] && [ $tile -le $upper ] && echo $tile
		done
	done <<< "2|$tiles|65536"
} | sort -h | uniq | egrep ${values// /|} | xargs)

echo "${tiles// /[%]$'\t'}[%]"

{ egrep "^[0-9]+ [0-9. ]+% +[0-9.]+%$" | while IFS= read -r res; do
	tile=${res%% *}
	win=${res##* }
	[ "$win" == "100.00%" ] && echo
	echo -n "$tile:$win "
done | tail -n+2; echo; } | while IFS= read -r result; do
	[[ $result ]] || continue
	declare -A stat

	tile=2
	win="100.00%"
	for res in $result; do
		for (( this=${res%:*}; tile<this; tile+=tile )); do
			stat[$tile]=$win
		done
		win=${res#*:}
	done
	stat[$tile]=$win
	for (( tile=tile*2; tile<=65536; tile+=tile )); do
		stat[$tile]="0.00%"
	done

	for tile in $tiles; do
		echo ${stat[$tile]}
	done | xargs | tr ' ' '\t'
done
