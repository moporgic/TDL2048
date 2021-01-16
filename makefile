INSTS ?= abm bmi bmi2 avx avx2
PGO_ALPHA ?= 0 fixed
PGO_OPTI ?= 1x10000
PGO_EVAL ?= 1x10000

default:
	g++ -std=c++1y -mtune=native $(addprefix -m,$(INSTS)) -O3 -lpthread -Wall -fmessage-length=0 -o 2048 2048.cpp
	
native:
	g++ -std=c++1y -march=native $(addprefix -m,$(INSTS)) -O3 -lpthread -Wall -fmessage-length=0 -o 2048 2048.cpp
	
dump:
	g++ -std=c++1y -mtune=native $(addprefix -m,$(INSTS)) -O3 -lpthread -Wall -fmessage-length=0 -g -o 2048 2048.cpp
	objdump -S 2048 > 2048.dump
	
static:
	g++ -std=c++1y -static $(addprefix -m,$(INSTS)) -O3 -pthread -Wall -fmessage-length=0 -o 2048 2048.cpp
	
profile:
	g++ -std=c++1y -mtune=native $(addprefix -m,$(INSTS)) -O3 -lpthread -Wall -fmessage-length=0 -fprofile-generate -o 2048 2048.cpp
	[ ! -e 2048.gcda ] || rm 2048.gcda && bash profile.sh
	g++ -std=c++1y -mtune=native $(addprefix -m,$(INSTS)) -O3 -lpthread -Wall -fmessage-length=0 -fprofile-use -o 2048 2048.cpp
	
4x6patt 5x6patt 6x6patt 7x6patt 8x6patt:
	[ -e $@.w ] || { [ -e $@.w.xz ] || curl -OJRf moporgic.info/data/2048/$@.w.xz; xz -vd $@.w.xz; }
	echo "./2048 -n $@ -i $@.w -a $(PGO_ALPHA) -t $(PGO_OPTI) -e $(PGO_EVAL) -% none -s" > profile.sh
	+make --no-print-directory profile && rm profile.sh
	
clean:
	rm 2048 || rm 2048.exe ||:
