#!/bin/bash
(( $RANDOM % 2 )) && run=./2048 || run=./2048-4x6patt
if (( $RANDOM % 2 )); then
	alpha=0.1
	valid=0022bfc2d024f21126e115f309cff4aa5c9d0e2a
else
	alpha=1.0
	valid=f6f07854e774a307f3087b1f41211f9acc25b341
fi
cksum=($($run -s -a $alpha -t 1x10000 -e 1x10000 -% none | grep ^local | sha1sum))
[ $cksum == $valid ]
