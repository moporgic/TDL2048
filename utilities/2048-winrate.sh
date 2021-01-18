#!/bin/bash
#History: 2019-06-15 moporgic
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

tiles=$((
	egrep -o '[0-9]+' <<< "$tiles"
	for range in $(egrep -o '[0-9]+\|\.\.\.\|[0-9]+'); do
		[[ $range =~ ([0-9]+)[^0-9]+([0-9]+) ]]
		lower=${BASH_REMATCH[1]}
		upper=${BASH_REMATCH[2]}
		for tile in $values; do
			[ $tile -ge $lower ] && [ $tile -le $upper ] && echo $tile
		done
	done <<< "2|$tiles|65536"
) | sort -h | uniq | egrep ${values// /|})

tiles=${tiles//$'\n'/|}
echo "${tiles//|/[%]$'\t'}[%]"
egrep "^($tiles) +[0-9]+ +[0-9]+ +[0-9]+ +[0-9.]+% +[0-9.]+%$" | awk -v raw="$tiles" '
BEGIN {
	n = split(raw, tiles, "|");
	expect = 1;
}
{
	tile = $1;
	win = $6;
	
	while (tile < tiles[expect]) {
		printf "0.00%%";
		expect = expect % n + 1;
		printf (expect != 1 ? "\t" : "\n");
	}
	
	while (tile != tiles[expect]) {
		printf "100.00%%\t";
		expect++;
	}
	
	printf "%s", win;
	expect = expect % n + 1;
	printf (expect != 1 ? "\t" : "\n");
}
END {
	if (!tile) {
		for (i = 1; i < n; i++) printf "100.00%%\t";
		printf "100.00%%\n";
	}
	while (expect != 1) {
		printf "0.00%%";
		expect = expect % n + 1;
		printf (expect != 1 ? "\t" : "\n");
	}
}'
