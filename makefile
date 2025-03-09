# TDL2048+ makefile

# C++ compiler options
STD ?= c++20
OLEVEL ?= 3
ARCH ?= tune=native
INSTS ?= abm bmi bmi2 avx avx2
FLAGS ?= -Wall -fmessage-length=0
SOURCE ?= 2048.cpp
OUTPUT ?= $(basename $(word 1, $(SOURCE)))
# other make options
TARGET ?= default
SCRIPT ?= make-profile.sh
PGO_ALPHA ?= 0 fixed
PGO_OPTI ?= 1x10000
PGO_EVAL ?= 1x10000
PGO_FLAGS ?= -s
# default command settings
MAKE ?= make
CXX ?= g++
RM ?= rm -f
OBJDUMP ?= objdump -S
MAKEFLAGS += --no-print-directory

# fine-tune for specific architectures
CXXVER := $(shell $(CXX) -dumpversion)
MACROS := $(shell echo | $(CXX) -x c++ -march=native -dM -E -)
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

# commit id for building
COMMIT_ID ?= $(shell git log -n1 --format=%h 2>/dev/null)$(if \
	$(shell git status -uno 2>&1 | grep -i changes)$(filter-out 2048.cpp, $(SOURCE)),+x)
ifneq ($(if $(filter -DCOMMIT_ID=%, $(FLAGS)),, $(COMMIT_ID)),)
	FLAGS += -DCOMMIT_ID=$(COMMIT_ID)
endif

default: # build with default
	$(CXX) -std=$(STD) -O$(OLEVEL) $(addprefix -m, $(ARCH) $(INSTS)) -pthread $(FLAGS) -o $(OUTPUT) $(SOURCE)

static: # build with static executable
	@+$(MAKE) default ARCH="tune=generic" FLAGS="$(FLAGS) -static"

native: # build with native architecture
	@+$(MAKE) default ARCH="arch=native"

profile: # build with profiling by using a custom script
	@+$(MAKE) $(TARGET) FLAGS="$(FLAGS)$(if $(filter -fprofile-update=%, $(FLAGS)),, -fprofile-update=single) -fprofile-generate"
	$(eval GCDA ?= $(if $(filter 1, $(shell expr $(CXXVER) \>= 11)), $(if $(filter $(basename $(SOURCE))$(SUFFIX), $(notdir $(OUTPUT))), \
						$(dir $(OUTPUT)), $(OUTPUT:%$(SUFFIX)=%)-))$(basename $(SOURCE)).gcda)
	$(if $(wildcard $(GCDA)), @$(RM) $(GCDA))
	@sed "s|"\$$"(OUTPUT)|$(dir $(OUTPUT))$(notdir $(OUTPUT))|g" $(SCRIPT) | BASH_XTRACEFD=1 bash -x \
		| xargs -d\\n -n1 echo \> | $(if $(findstring s, $(filter-out -%, $(MAKEFLAGS))), grep -v "^> + ", sed "s|^> + ||g")
	@+$(MAKE) $(TARGET) FLAGS="$(filter-out -fprofile-update=%, $(FLAGS)) -fprofile-use"

4x6patt 5x6patt 6x6patt 7x6patt 8x6patt: # build with profiling by using predefined settings
	$(if $(wildcard $@.w),, [ -e $@.w.xz ] || curl -OJRf moporgic.info/2048/model/$@.w.xz && xz -vd $@.w.xz)
	@echo \$$"(OUTPUT) -n $@ -i $@.w -a $(PGO_ALPHA) -t $(PGO_OPTI) -e $(PGO_EVAL) -% none $(PGO_FLAGS)" > $(SCRIPT)
	@+$(MAKE) profile FLAGS="$(FLAGS) -fprofile-update=single"

dump: # build and dump the disassembly
	@+$(MAKE) $(word 1, $(TARGET)) TARGET="$(word 2, $(TARGET) default)" FLAGS="$(FLAGS) -g"
	$(OBJDUMP) $(OUTPUT) > $(OUTPUT:%$(SUFFIX)=%).dump

clean: # cleanup
	$(eval FILES ?= $(shell file -0 $(wildcard $(OUTPUT:%$(SUFFIX)=%)* .) | grep -Ea ": +(ELF|PE32|GCC gcda)" | cut -d '' -f1))
	$(if $(FILES), $(RM) $(FILES))
