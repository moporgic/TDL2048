#!/bin/bash
# this script collects 2048 related benchmarking and testing procedures

# default benchmarking kernel, will be invoked by "bench" and "compare"
# note: this function links to a specialized function set by variable "test"
test() { ${test:-test-st} "${@:-./2048}"; }
# specialized benchmarking kernels, should be wrapped with "test" before use
# usage: test-[s|m]t [binary:./2048] [attempt:1x10000|$(nproc)0]{2}
#        test-[t|e]-[s|m]t [binary:./2048] [attempt:1x10000|$(nproc)0]
test-st() (
	run=$(runas "${1:-./2048}") || return $?
	taskset -cp $(<<< "${taskset[@]}" tr "; " ,) $BASHPID >/dev/null
	opti=(${2} 1x${ratio:-10}000)
	eval=(${3} ${2/#0*/} 1x${ratio:-10}000)
	(( ${opti/x*/} )) && run+=(-t $opti)
	(( ${eval/x*/} )) && run+=(-e $eval)
	${run[@]} -% -s | grep summary | egrep -o [0-9.]+ops
)
test-mt() (
	run=$(runas "${1:-./2048}") || return $?
	taskset -cp $(<<< "${taskset[@]}" tr "; " ,) $BASHPID >/dev/null
	taskset=($(<<< "${taskset[@]}" tr ";" " "))
	taskset=(${taskset[@]:-""})
	num=$(($(nproc)*${ratio:-10}/${#taskset[@]}))
	opti=(${2} $num)
	eval=(${3} ${2/#0*/} $num)
	xsplit=-L${#taskset[@]}
	(( ${opti/x*/} )) && run+=(-t $opti) || unset xsplit
	(( ${eval/x*/} )) && run+=(-e $eval) || unset xsplit
	{	for cores in ${taskset[@]:-""}; do
			taskset -pc $cores $BASHPID >/dev/null
			${run[@]} -p $(nproc) -% -s &
		done; wait
	} | grep summary | egrep -o [0-9.]+ops | xargs $xsplit | sed -e "s/ /+/g" -e "s/ops//g" | \
		xargs printf "scale=2;%s;\n" | bc -l | xargs -I% echo %ops | grep .
)
test-t-st() { test-st "${1:-./2048}" ${2:-1x10000} 0; }
test-e-st() { test-st "${1:-./2048}" 0 ${2:-1x10000}; }
test-t-mt() { test-mt "${1:-./2048}" ${2:-""} 0 ${3}; }
test-e-mt() { test-mt "${1:-./2048}" 0 ${2:-""} ${3}; }

# benchmark binaries
# usage: bench [binary:./2048]... [attempt:10]
bench() (
	(( $# )) || set -- ./2048
	run=() res=()
	while opt=$(runas "$1") || (( $# > 1 )); do
		[[ $opt ]] && run+=("$opt")
		shift
	done 2>/dev/null
	num=${1:-10}
	line() { <<< "${@//ops/}" tr ' ' '\n'; }
	mean() { <<< "scale=2;(${@//ops/+}0)/$#" bc -l; }
	for i in $(seq -w 1 $num); do
		echo -n "#$i: "
		new=($(for opt in "${run[@]}"; do ${norm:-line} $(test "$opt"); done))
		res=($(paste -d+ <(line "${res[@]}") <(line "${new[@]}")))
		echo ${new[@]/%/ops}
		sleep 1
	done
	echo -n ">$(sed 's/./>/g' <<< $i)> "
	for ops in ${res[@]}; do
		<<< "scale=2;(${ops#+})/$num" bc -l
	done | xargs -I% echo %ops | xargs | grep .
)

# compare binaries
# usage: compare [binary:./2048]... [attempt:10]
compare() { norm=${norm:-mean} bench "$@"; }

# benchmark binaries completely
# usage: [option]... benchmark [binary:./2048]...
# configurable options: recipes, networks, threads, order, taskset, N_init, N_load
benchmark() (
	echo "TDL2048+ Benchmark @ $(hostname) @ ${when:=$(date +'%F %T')}"
	envinfo | stdbuf -o0 sed "s/^/# /g"
	sleep 5

	recipes=($(<<< "${@:-${recipes[@]:-2048}}" tr ";" " "))
	networks=($(<<< "${networks[@]:-4x6patt 8x6patt}" tr ";" " "))
	threads=($(<<< "${threads[@]:-single multi}" tr ";" " "))
	order=($(<<< "${order[@]:-network thread recipe}" tr ";" " "))
	N_init=${N_init:-4}
	N_load=${N_load:-2}

	tokens=$(eval echo $(for o in ${order[@]}; do
		vars=$(eval 'for var in ${'${o}'s[@]:?}; do echo _${var}_; done' | xargs | tr ' ' ',')
		<<< "$vars" grep -q , && vars={$vars}
		echo -n $vars
	done))

	for token in ${tokens[@]}; do
		token=(${token//_/ })
		declare ${order[0]}=${token[0]} ${order[1]}=${token[1]} ${order[2]}=${token[2]}

		echo "[${recipe:?}] ${network:?} ${thread:?}-thread"
		echo -n ">"

		run=$(runas "${recipe:?}")
		run_init="${run:?} -n $network"
		run_load="${run:?} -n $network -i $network.w -a 0"

		if (( $N_init )); then
			test="test-${thread:0:1}t" \
				bench "$run_init" $N_init | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
			sleep 1
		fi
		if (( $N_load )) && [ -e $network.w ]; then
			test="test-${thread:0:1}t" \
				bench "$run_load" $N_load | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
			sleep 1

			test="test-e-${thread:0:1}t" \
				bench "$run_load" $N_load | tail -n1 | egrep -o [0-9.][0-9.]+ops | xargs echo -n ""
			sleep 1
		fi
		echo
	done
)

# output the correct executable for command line
runas() (
	2048() { return 1; }
	run="$@"
	for run in "$run" "./$run" ""; do
		$run -\| -n none -e 0 && break
	done >/dev/null 2>&1
	echo ${run:?\'$@\' is unavailable}
)
# measure CPU performance in GHz
cpu-perf() (
	$(ls ./2048* | head -n1) -n 4x6patt -t ${1:-1}00 -p ${1:-1} >/dev/null &
	sleep ${2:-5}
	speed=$(grep MHz /proc/cpuinfo | cut -d':' -f2 | cut -b2- | sort -n | tail -n${1:-1})
	pkill -P $(jobs -p)
	kill $(jobs -p)
	printf "%.1f\n" $(<<< "(${speed//$'\n'/+})/$(<<< "$speed" wc -l)000" bc -l)
)
# display the current environment
envinfo() (
	# CPU model
	cpuinfo=$(grep -m1 name /proc/cpuinfo | sed -E 's/.+:|\(\S+\)|CPU|[0-9]+-Core.+|@.+//g' | xargs)
	nodes=$(lscpu | grep 'NUMA node(s)' | cut -d: -f2 | xargs)
	(( ${nodes:-1} > 1 )) && cpuinfo+=" x$nodes"
	# available cores
	taskset -cp $(<<< "${taskset[@]}" tr "; " ,) $BASHPID >/dev/null
	taskset=$(<<< "${taskset[@]}" tr ' ' ';')
	nproc=$(for cpus in ${taskset//;/ }; do echo $(taskset -c $cpus nproc)x; done | xargs)
	[[ $taskset ]] && nproc="${nproc// /|} ($taskset)" || nproc=$(nproc)x
	# CPU speed
	perf=$(cpu-perf 1)G
	(( $(nproc) > 1 )) && perf+=-$(cpu-perf $(nproc))G
	# memory info
	meminfo=$(sudo -n lshw -short -c memory 2>/dev/null | egrep -v "BIOS|cache|empty")
	if [[ $meminfo ]]; then
		dimm=$(<<< "$meminfo" grep "DIMM" | cut -d' ' -f2-)
		type=($(<<< "$dimm" grep -o "DDR."))
		speed=($(<<< "$dimm" egrep -o "\S+ MHz"))
		size=$(<<< "$meminfo" grep "System Memory" | egrep -Eo "[0-9]+[MGT]")B
		meminfo="$type-$speed x$(<<< "$dimm" wc -l) $size"
	else # if memory info cannot be retrieved
		size=($(head -n1 /proc/meminfo))
		meminfo=$(printf "DRAM %.1fG" $(<<< "${size[1]}/1024/1024" bc -l))
	fi

	echo "$cpuinfo @ $nproc $perf + $meminfo"

	# git commit
	commit=($(git log -n1 --format=%h 2>/dev/null) "???????")
	git status -uno 2>/dev/null | grep -iq changes && commit+="+x"
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
	[[ $makeinfo ]] && gccinfo+=" $makeinfo"
	# current time
	when=${when:=$(date +'%F %T')}

	echo "$commit @ $osinfo + $gccinfo @ $when"
)

# ======================================== main routine ========================================
# check whether is running as benchmark or as source
# usage as benchmark: $0 [-D|-P|-p][=45678sm] [-c=cpu_list] [binary]...
#       as source:    . $0
if (( $# + ${#recipes} )) && [ "$0" == "$BASH_SOURCE" ]; then ( # execute benchmarks
	while (( $# )); do
		case $1 in
		-c*) taskset=${1:2}; taskset=${taskset#=}; ;;
		-*)  options+=${1:1}; ;;
		*)   recipes+=${recipes:+ }$1; ;;
		esac; shift
	done
	networks=$(<<< "$options" egrep -o [0-9] | sort | uniq | xargs -I% echo %x6patt)
	threads=$(<<< "$options" egrep -o [sm] | sort -r | uniq | sed -e "s/s/single/g" -e "s/m/multi/g")

	for network in ${networks:-4x6patt 8x6patt}; do
		[ -e $network.w ] && continue
		echo "Retrieving \"$network.w\" from moporgic.info..."
		curl -OJRfs "moporgic.info/data/2048/$network.w.xz" && xz -d $network.w.xz || \
			echo "Error: \"$network.w\" is unavailable"
	done

	output() { tee -a 2048-bench.log; }
	if [[ $recipes ]]; then # execute dedicated benchmarks
		[[ $options =~ [DPp] ]] && echo ========== Benchmarking Dedicated TDL2048+ ==========
		benchmark $recipes | output || exit $?
	fi
	prefix() { xargs -d\\n -n1 echo \>; }
	if [[ $options =~ [D] ]]; then # build and benchmark default target
		echo ============= Building Default TDL2048+ =============
		make OUTPUT=2048 | prefix || exit $?
		echo =========== Benchmarking Default TDL2048+ ===========
		benchmark 2048 | output || exit $?
	fi
	if [[ $options =~ [Pp] ]]; then # build and benchmark profiled target
		prefix-fix() { sed -u "/^>/d" | prefix; }
		declare -A profiled
		for network in ${networks:-4x6patt 8x6patt}; do
			echo ======== Building $network-Profiled TDL2048+ =========
			make $network OUTPUT=2048-$network | prefix-fix || exit $?
			profiled[$network]="2048-$network"
			if [[ $options =~ [P] ]]; then
				make $network PGO_EVAL=0 OUTPUT=2048-$network-t | prefix-fix || exit $?
				make $network PGO_OPTI=0 OUTPUT=2048-$network-e | prefix-fix || exit $?
				profiled[$network]+=" 2048-$network-t 2048-$network-e"
			fi
		done
		output-fix() { output; }
		echo ========== Benchmarking Profiled TDL2048+ ===========
		for network in ${networks:-4x6patt 8x6patt}; do
			networks=$network makeinfo=profile benchmark ${profiled[$network]} | output-fix || exit $?
			output-fix() { stdbuf -o0 tail -n+4 | output; }
		done
	fi
) elif [ "$0" != "$BASH_SOURCE" ]; then # otherwise print help info if script is sourced
	echo "=========== Benchmarking Scripts Manual ============"
	echo "usage: test [binary:./2048] [attempt:1x10000|$(nproc)0]"
	echo "       available suffixes are -st -mt -t-st|mt -e-st|mt"
	echo "usage: bench [binary:./2048]... [attempt:10]"
	echo "       this function uses \"test\" as kernel"
	echo "usage: compare [binary:./2048]... [attempt:10]"
	echo "       this function uses \"test\" as kernel"
	echo "usage: benchmark [binary:./2048]..."
	echo "       this function uses \"bench\" as kernel"
fi