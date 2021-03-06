#!/bin/bash
[ -e 2048 ] || make OUTPUT=2048 || exit $?
[ -e 2048-4x6patt ] || make 4x6patt OUTPUT=2048-4x6patt || exit $?
if [ ${1:-$(nproc)} != exec ]; then
	interrupt() { echo; echo Interrupted; exit 0; }
	trap interrupt SIGINT
	random-string() { tr -dc ${2:-[:alnum:]} < /dev/urandom | head -c ${1:-32}; }
	for i in $(seq 1 1 ${1:-$(nproc)}); do
		while $0 exec; do random-string 1 [:graph:]; done &
	done
	wait -n
	echo; echo -n "Execution failed! "
	pkill 2048 >/dev/null 2>&1
else
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
fi