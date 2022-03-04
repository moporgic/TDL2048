#!/bin/bash
# this script collects 2048 related benchmarking and testing procedures

# default benchmarking kernel, will be invoked by "bench" and "compare"
# note: this function is usually linked to a specialized function below
test() { test-st "${@:-./2048}"; }
# specialized benchmarking kernels, should be wrapped with "test" before use
# usage: test-[s|m]t [binary:./2048] [attempt:1x10000|$(nproc)0]{2} [thread:1|$(nproc)]
#        test-[t|e]-[s|m]t [binary:./2048] [attempt:1x10000|$(nproc)0] [thread:1|$(nproc)]
test-st() {
	run=$(name "${1:-./2048}")
	invoke=${taskset:+taskset -c ${taskset[@]//;/,}}
	$invoke $run -s -t ${2:-1x10000} -e $(echo-1st ${3} ${2/#0*/} 1x10000) -% | sum-ops
}
test-mt() {
	run=$(name "${1:-./2048}")
	tasks=(${taskset[@]//;/ }); tasks=(${tasks[@]:-""})
	N_exec=$(($(nproc)0/${#tasks[@]}))
	N_opti=${2:-$N_exec}
	N_eval=$(echo-1st ${3} ${2/#0*/} $N_exec)
	xsplit=-L${#tasks[@]}
	(( ${N_opti/x*/} )) || unset N_opti xsplit
	(( ${N_eval/x*/} )) || unset N_eval xsplit
	{	for cores in ${tasks[@]:-""}; do
			invoke=${cores:+taskset -c $cores}
			N_proc=${4:-$($invoke nproc)}
			$invoke $run -s ${N_opti:+-t $N_opti} ${N_eval:+-e $N_eval} -p $N_proc -% &
		done; wait
	} | sum-ops | xargs $xsplit | sed -e "s/ /+/g" -e "s/ops//g" | \
		xargs printf "scale=2;%s;\n" | bc -l | sed -e "s/$/ops/g" | grep .
}
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
		ops0+=${ops0:++}${res[0]}
		ops1+=${ops1:++}${res[1]}
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
		Lx+=+$(<<< "scale=2;$L" bc -l)
		Rx+=+$(<<< "scale=2;$R" bc -l)
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
	PID=$BASHPID
	tasksav=$(taskset -cp ${taskset[@]//;/,} $PID | head -n1 | cut -d':' -f2 | xargs)
	taskset=${taskset:-$tasksav}

	recipes=${@:-${recipes:-2048}}
	networks=${networks:-4x6patt 8x6patt}
	threads=${threads:-single multi}
	N_init=${N_init:=4}
	N_load=${N_load:-2}

	echo "TDL2048+ Benchmark @ $(hostname) @ ${when:=$(date +'%F %T')}"
	envinfo | stdbuf -o0 sed "s/^/# /g"

	for network in $networks; do
		[ -e $network.w ] || ! (( $N_load )) && continue
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
				if (( $N_init )); then
					test() { test-${thread:0:1}t "$1 -n $network" ${@:2}; }
					bench $runas $N_init | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 1
				fi
				if (( $N_load )) && [ -e $network.w ]; then
					test() { test-${thread:0:1}t "$1 -n $network -i $network.w -a 0" ${@:2}; }
					bench $runas $N_load | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 1

					test() { test-e-${thread:0:1}t "$1 -n $network -i $network.w -a 0" ${@:2}; }
					bench $runas $N_load | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
					sleep 1
				fi
				echo
			done
		done
	done

	taskset -cp $tasksav $PID >/dev/null
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
# echo the 1st argument
echo-1st() { echo "$1"; }
# measure CPU performance in GHz
cpu-perf() {
	$(ls ./2048* | head -n1) -n 4x6patt -t ${1:-1}00 -p ${1:-1} >/dev/null &
	sleep ${2:-5}
	speed=$(grep MHz /proc/cpuinfo | cut -d':' -f2 | cut -b2- | sort -n | tail -n${1:-1})
	pkill -P $(jobs -p)
	kill $(jobs -p)
	printf "%.1f\n" $(<<< "(${speed//$'\n'/+})/$(<<< $speed wc -l)000" bc -l)
}
# display the current environment
envinfo() { (
	# CPU model
	cpuinfo=$(grep -m1 name /proc/cpuinfo | sed -E 's/.+:|\(\S+\)|CPU|[0-9]+-Core.+|@.+//g' | xargs)
	nodes=$(lscpu | grep 'NUMA node(s)' | cut -d: -f2 | xargs)
	(( ${nodes:-1} > 1 )) && cpuinfo+=" x$nodes"
	# available cores
	nproc=($(for cpus in ${taskset[@]//;/ }; do echo $(taskset -c $cpus nproc)x; done))
	nproc=$(<<< ${nproc[@]:-$(nproc)x} tr ' ' '|')
	[[ ${taskset[@]} ]] && nproc+=" (${taskset[@]// /;})"
	# CPU speed
	perf=$(cpu-perf 1)G-$(cpu-perf $(nproc))G
	# memory info
	meminfo=$(sudo -n lshw -short -c memory 2>/dev/null | egrep -v "BIOS|cache|empty")
	if [[ $meminfo ]]; then
	    dimm=$(<<< $meminfo grep "DIMM" | cut -d' ' -f2-)
	    type=($(<<< $dimm grep -o "DDR."))
	    speed=($(<<< $dimm egrep -o "\S+ MHz"))
	    size=$(<<< $meminfo grep "System Memory" | egrep -Eo "[0-9]+[MGT]")B
	    meminfo="$type-$speed x$(<<< $dimm wc -l) $size"
	else # if memory info cannot be retrieved
	    size=($(head -n1 /proc/meminfo))
	    meminfo=$(printf "DRAM %.1fG" $(<<< "${size[1]}/1024/1024" bc -l))
	fi

	echo "$cpuinfo @ $nproc $perf + $meminfo"

	# git commit
	commit=($(git log 2>/dev/null | head -n1 | cut -b8-14) "???????")
	git status -uno 2>/dev/null | grep -iq changes && commit+="+?"
	# OS name and version
	osinfo=$(uname -o 2>/dev/null | sed "s|GNU/||")
	[[ $(uname -r) == *lts* ]] && osinfo+=" LTS"
	osinfo+=" $(uname -r | sed -E 's/[^0-9.]+.+$//g')"
	if [[ $OSTYPE =~ cygwin|msys ]]; then
		ver=($(cmd /c ver 2>/dev/null | tr "[\r\n]" " "))
		(( ${#ver[@]} )) && osinfo+=" (Windows ${ver[-1]})"
	fi
	# GCC version
	gccinfo=($(gcc --version | head -n1))
	gccinfo="GCC ${gccinfo[-1]}"
	[[ $gcccmt ]] && gccinfo+=" $gcccmt"
	# current time
	when=${when:=$(date +'%F %T')}

	echo "$commit @ $osinfo + $gccinfo @ $when"
) }

# ======================================== main routine ========================================
# check whether is running as benchmark or as source
# usage as benchmark: $0 [-D|-P|-p][=45678sm] [-c=cpu_list] [binary]...
#       as source:    . $0
if (( $# + ${#recipes} )) && [ "$0" == "$BASH_SOURCE" ]; then # execute benchmarks
	while (( $# )); do
		case $1 in
		-D*) default=${1:1}; default=${default//x6patt/}; ;;
		-p*) prof_type=lite; ;&
		-P*) profile=${1:1}; profile=${profile//x6patt/}; ;;
		-c*) taskset=${1:2}; taskset=${taskset/=/}; ;;
		*)   recipes+=${recipes:+ }$1; ;;
		esac; shift
	done
	output() { tee -a 2048-bench.log; }
	prefix() { xargs -d\\n -n1 echo \>; }
	declare -A thdname=([s]=single [m]=multi)
	if [[ $recipes ]]; then ( # execute dedicated benchmarks
		[[ $default$profile ]] && echo ========== Benchmarking Dedicated TDL2048+ ==========
		benchmark $recipes | output || exit $?
	) fi
	if [[ $default ]]; then ( # build and benchmark default target
		networks=$(<<< $default egrep -o [0-9] | xargs -I% echo %x6patt)
		threads=$(eval echo $(<<< $default egrep -Eo [sm] | xargs -I{} echo "\${thdname[{}]}"))
		echo ============= Building Default TDL2048+ =============
		make OUTPUT=2048 | prefix || exit $?
		echo =========== Benchmarking Default TDL2048+ ===========
		networks=$networks threads=$threads benchmark 2048 | output || exit $?
	) fi
	if [[ $profile ]]; then ( # build and benchmark profiled target
		networks=$(<<< $profile egrep -o [0-9] | xargs -I% echo %x6patt)
		threads=$(eval echo $(<<< $profile egrep -Eo [sm] | xargs -I{} echo "\${thdname[{}]}"))
		[ ${prof_type:-full} == full ] && profiled="{,-t,-e}" || profiled=
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
			networks=$network threads=$threads gcccmt=profile benchmark $recipes | output-fix || exit $?
			output-fix() { stdbuf -o0 tail -n+4 | output; }
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