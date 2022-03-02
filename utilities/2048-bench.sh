#!/bin/bash
# this script collects 2048 related benchmarking and testing procedures

# default benchmarking kernel, will be invoked by "bench" and "compare"
# note: this function is usually linked to a specialized function below
test() { test-st "${@:-./2048}"; }
# specialized benchmarking kernels, should be wrapped with "test" before use
# usage: test-[s|m]t [binary:./2048] [attempt:1x10000|$(nproc)0]{2} [thread:N/A|$(nproc)]
test-st() {
	run=$(name "${1:-./2048}")
	invoke=${taskset:+taskset -c ${taskset[@]//;/,}}
	$invoke $run -s -t ${2:-1x10000} -e $(basename ${3:-${2/#0*/}} 1x10000) -% | sum-ops
}
test-mt() {
	run=$(name "${1:-./2048}")
	tasks=(${taskset[@]//;/ })
	N_exec=$(($(nproc)0 / (${#tasks[@]} ? ${#tasks[@]} : 1)))
	N_opti=${2:-$N_exec}
	N_eval=${3:-$(($N_opti ? $N_opti : $N_exec))}
	both=-L2
	(( $N_opti )) || unset N_opti both
	(( $N_eval )) || unset N_eval both
	{	for cores in ${tasks[@]:-""}; do
			invoke=${cores:+taskset -c $cores}
			N_proc=${4:-$($invoke nproc)}
			$invoke $run -s ${N_opti:+-t $N_opti} ${N_eval:+-e $N_eval} -p $N_proc -% &
		done; wait
	} | sum-ops | xargs $both | sed -e "s/ /+/g" -e "s/ops//g" | \
		xargs printf "scale=2;%s;\n" | bc -l | sed -e "s/$/ops/g" | grep .
}
# usage: test-[t|e]-[s|m]t [binary:./2048] [attempt:1x10000|$(nproc)0] [thread:N/A|$(nproc)]
test-t-st() { test-st "${1:-./2048}" ${2:-1x10000} 0; }
test-e-st() { test-st "${1:-./2048}" 0 ${2:-1x10000}; }
test-t-mt() { test-mt "${1:-./2048}" ${2:-""} 0 ${3}; }
test-e-mt() { test-mt "${1:-./2048}" 0 ${2:-""} ${3}; }

# benchmarking routine
# usage: bench [binary:./2048] [attempt:10]
# note: kernel function "test" should be defined before use
bench () {
	run=$(name "${1:-./2048}")
	unset ops0 ops1
	for i in $(seq -w 1 1 ${2:-10}); do
		echo -n "#$i: "
		res=($(test "$run"))
		echo ${res[@]}
		res=(${res[@]//ops/})
		ops0=${ops0:+${ops0}+}${res[0]}
		ops1=${ops1:+${ops1}+}${res[1]}
		sleep 1
	done
	echo -n ">$(sed "s/./>/g" <<< $i)> "
	for ops in $ops0 $ops1; do
		<<< "scale=2;(${ops})/${2:-10}" bc -l
	done | sed "s/$/ops/g" | tr '\n' ' ' | grep .
}

# comparing two binaries
# usage: compare [binary1:./2048] [binary2:./2048] [attempt:10]
# note: kernel function "test" should be defined before use
compare() {
	Lc=$(name "${1:-./base}")
	Rc=$(name "${2:-./2048}")
	Lx=0
	Rx=0
	for i in $(seq -w 1 1 ${3:-10}); do
		echo -n "#$i: "
		if (( ${i: -1} % 2 == 0 )); then
			L=($(test "$Lc"))
			R=($(test "$Rc"))
		else
			R=($(test "$Rc"))
			L=($(test "$Lc"))
		fi
		L=(${L[@]//ops/+}0)/${#L[@]}
		R=(${R[@]//ops/+}0)/${#R[@]}
		Lx=$Lx+$(<<< "scale=2;$L" bc -l)
		Rx=$Rx+$(<<< "scale=2;$R" bc -l)
		echo ${Lx##*+}ops ${Rx##*+}ops
		sleep 1
	done
	echo -n ">$(sed "s/./>/g" <<< $i)> "
	echo $(<<< "scale=2;(${Lx})/${3:-10}" bc -l)ops \
	     $(<<< "scale=2;(${Rx})/${3:-10}" bc -l)ops | grep .
}

# full benchmarking routine
# usage: benchmark [binary:./2048]
# note: kernel functions "bench" and "test-*" should be defined before use
#       this procedure will automatically bind "test-*" as "test" for "bench"
#       configurable variables: recipes, networks, threads, taskset, N_init, N_load
benchmark() {
	echo TDL2048+ Benchmark @ $(hostname) @ $(date +"%F %T")

	recipes=${@:-${recipes:-2048}}
	networks=${networks:-4x6patt 8x6patt}
	threads=${threads:-single multi}

	(( ${N_load:-2} )) && for network in $networks; do
		[ -e $network.w ] && continue
		echo "Retrieving \"$network.w\" from moporgic.info..." >&2
		curl -OJRfs moporgic.info/data/2048/$network.w.xz && xz -d $network.w.xz || {
			echo "Error: \"$network.w\" not available" >&2
			exit 8
		}
	done
	sleep 4

	for recipe in $recipes; do
		runas=$(name "$recipe")

		for network in $networks; do

			for thread in $threads; do
				echo "[$recipe] $network $thread-thread"
				echo -n ">"

				if (( ${N_init:-4} )); then
					test() { test-${thread:0:1}t "$1 -n $network" ${@:2}; }
					bench $runas ${N_init:-4} | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 1
				fi
				if (( ${N_load:-2} )) && [ -e $network.w ]; then
					test() { test-${thread:0:1}t "$1 -n $network -i $network.w -a 0" ${@:2}; }
					bench $runas ${N_load:-2} | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 1

					test() { test-e-${thread:0:1}t "$1 -n $network -i $network.w -a 0" ${@:2}; }
					bench $runas ${N_load:-2} | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 1
				fi

				echo
			done
		done
	done
}

# output the correct executable for command line
name() {
	2048() { return 1; }
	run="$@"
	for run in "$run" "./$run" ""; do
		$run -\| -n none -e 0 && break
	done >/dev/null 2>&1
	unset 2048
	echo ${run:?\'$@\'}
}

# extract ops from summary block
sum-ops() { grep summary | egrep -o [0-9.]+ops; }

# script main routine: check whether is running as benchmark or as source
# usage (as benchmark): $0 -[D|P|p][=45678sm] [binary]...
# usage (as source):    . $0
if (( $# + ${#recipes} )) && [ "$0" == "$BASH_SOURCE" ]; then # execute benchmarks
	while (( $# )); do
		case $1 in
		-D*) default=$1; ;;
		-p*) prof_type=lite; ;&
		-P*) profile=$1; ;;
		*)   recipes+=${recipes:+ }$1; ;;
		esac; shift
	done
	output() { tee -a 2048-bench.log; }
	prefix() { xargs -d\\n -n1 echo \>; }
	x6patt() { sed -u "s/x6patt//g" | egrep -o [0-9] | xargs -I% echo %x6patt; }
	declare -A thdname=([s]=single [m]=multi)
	if [[ $recipes ]]; then ( # execute dedicated benchmarks
		[[ $default$profile ]] && echo ========== Benchmarking Dedicated TDL2048+ ==========
		benchmark $recipes | output || exit $?
	) fi
	if [[ $default ]]; then ( # build and benchmark default target
		[[ $default =~ ^-.[0-9]+$ ]] && networks=$(x6patt <<< $default)
		[[ $default =~ ^-.+=(.+)$ ]] && networks=${BASH_REMATCH[1]//,/ }
		threads=$(eval echo $(<<< $default egrep -Eo [sm] | xargs -I{} echo "\${thdname[{}]}"))
		echo ============= Building Default TDL2048+ =============
		make OUTPUT=2048 | prefix || exit $?
		echo =========== Benchmarking Default TDL2048+ ===========
		networks=$networks threads=$threads benchmark 2048 | output || exit $?
	) fi
	if [[ $profile ]]; then ( # build and benchmark profiled target
		[[ $profile =~ ^-.[0-9]+$ ]] && networks=$(x6patt <<< $profile)
		[[ $profile =~ ^-.+=(.+)$ ]] && networks=${BASH_REMATCH[1]//,/ }
		[ ${prof_type:-full} == full ] && profiled="{,-t,-e}" || profiled=
		threads=$(eval echo $(<<< $profile egrep -Eo [sm] | xargs -I{} echo "\${thdname[{}]}"))
		output-fix() { output; }
		prefix-fix() { sed -u "/^>/d" | prefix; }
		for network in ${networks:-4x6patt 8x6patt}; do
			echo ======== Building $network-Profiled TDL2048+ =========
			make $network OUTPUT=2048-$network | prefix-fix || exit $?
			if [ ${prof_type:-full} == full ]; then
				make $network PGO_EVAL=0 OUTPUT=2048-$network-t | prefix-fix || exit $?
				make $network PGO_OPTI=0 OUTPUT=2048-$network-e | prefix-fix || exit $?
			fi
		done
		echo ========== Benchmarking Profiled TDL2048+ ===========
		for network in ${networks:-4x6patt 8x6patt}; do
			recipes=$(eval echo 2048-$network$profiled)
			networks=$network threads=$threads benchmark $recipes | output-fix || exit $?
			output-fix() { stdbuf -o0 tail -n+2 | output; }
		done
	) fi
elif [ "$0" != "$BASH_SOURCE" ]; then # otherwise print help info if script is sourced
	echo "=========== Benchmarking Scripts Manual ============"
	echo "usage: test [binary:./2048] [attempt:1x10000|$(nproc)0]"
	echo "       available suffixes are -st -mt -t-st|mt -e-st|mt"
	echo "usage: bench [binary:./2048] [attempt:10]"
	echo "       this function uses \"test\" as kernel"
	echo "usage: compare [binary1:./base] [binary2:./2048] [attempt:10]"
	echo "       this function uses \"test\" as kernel"
	echo "usage: benchmark [binaries:./2048]..."
	echo "       this function uses \"bench\" as kernel"
fi