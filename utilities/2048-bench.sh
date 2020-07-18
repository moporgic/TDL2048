#!/bin/bash
# benchmarking targets
# if input arguments are provided, this script will run benchmarking automatically
recipes="$@"
networks="4x6patt 8x6patt"
threads="single multi"

# benchmarking routine
# usage: bench [binary:./2048] [attempt:10]
# note: kernel function "test" should be defined before use
bench () {
	optimize=();
	evaluate=();
	for i in $(seq -w 1 1 ${2:-10}); do
		echo -n "loop $i: ";
		res=($(test ${1:-./2048}));
		echo ${res[@]};
		res=(${res[@]//ops/});
		optimize+=(${res[0]});
		evaluate+=(${res[1]});
		sleep 1;
	done;
	optimize=${optimize[@]};
	evaluate=${evaluate[@]};
	echo average: $(bc -l <<< "scale=2; (${optimize// /+})/${2:-10}")ops $(bc -l <<< "scale=2; (${evaluate// /+})/${2:-10}")ops | egrep --color=auto [0-9.]+ops
}

# comparing two binaries
# usage: compare [binary1:./2048] [binary2:./2048] [attempt:10]
# note: kernel function "test" should be defined before use
compare() {
	lc=${1:-./2048}
	rc=${2:-./2048}
	[ -e $lc ] || exit 1
	[ -e $rc ] || exit 2
	lhs=0; rhs=0;
	for i in $(seq 1 1 ${3:-10}); do
		echo -en "#$i\t"
		if (( i % 2 == 0 )); then
			L=($(test $lc))
			R=($(test $rc))
		else
			R=($(test $rc))
			L=($(test $lc))
		fi
		L=(${L[@]%ops})
		lhs=$lhs+${L[0]}+${L[1]}
		R=(${R[@]%ops})
		rhs=$rhs+${R[0]}+${R[1]}
		echo -en "$(bc -l <<< "scale=2; ($lhs)/$((i+i))")\t"
		echo -en "$(bc -l <<< "scale=2; ($rhs)/$((i+i))")\n"
		sleep 1
	done
}

# specialized benchmarking kernels, should be wrapped with "test" before use
test-st() { ${1:-./2048} -s -t 1x${2:-10000} -e 1x${2:-10000} -% none | egrep -o [0-9.]+ops; }
test-mt() { ${1:-./2048} -s -t ${2:-$(nproc)0} -e ${2:-$(nproc)0} -p -% | grep summary | egrep -o [0-9.]+ops; }
test-e-st() { echo nanops; ${1:-./2048} -s -e 1x${2:-10000} -% none | egrep -o [0-9.]+ops; }
test-e-mt() { echo nanops; ${1:-./2048} -s -e ${2:-$(nproc)0} -p -% | grep summary | egrep -o [0-9.]+ops; }
# default benchmarking kernel, will be invoked by "bench" and "compare"
test() { test-st $@; }

# main, will not execute if no recipes is given
if [ "$recipes" ]; then
	sleep 10
	echo TDL2048+ Benchmark @ $(hostname) @ $(date +"%F %T")
	for recipe in $recipes; do
		[ -e $recipe ] || continue
		
		for network in $networks; do
			for thread in $threads; do
				echo "$recipe $network $thread-thread"
				echo -n ">"
	
				test() { test-${thread:0:1}t $@; }
				run() { ./$recipe -n $network $@; }
				bench run 8 | grep -v loop | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
				sleep 10
	
				if [ -e $network.w ]; then
					test() { test-${thread:0:1}t $@; }
					run() { ./$recipe -n $network -i $network.w -a 0 $@; }
					bench run 4 | grep -v loop | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 10
		
					test() { test-e-${thread:0:1}t $@; }
					run() { ./$recipe -n $network -i $network.w -a 0 $@; }
					bench run 4 | grep -v loop | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 10
				fi
	
				echo
			done
		done
	done
done | tee -a 2048-bench.log