#!/bin/bash
#Program: concentrator of TDL2048+ log
#History: 2019-05-20 moporgic

#This script takes parsed log as input
#so run 2048-parse before this script!
#AVG	MAX	TILE	WIN
#62899	109068	8192	98.40%
#104104	173168	8192	99.40%

awk -v NUM=$([ $1 ] && echo $1 || echo 1000) '{
	if (match($0, /^[0-9]+.[0-9]+.[0-9]+.[0-9.]+%$/)) {
		AVG += $1;
		MAX = ($2 > MAX) ? $2 : MAX;
		TILE = ($3 > TILE) ? $3 : TILE;
		WIN += substr($4, 1, length($4)-1);
		if (++N >= NUM) {
			printf "%.0f\t%d\t%d\t%.2f%%\n", AVG/N, MAX, TILE, WIN/N;
			AVG = MAX = TILE = WIN = N = 0;
		}
	} else {
		print $0
	}
} END {
	if (N > 0) {
		printf "%.0f\t%d\t%d\t%.2f%%\t(last %d)\n", AVG/N, MAX, TILE, WIN/N, N;
	}
}'

exit 0