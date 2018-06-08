#!/bin/bash
#Program: parser of TDL2048+
#History: 2018/04/01 moporgic
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

#001/500 1765ms 596549.01ops [8]
#local:  avg=18580 max=51140 tile=4096 win=28.60%
#total:  avg=18580 max=51140 tile=4096 win=28.60%

echo -e "AVG\tMAX\tTILE\tWIN"
while read -a res; do
	if [ "${res[0]}" != "local:" ] && [ "${res[0]}" != "total:" ]; then
		echo failed at ${res[@]} 1>&2
		exit 1
	fi
	avg=${res[1]}
	max=${res[2]}
	tile=${res[3]}
	win=${res[4]}
	if [ ${#res[@]} -gt 5 ] || \
	   [ "${avg:0:3}" != "avg" ] || \
	   [ "${max:0:3}" != "max" ] || \
	   [ "${tile:0:4}" != "tile" ] || \
	   [ "${win:0:3}" != "win" ]; then
		echo failed at ${res[@]} 1>&2
		exit 1
	fi
	index=$((${index:-0}+1))
	echo -e "${avg:4}\t${max:4}\t${tile:5}\t${win:4}"
done

exit 0