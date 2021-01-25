STD ?= c++1y
OLEVEL ?= 3
ARCH ?= tune=native
INSTS ?= abm bmi bmi2 avx avx2
DEFINES := $(shell echo | gcc -x c++ -march=native -dM -E -)
ifneq ($(findstring __BMI2__, $(DEFINES)), __BMI2__)
	INSTS := $(filter-out abm bmi bmi2, popcnt $(INSTS))
endif
ifneq ($(findstring __AVX2__, $(DEFINES)), __AVX2__)
	INSTS := $(filter-out avx avx2, $(INSTS))
endif
FLAGS ?= -Wall -fmessage-length=0
OUTPUT ?= 2048
TARGET ?= default
PGO_ALPHA ?= 0 fixed
PGO_OPTI ?= 1x10000
PGO_EVAL ?= 1x10000

default:
	g++ -std=$(STD) -O$(OLEVEL) $(addprefix -m, $(ARCH) $(INSTS)) -lpthread $(FLAGS) -o $(OUTPUT) 2048.cpp

static:
	@+make --no-print-directory default ARCH="tune=generic" FLAGS="-pthread $(FLAGS) -static"

native:
	@+make --no-print-directory default ARCH="arch=native" INSTS=

profile:
	@+make --no-print-directory $(TARGET) FLAGS="$(filter-out -g, $(FLAGS)) -fprofile-generate"
	[ ! -e 2048.gcda ] || rm 2048.gcda && bash make-profile.sh | xargs -d\\n -n1 echo \>
	@+make --no-print-directory $(TARGET) FLAGS="$(FLAGS) -fprofile-use"

4x6patt 5x6patt 6x6patt 7x6patt 8x6patt:
	[ -e $@.w ] || { [ -e $@.w.xz ] || curl -OJRf moporgic.info/data/2048/$@.w.xz; xz -vd $@.w.xz; }
	echo "./$(OUTPUT) -n $@ -i $@.w -a $(PGO_ALPHA) -t $(PGO_OPTI) -e $(PGO_EVAL) -% none -s" > make-profile.sh
	@+make --no-print-directory profile

dump:
	@+make --no-print-directory $(word 1, $(TARGET)) TARGET="$(word 2, $(TARGET) default)" FLAGS="$(FLAGS) -g"
	objdump -S $(word 1, $(wildcard $(OUTPUT) $(OUTPUT).exe)) > $(OUTPUT).dump

clean:
	rm $(wildcard $(OUTPUT) $(OUTPUT).exe) ||:
