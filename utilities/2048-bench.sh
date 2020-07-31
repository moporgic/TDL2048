#!/bin/bash

# default benchmarking kernel, will be invoked by "bench" and "compare"
test() { command -v ${1:-./2048} >/dev/null 2>&1 && test-st ${@:-./2048} || test-st ./${@:-./2048}; }
# specialized benchmarking kernels, should be wrapped with "test" before use
test-st() { ${1:-./2048} -s -t 1x${2:-10000} -e 1x${2:-10000} -% none | egrep -o [0-9.]+ops; }
test-mt() { ${1:-./2048} -s -t ${2:-$(nproc)0} -e ${2:-$(nproc)0} -p -% | grep summary | egrep -o [0-9.]+ops; }
test-e-st() { echo nanops; ${1:-./2048} -s -e 1x${2:-10000} -% none | egrep -o [0-9.]+ops; }
test-e-mt() { echo nanops; ${1:-./2048} -s -e ${2:-$(nproc)0} -p -% | grep summary | egrep -o [0-9.]+ops; }

# benchmarking routine
# usage: bench [binary:./2048] [attempt:10]
# note: kernel function "test" should be defined before use
bench () {
	run=${1:-2048}; [ -e $run ] && run=./$run
	command -v $run >/dev/null 2>&1 || exit 1
	optimize=();
	evaluate=();
	for i in $(seq -w 1 1 ${2:-10}); do
		echo -n "#$i: ";
		res=($(test $run));
		echo ${res[@]};
		res=(${res[@]//ops/});
		optimize+=(${res[0]});
		evaluate+=(${res[1]});
		sleep 1;
	done;
	optimize=${optimize[@]};
	evaluate=${evaluate[@]};
	echo -n ">$(sed "s/./>/g" <<< $i)> "
	echo $(bc -l <<< "scale=2; (${optimize// /+})/${2:-10}")ops $(bc -l <<< "scale=2; (${evaluate// /+})/${2:-10}")ops | egrep --color=auto [0-9.]+ops
}

# comparing two binaries
# usage: compare [binary1:./2048] [binary2:./2048] [attempt:10]
# note: kernel function "test" should be defined before use
compare() {
	lc=${1:-2048}; [ -e $lc ] && lc=./$lc
	rc=${2:-2048}; [ -e $rc ] && rc=./$rc
	command -v $lc >/dev/null 2>&1 || exit 1
	command -v $rc >/dev/null 2>&1 || exit 2
	lhs=0; rhs=0;
	Lx=(); Rx=();
	for i in $(seq -w 1 1 ${3:-10}); do
		echo -n "#$i: "
		if (( ${i: -1} % 2 == 0 )); then
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
		Lx+=($(bc -l <<< "scale=2; ($lhs)/$((i+i))"))
		Rx+=($(bc -l <<< "scale=2; ($rhs)/$((i+i))"))
		echo ${Lx[-1]}ops ${Rx[-1]}ops
		sleep 1
	done
	Lx=${Lx[@]};
	Rx=${Rx[@]};
	echo -n ">$(sed "s/./>/g" <<< $i)> "
	echo $(bc -l <<< "scale=2; (${Lx// /+})/${3:-10}")ops $(bc -l <<< "scale=2; (${Rx// /+})/${3:-10}")ops | egrep --color=auto [0-9.]+ops
}

# full benchmarking routine
# usage: benchmark [binary:./2048]
# note: kernel functions "bench" and "test-*" should be defined before use
#       this procedure will automatically bind "test-*" as "test" for "bench"
#       configurable variables: recipes, networks, threads, N_init, N_load
benchmark() {
	if [ -e init ] || [ -e load ]; then
		echo "Error: \"init\" and \"load\" are reserved names" >&2
		exit 7
	fi

	recipes="${@:-${recipes:-2048}}"
	networks="${networks:-"4x6patt 8x6patt"}"
	threads="${threads:-"single multi"}"

	(( ${N_load:-4} )) && for network in $networks; do
		[ -e $network.w ] && continue
		read -p "Download $network.w from moporgic.info? [Y/n] " >&2
		[[ $REPLY =~ ^[Nn] ]] && continue
		wget -nv "moporgic.info/data/2048/$network.w.xz" >&2 || continue
		pixz -kd $network.w.xz || xz -kd $network.w.xz
		touch -r $network.w.xz $network.w && rm $network.w.xz
	done
	sleep 10

	echo TDL2048+ Benchmark @ $(hostname) @ $(date +"%F %T")
	for recipe in $recipes; do
		[ -e $recipe ] && runas=./$recipe || runas=$recipe
		command -v $runas >/dev/null 2>&1 || continue

		for network in $networks; do
			init() { $runas -n $network $@; }
			load() { $runas -n $network -i $network.w -a 0 $@; }

			for thread in $threads; do
				echo "[$recipe] $network $thread-thread"
				echo -n ">"

				if (( ${N_init:-8} )); then
					test() { test-${thread:0:1}t $@; }
					bench init ${N_init:-8} | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 10
				fi
				if [ -e $network.w ] && (( ${N_load:-4} )); then
					test() { test-${thread:0:1}t $@; }
					bench load ${N_load:-4} | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 10

					test() { test-e-${thread:0:1}t $@; }
					bench load ${N_load:-4} | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 10
				fi

				echo
			done
		done
	done
}

# execute benchmark automatically if recipes are given
if (( $# )); then
	benchmark "$@" | tee -a 2048-bench.log
else # otherwise, print help info
	echo "usage: test [binary:./2048]"
	echo "       available suffixes are -st -mt -e-st -e-mt"
	echo "usage: bench [binary:./2048] [attempt:10]"
	echo "       this function uses \"test\" as kernel"
	echo "usage: compare [binary1:./2048] [binary2:./2048] [attempt:10]"
	echo "       this function uses \"test\" as kernel"
	echo "usage: benchmark [binaries:./2048]..."
	echo "       this function uses \"bench\" as kernel"
fi