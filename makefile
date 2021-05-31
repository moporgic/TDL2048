# TDL2048+ makefile

# C++ compiler options
STD ?= c++14
OLEVEL ?= 3
ARCH ?= tune=native
INSTS ?= abm bmi bmi2 avx avx2
FLAGS ?= -Wall -fmessage-length=0
OUTPUT ?= 2048
# other make settings
TARGET ?= default
SCRIPT ?= make-profile.sh
PGO_ALPHA ?= 0 fixed
PGO_OPTI ?= 1x10000
PGO_EVAL ?= 1x10000
PGO_FLAGS ?= -s

# fine-tune for specific architectures
CXXVER := $(shell gcc -dumpversion)
MACROS := $(shell echo | gcc -x c++ -march=native -dM -E -)
ifneq ($(findstring $(or $(BMI2), BMI2), $(MACROS)), BMI2)
	INSTS := $(filter-out abm bmi bmi2, popcnt $(INSTS) no-bmi2)
endif
ifneq ($(findstring $(or $(AVX2), AVX2), $(MACROS)), AVX2)
	INSTS := $(filter-out avx avx2, $(INSTS) no-avx2)
endif
ifneq ($(findstring x86_64, $(MACROS)), x86_64)
    INSTS :=
    FLAGS := -Wall -Wno-psabi -fmessage-length=0
endif

default: # build with default
	g++ -std=$(STD) -O$(OLEVEL) $(addprefix -m, $(ARCH) $(INSTS)) -pthread $(FLAGS) -o $(OUTPUT) 2048.cpp

static: # build with static executable
	@+make --no-print-directory default ARCH="tune=generic" FLAGS="$(FLAGS) -static"

native: # build with native architecture
	@+make --no-print-directory default ARCH="arch=native"

profile: # build with profiling by using a custom script
	@+make --no-print-directory $(TARGET) FLAGS="$(FLAGS)$(if $(filter -fprofile-update=%, $(FLAGS)),, -fprofile-update=single) -fprofile-generate"
	@sed "s|"\$$"(OUTPUT)|$(if $(findstring /, $(OUTPUT)),,./)$(OUTPUT)|g" $(SCRIPT) | tee $(SCRIPT).run
	@rm $(if $(and $(OUTPUT:2048=), $(filter 1, $(shell expr $(CXXVER) \>= 11))), $(OUTPUT)-)2048.gcda 2>/dev/null ||:
	@bash $(SCRIPT).run | xargs -d\\n -n1 echo \> && rm $(SCRIPT).run
	@+make --no-print-directory $(TARGET) FLAGS="$(filter-out -fprofile-update=%, $(FLAGS)) -fprofile-use"

4x6patt 5x6patt 6x6patt 7x6patt 8x6patt: # build with profiling by using predefined settings
	[ -e $@.w ] || { [ -e $@.w.xz ] || curl -OJRf moporgic.info/data/2048/$@.w.xz; xz -vd $@.w.xz; }
	@echo \$$"(OUTPUT) -n $@ -i $@.w -a $(PGO_ALPHA) -t $(PGO_OPTI) -e $(PGO_EVAL) -% none $(PGO_FLAGS)" > $(SCRIPT)
	@+make --no-print-directory profile FLAGS="$(FLAGS) -fprofile-update=single"

dump: # build and dump the disassembly
	@+make --no-print-directory $(word 1, $(TARGET)) TARGET="$(word 2, $(TARGET) default)" FLAGS="$(FLAGS) -g"
	objdump -S $(word 1, $(wildcard $(OUTPUT) $(OUTPUT).exe)) > $(OUTPUT).dump

clean: # cleanup
	rm $(wildcard $(OUTPUT) $(OUTPUT).exe) ||:
