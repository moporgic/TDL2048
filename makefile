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
MAKEFLAGS += --no-print-directory

# fine-tune for specific architectures
CXXVER := $(shell gcc -dumpversion)
MACROS := $(shell echo | gcc -x c++ -march=native -dM -E -)
ifneq ($(filter WIN32 WIN64 WINNT, $(MACROS)),)
	override OUTPUT := $(OUTPUT)$(if $(filter %.EXE %.exe, $(OUTPUT)),,.exe)
	override SUFFIX := $(suffix $(OUTPUT))
endif
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
	@+make default ARCH="tune=generic" FLAGS="$(FLAGS) -static"

native: # build with native architecture
	@+make default ARCH="arch=native"

profile: # build with profiling by using a custom script
	@+make $(TARGET) FLAGS="$(FLAGS)$(if $(filter -fprofile-update=%, $(FLAGS)),, -fprofile-update=single) -fprofile-generate"
	$(eval GCDA ?= $(if $(filter 1, $(shell expr $(CXXVER) \>= 11)), $(if $(filter 2048$(SUFFIX), $(notdir $(OUTPUT))), \
						$(dir $(OUTPUT)), $(OUTPUT:%$(SUFFIX)=%)-))2048.gcda)
	$(if $(wildcard $(GCDA)), @rm -f $(GCDA))
	@sed "s|"\$$"(OUTPUT)|$(dir $(OUTPUT))$(notdir $(OUTPUT))|g" $(SCRIPT) | BASH_XTRACEFD=1 bash -x \
		| xargs -d\\n -n1 echo \> | $(if $(findstring s, $(filter-out -%, $(MAKEFLAGS))), grep -v "^> + ", sed "s|^> + ||g")
	@+make $(TARGET) FLAGS="$(filter-out -fprofile-update=%, $(FLAGS)) -fprofile-use"

4x6patt 5x6patt 6x6patt 7x6patt 8x6patt: # build with profiling by using predefined settings
	$(if $(wildcard $@.w),, [ -e $@.w.xz ] || curl -OJRf moporgic.info/data/2048/$@.w.xz && xz -vd $@.w.xz)
	@echo \$$"(OUTPUT) -n $@ -i $@.w -a $(PGO_ALPHA) -t $(PGO_OPTI) -e $(PGO_EVAL) -% none $(PGO_FLAGS)" > $(SCRIPT)
	@+make profile FLAGS="$(FLAGS) -fprofile-update=single"

dump: # build and dump the disassembly
	@+make $(word 1, $(TARGET)) TARGET="$(word 2, $(TARGET) default)" FLAGS="$(FLAGS) -g"
	objdump -S $(OUTPUT) > $(OUTPUT:%$(SUFFIX)=%).dump

clean: # cleanup
	$(eval FILES ?= $(shell file $(wildcard $(OUTPUT:%$(SUFFIX)=%)* .) | grep -E "executable|gcda" | grep -v script | cut -d: -f1))
	$(if $(FILES), rm -f $(FILES))
