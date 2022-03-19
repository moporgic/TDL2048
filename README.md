# moporgic/TDL2048+

[TDL2048+](https://github.com/moporgic/TDL2048) is a highly optimized temporal difference (TD) learning framework for [2048](https://gabrielecirulli.github.io/2048/).

### Features

Many common methods related to 2048 are supported, including
- Basic learning algorithms including TD(0), TD(λ), and n-step TD.
- Advanced algorithms including TC, multistage learning, and optimistic learning.
- N-tuple network with heterogeneous designs.
- Expectimax search with transposition table.

A highly effective bitboard is implemented, which features with
- Effective move generation and tile generation.
- Operations for isomorphisms, such as mirroring, rotating, or transposition.
- APIs for accessing many puzzle information.
- Support of both 65536-tile and 131072-tile.

### Performance

As of September 2021, TDL2048+ achieves the [state-of-the-art](https://doi.org/10.1109/TG.2021.3109887) in terms of average score and reaching rates of 32768-tile, by using optimistic TD+TC (OTD+TC) learning for training and tile-downgrading (DG) expectimax search for testing. The results of different search depths in the specified number of tested games are summarized as follows.

|Search      | Score  |8192 [%]|16384 [%]|32768 [%]|  # Games|
|:-----------|:------:|:------:|:-------:|:-------:|--------:|
|1-ply w/ DG | 412,785|  97.24%|   85.39%|   30.16%|1,000,000|
|2-ply w/ DG | 513,301|  99.17%|   94.40%|   48.92%|  100,000|
|3-ply w/ DG | 563,316|  99.63%|   96.88%|   57.90%|   10,000|
|4-ply w/ DG | 586,720|  99.60%|   98.60%|   62.00%|    1,000|
|5-ply w/ DG | 608,679|  99.80%|   97.80%|   67.40%|      100|
|6-ply w/ DG | 625,377|  99.80%|   98.80%|   72.00%|      100|

In addition, for sufficiently large tests, 65536-tiles are reached at a rate of 0.02%.

As of April 2021, TDL2048+ is the most efficient framework for 2048 in terms of speed.
The single-thread (ST) and multi-thread (MT) speed (moves/s) for the 4x6-tuple network are summarized as follows.

|Processor         |ST Training|ST  Testing|MT Training|MT  Testing|
|:-----------------|----------:|----------:|----------:|----------:|
|Ryzen 9 5950X     |  5,753,458|  6,554,100| 64,696,464|102,549,921|
|Xeon E5-2698 v4 x2|  2,943,012|  3,547,508| 56,530,890| 98,233,894|
|Core i9-7980XE    |  3,789,824|  4,307,902| 34,947,198| 67,576,154|
|Xeon E5-2698 v4   |  2,911,945|  3,551,124| 30,013,758| 49,707,145|
|Core i7-6950X     |  3,237,049|  3,860,228| 22,646,286| 35,324,773|
|Core i7-6900K     |  3,080,106|  3,679,456| 17,707,134| 27,486,805|
|Xeon E5-2696 v3   |  1,776,048|  1,983,475| 16,294,325| 21,588,230|
|Ryzen 9 3900X     |  2,674,171|  2,985,003| 13,354,437| 35,577,365|
|Core i9-9900K     |  3,947,512|  4,520,398| 11,822,327| 31,111,269|
|Core i7-7700K     |  2,996,736|  3,353,651|  7,171,175| 10,909,549|
|Celeron J4005     |    470,325|    493,897|    729,730|    889,492|
|Raspberry Pi 2B   |     54,098|     61,644|    157,756|    183,022|

## Prerequisites

This section briefly describes the prerequisites for executing and compiling TDL2048+.

### Platform

To maximize the efficiency of TDL2048+, the recommendations are as follows.
- Modern CPU with efficient [BMI2](https://en.wikipedia.org/wiki/Bit_manipulation_instruction_set#BMI2) and [AVX2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2) support
- [Quad memory channels](https://en.wikipedia.org/wiki/Multi-channel_memory_architecture#Quad-channel_architecture) with all channels are filled
- 64-bit operating system

TDL2048+ should support almost all common platforms with properly installed build tools.
However, it will be less efficient if the target platform does not meet the recommendations.

### Build Tools

[GCC](https://gcc.gnu.org/) and [GNU Make](https://www.gnu.org/software/make/) are the only two required tools for building TDL2048+.
- GCC 5.0 is the minimum requirement, while a more recent version is recommended.
- GNU Make should also be installed to use the makefile.

Both GCC and Make are common tools and may have already been installed.
```bash
gcc --version # check the installed GCC
make --version # check the installed Make
```

If the build tools are not properly installed, follow the steps below to install them manually.

<details><summary>For Arch Linux</summary>

Install the [```base-devel```](https://archlinux.org/groups/x86_64/base-devel/) package.
```bash
sudo pacman -Sy --needed base-devel
```
</details>

<details><summary>For Ubuntu or Debian</summary>

Install the [```build-essential```](https://packages.ubuntu.com/xenial/build-essential) package.
```bash
sudo apt update
sudo apt install build-essential
```
</details>

<details><summary>For Windows</summary>

Install the [MinGW-w64](https://sourceforge.net/projects/mingw-w64/) and follow [these steps](https://www.eclipse.org/4diac/documentation/html/installation/minGW.html) to set up the installation.
</details>

### Other Tools

Besides build tools, some provided utilities are written in [Bash script](https://www.gnu.org/software/bash/) with Linux CLI tools.

<details><summary>For Arch Linux</summary>

```bash
sudo pacman -Sy --needed curl xz # other tools should already be installed
```
</details>

<details><summary>For Ubuntu or Debian</summary>

```bash
sudo apt update
sudo apt install curl xz-utils # other tools should already be installed
```
</details>

<details><summary>For Windows</summary>

Install [Cygwin](https://www.cygwin.com/) for a Bash environment, make sure that [curl](https://cygwin.com/packages/summary/curl.html) and [xz](https://cygwin.com/packages/summary/xz.html) are installed.
</details>

### Tested Platforms

<details><summary>Show tested platforms</summary>

|Processor      |Operating System   |GCC     |
|---------------|-------------------|--------|
|Xeon E5-2698 v4|Arch Linux 5.10 LTS|GCC 10.2|
|Xeon E5-2683 v3|Ubuntu 20.04.2 LTS |GCC 9.3 |
|Core i9-7980XE |Arch Linux 5.10    |GCC 10.2|
|Core i9-9900K  |Arch Linux 5.11    |GCC 10.2|
|Core i7-6900K  |Windows 10 Pro 1803|GCC 7.2 |
|Core i7-10510U |Windows 10 Pro 20H2|GCC 7.3 |
|Celeron J4005  |Arch Linux 5.4 LTS |GCC 10.1|
|Ryzen 9 3900X  |Arch Linux 5.7     |GCC 10.1|
|Ryzen 9 5950X  |Arch Linux 5.16    |GCC 11.2|
|Raspberry Pi 2B|Arch Linux 5.10    |GCC 10.2|

</details>

## Compilation

The provided makefile is designed for the most common use, which is covered in this subsection.

### Building

By default, TDL2048+ is built with ```-O3 -mtune=native```.
```bash
make # build TDL2048+ with default settings
```
After a successful build, a binary file ```./2048``` should be generated.

<details><summary>Show advanced options</summary><br>

To use another output file name instead of ```./2048```, set the ```OUTPUT``` as follows.
```bash
make OUTPUT="run" # the output binary will be ./run instead of ./2048
```

Also, to build with another source file instead of ```2048.cpp```, set the ```SOURCE``` as follows.
```bash
make SOURCE="2048-test.cpp" # compile 2048-test.cpp and generate ./2048-test
```

If another optimization level is needed, set the ```OLEVEL``` as follows.
```bash
make OLEVEL="g" # build with -Og -mtune=native
```

For deployment, a statically linked binary can be built with
```bash
make static # build with -O3 -mtune=generic -static
```

Building with ```-march=native``` is not recommended since it is observed to be less efficient; however, make with target ```native``` if you insist.
```bash
make native # build with -O3 -march=native
```

For debugging, a target ```dump``` enables ```-g``` flag, and invokes [```objdump```](https://en.wikipedia.org/wiki/Objdump) to dump the assembly code.
```bash
make dump # build with -O3 -mtune=native -g, then dump the binary with objdump -S
```
After building with ```dump```, the assembly code should be saved as a ```.dump``` text file.
</details>

### Profiling

GCC built-in [profile-guided optimization (PGO)](https://en.wikipedia.org/wiki/Profile-guided_optimization) (```-fprofile-use```) is also included in the makefile.
Building with PGO may significantly improve the program speed of certain scenarios, at the expense of overall speed. However, it is still worth especially if long-term training is required.

To profile the 4x6-tuple network, prepare a pre-trained network  ```4x6patt.w``` and build with
```bash
make 4x6patt # make and profile with default settings on the 4x6-tuple (need some time)
```

Five pre-defined targets support profiling: ```4x6patt```, ```5x6patt```, ```6x6patt```, ```7x6patt```, and ```8x6patt```.
Note that if the pre-trained network is not found, it will be automatically downloaded.
<details><summary>Show download links</summary>

```4x6patt```: https://moporgic.info/data/2048/4x6patt.w.xz
```5x6patt```: https://moporgic.info/data/2048/5x6patt.w.xz
```6x6patt```: https://moporgic.info/data/2048/6x6patt.w.xz
```7x6patt```: https://moporgic.info/data/2048/7x6patt.w.xz
```8x6patt```: https://moporgic.info/data/2048/8x6patt.w.xz
</details>


<details><summary>Show advanced options</summary><br>

The pre-defined profiling recipes are designed to balance training and testing performance.

To maximize the training speed (however, with poor testing speed), set the ```PGO_EVAL``` as follows.
```bash
make 4x6patt PGO_EVAL="0" # profile with only training routine
```

Similarly, to maximize the testing speed, set the ```PGO_OPTI``` as follows.
```bash
make 4x6patt PGO_OPTI="0" # profile with only testing routine
```

The above profiling examples are all with TD. To profile with TC, set the ```PGO_ALPHA``` as follows.
```bash
make 4x6patt PGO_ALPHA="0 coherence" # profile with TC learning
```

To profile the expectimax search, disable the training with ```PGO_OPTI``` and set the depth with ```PGO_FLAGS``` as follows.
```bash
make 4x6patt PGO_OPTI="0" PGO_FLAGS="-d 2p" # profile with 2-ply search
```

#### Profiling with Custom Recipes

If the pre-defined profiling recipes do not meet your requirements, a script ```make-profile.sh``` can be used for custom profiling. First, define your profiling recipe in ```make-profile.sh``` like
```bash
./2048 -n 4x6patt -i 4x6patt.w -a 0 fixed -t 1x10000 -e 1x10000 -% none -s
```

Then, make with target ```profile``` as follows.
```bash
make profile # profile with make-profile.sh
```

To profile with multithreading (```-p```), set ```-fprofile-update=atomic``` explicitly as follows.
```bash
make profile FLAGS="-fprofile-update=atomic" # profile with multithreading
```

Note that pre-defined profiling recipes (e.g., ```4x6patt```) will overwrite the ```make-profile.sh```.
</details>

### Miscellaneous

#### Use Custom Optimizations

By default, the makefile should automatically toggle platform-specific optimizations.
However, the default settings can be overridden by using following variables.

<details><summary>Show advanced options</summary>

```bash
make STD="c++14" # build with -std=c++14
make ARCH="tune=native" # build with -mtune=native
make INSTS="abm bmi bmi2 avx avx2" # build with -mabm -mbmi -mbmi2 -mavx -mavx2
make FLAGS="-Wall -fmessage-length=0" # build with specified flags
```
</details><br>

For simplicity, BMI2 and AVX2 can be disabled with ```BMI2=no``` and ```AVX2=no``` respectively.

On some machines, such as the AMD Ryzen 3000 series, you may want to explicitly disable BMI2 because of their [slow implementation](http://www.talkchess.com/forum3/viewtopic.php?f=7&t=72538). This can be done as follows.
```bash
make BMI2="no" # build with default settings but disable the BMI2 optimization
```

#### Specify Default Target

Note that target ```default``` is used when making ```dump```, ```profile```, ```4x6patt```, ..., and ```8x6patt```.
To specify another target, such as ```static```, set ```TARGET="static"``` as follows.

<details><summary>Show advanced options</summary>

```bash
make dump TARGET="static" # dump a static binary
make 4x6patt TARGET="static" # make a static profiled 4x6patt binary
make dump TARGET="4x6patt static" # make a static profiled 4x6patt binary and dump it
```
</details>


## Usage

This subsection briefly introduces the CLI options, log format, and the provided utilities.

### CLI Options

Below is an example of training, in which a new network is initialized as the pre-defined ```4x6patt``` structure, then it is trained with 1000k episodes, and finally the result is saved as ```4x6patt.w```.
```bash
./2048 -n 4x6patt -t 1000 -o 4x6patt.w
```

Similarly, below is an example of testing, in which a pre-trained network ```4x6patt.w``` is loaded and tested with 1000k episodes, and the log is written to ```4x6patt.x```.
```bash
./2048 -n 4x6patt -e 1000 -i 4x6patt.w -o 4x6patt.x
```

More CLI options will be introduced in the following subsection.

#### N-Tuple Network

Use ```-n``` to specify the target network structure as follows. TDL2048+ supports any pattern-based structures with no more than 8-tuple, in which  common pattern-based designs are per-defined for simplicity, such as ```4x6patt```, ```5x6patt```, ```6x6patt```, ```7x6patt```, ```8x6patt```, and so on.
```bash
./2048 -n 8x6patt -t 1000 # use the 8x6patt network
```

<details><summary>Show advanced options</summary><br>

Pattern-based structures are defined with cell locations, by using hexadecimal characters from ```0```, ```1```, ..., ```f``` to indicate the upper-left cell to the lower-right cell, respectively. Also, isomorphisms including rotations and reflections are enabled by default, e.g., a declared pattern ```012345``` actually involves ```012345 37bf26 fedcba c840d9 cdef89 fb73ea 321076 048c15```.

For simplicity, aliases (e.g., ```4x6patt```) are defined for commonly used structures. The definitions of commonly used built-in aliases are listed below. Check the source for a detailed list.
```
4x6patt = 012345 456789 012456 45689a
5x6patt = 012345 456789 89abcd 012456 45689a
6x6patt = 012456 456789 012345 234569 01259a 345678
7x6patt = 012456 456789 012345 234569 01259a 345678 134567
8x6patt = 012456 456789 012345 234569 01259a 345678 134567 01489a
2x7patt = 0123456 456789a
3x7patt = 0123456 456789a 89abcde
1x8patt = 01234567
2x8patt = 01234567 456789ab
4x5patt = 01234 45678 01245 45689
2x4patt = 0123 4567
5x4patt = 0123 4567 0145 1256 569a
```

To define a structure with custom patterns, specify the patterns in the ```-n``` command as follows. Note that custom patterns may be less efficient than built-in patterns.
```bash
./2048 -n 012345 456789 012456 45689a -t 1000
```
</details><br>

In addition, some statistic-based designs are also supported, such as the monotonicity (```mono```), and the number of tiles of each type (```num```). You may use them with pattern-based designs.
```bash
./2048 -n 4x6patt mono num -t 1000 # use 4x6patt, mono, and number of tiles
```

<details><summary>Show advanced options</summary><br>

Statistic-based structures improve the capacity of a network, but with slower program speed.

```
mono      = mono/0123 + mono/4567
mono/0123 = the monotonicity of the outer line
mono/4567 = the monotonicity of the inner line
num       = num@lt + num@st
num@lt    = the number of large tiles
num@st    = the number of small tiles
```
</details>

#### Learning Rate and TC

The learning rate is 0.1 by default, but you may want to manually modulate the value especially when the network is saturated. This can be done by using ```-a``` as follows.
```bash
./2048 -n 4x6patt -t 1000 -a 0.01 # use 0.01 as the base learning rate
```

Note that setting ```-a 1.0``` enables TC, in which the learning rate is automatically modulated.

<details><summary>Show advanced options</summary><br>

To explicitly specify TD or TC, use option ```fixed``` or ```coherence``` together with ```-a``` as follows.
```bash
./2048 -n 4x6patt -t 1000 -a 1.0 fixed # force using TD with alpha=1.0
./2048 -n 4x6patt -t 1000 -a 0.1 coherence # force using TC with alpha=0.1
```

The learning rate is distributed to each n-tuple feature weight. For example, the ```4x6patt``` network has 32 feature weights, so a weight is actually adjusted with a rate of 0.01 when ```-a 0.32``` is set.

However, you may use ```norm``` together with ```-a``` to override the default behavior as
```bash
./2048 -n 4x6patt -t 1000 -a 0.0025 norm=1 # adjust each weight with a rate of 0.0025/1
```
</details>

#### TD(λ) and N-Step TD

The default training method is 1-step TD(0). Follow instructions below to enable other methods.

To enable n-step TD training, use ```-N``` to specify a step size.
```bash
./2048 -n 4x6patt -t 1000 -N 10 # use 10-step TD training
```

To enable TD(λ) training, use ```-l``` to specify a lambda value.
```bash
./2048 -n 4x6patt -t 1000 -l 0.5 # use TD(0.5) training
```

#### Multistage TD

Multistage TD is a kind of hierarchical TD learning that divides the entire episode into multiple stages, in which each stage has an independent value function. This technique significantly improves the performance at the cost of additional storage for stages.

To enable multistage TD(0), use ```-@``` to specify the stage thresholds.
```bash
./2048 -n 4x6patt@2 -@ 0,16384 -t 1000 -o 4x6patt@2.w # use 2-stage TD(0) training
./2048 -n 4x6patt@2 -@ 0,16384 -e 1000 -i 4x6patt@2.w # testing the above network
./2048 -n 4x6patt@4 -@ 0,16384,32768,49152 -t 1000 # 4 stages, 49152 = 32768+16384
```

<details><summary>Show advanced options</summary><br>

If the base network has an alias, its multistage form can be declared with ```@N```, where ```N``` is the number of stages. However, if the base network does not have an alias, use ```|``` operator to separate network stages. For example, if the base network is ```"01234 45678 01245 45689"```, its 2-stage form can be declared as follows.
```bash
# the 2-stage form of "01234 45678 01245 45689"
./2048 -n "01234|00000000 45678|00000000 01245|00000000 45689|00000000"
          "01234|10000000 45678|10000000 01245|10000000 45689|10000000"
```

Note that due to a current limitations, each stage must have the same structure.
</details>

#### Carousel Shaping and Restart

Carousel shaping is a technique that prevents the network from overfiting at the beginning of the episodes, by starting with non-initial states.

To enable TD(0) training with carousel shaping, use ```-b``` to specify the block size. Note that differ from the originally proposed method, the implementation here also applies to a single stage.
```bash
./2048 -n 4x6patt -b 2048 -t 1000 # block size is 2048
./2048 -n 4x6patt@2 -@ 0,32768 -b 16384 -t 1000 # also works together with multistage
```

Similar to the carousel shaping, restart is a technique that uses the states recorded in the previous episode to restart the game, to focus on the later game stages.

To enable TD(0) training with restart, set the training mode as ```restart```.
```bash
./2048 -n 4x6patt -t 1000 -tt restart # use restart strategy
```

<details><summary>Show advanced options</summary><br>

When using carousel shaping ```-b```, each episode will run until a 65536-tile is reached by default. More specifically, when an episode ends at 16384-tile+8192-tile, a new episode with 16384-tile+8192-tile+2048-tile will start. This process continues until it reaches the upper bound of shaping, i.e., 65536-tile by default.

To change the upper bound of shaping, specify the tiles as follows.
```bash
./2048 -n 4x6patt -b 2048 49152 -t 1000 # block size 2048, shape up to 32768+16384
```

When using restart, options ```L``` and ```at``` can be specified to control its behavior, as shown below.
```bash
# restart when episode length >= 10, at 50% of the recorded states (default)
./2048 -t 1000 mode=restart L=10 at=0.5
# restart when episode length >= 100, at 30% of the recorded states
./2048 -t 1000 mode=restart L=100 at=0.3
```
</details>

#### Optimistic Initialization

To enable optimistic initialization, set the initial value when declaring networks.
```bash
./2048 -n 4x6patt=320000/norm -t 1000 # initial V is 320000
./2048 -n 8x6patt=5000 -t 1000 # initial V is 320000 (weight is initialized to 5000)
```

The above examples are optimistic TD (OTD) learning. Note that OI can be enabled together with any training mode, e.g., specify both ```-a 1.0``` and OI to use optimistic TC (OTC) learning.

To use optimistic TD+TC (OTD+TC) learning, train a network with OTD then follow by TC as follows.
```bash
./2048 -n 4x6patt=320000/norm -t 9000 -a 0.1 -o 4x6patt.w # traing with 90% OTD
./2048 -n 4x6patt -t 1000 -e 100 -a 1.0 -io 4x6patt.w # train with 10% OTC and test
```

<details><summary>Show advanced options</summary><br>

Note that when using ```/norm```, the value will be evenly distributed to weights. For example, a ```4x6patt``` has 32 features, setting ```320000/norm``` will actully initialize each weights as ```10000```.

Due to a framework limitation, the ```/norm``` will use the total number of features of all stages when combining OI with multistage network. To prevent network from being unexpectedly initialized, please declare the weight value explicitly for multistage networks, e.g., ```4x6patt@2=10000```.
</details>

#### Ensemble Learning

By using ensemble learning, multiple networks can be averaged to improve the final performance.
To enable ensemble learning, specify multiple weight files as input as follows.
```bash
./2048 -n 4x6patt -e 100 -i 4x6-0.w 4x6-1.w # evaluate the ensemble of 2 x 4x6patt
```
Check the loading options below for more details about using ensemble learning.

#### Forward and Backward Training

Training modes (TD, n-step TD, TD(λ), MSTD, and restart) are implemented with both forward and backward variants. These methods use forward variants by default, except of TD(λ).

Compared with forward training, backward training for TD and n-step is slightly inefficient in terms of speed, but it achieves a higher average score with the same learned episodes.

Option ```-tt``` specifies an advanced training mode as follows.
```bash
./2048 -n 4x6patt -t 1000 -tt backward # backward TD(0)
./2048 -n 4x6patt -t 1000 -tt step-backward -N 5 # backward 5-step TD(0)
```

You may enable the forward TD(λ) with ```-tt lambda-forward``` with a step size with ```-N``` as follows.
```bash
./2048 -n 4x6patt -t 1000 -tt lambda-forward -l 0.5 -N 5 # forward 5-step TD(0.5)
```

#### Expectimax Search

An additional search usually improves the program strength, which can be enabled with ```-d``` flag.
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 3p # enable a 3-ply search
```

Note that the default search depth is 1-ply, i.e., no additional search.

<details><summary>Show advanced options</summary><br>

You may want to log each episode when running a deep search, which can be done by
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10000x1 -d 3p # 10000 episodes
```
Note that the number of tested episodes is the same as ```-e 10```, but with a more detailed log.

In order to prevent the search tree from becoming too large, it is best to limit the search depth when there are many empty cells on the puzzle. This can be set by using ```limit``` with ```-d``` as follows.
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 5p limit=5p,5p,5p,5p,4p,4p,4p,4p,3p
```
In the above example, the search starts with 5-ply at root, in which the depth is limited to 5-ply if there are 0 to 3 empty cells; 4-ply if there are 4 to 7 empty cells; and 3-ply if there are 8 or more empty cells. Note that ```limit=``` accepts at most 16 values, corresponding to 0 to 15 empty cells.
</details><br>

To speed up the search, a transposition table (TT) can be enabled with ```-c``` as
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 3p -c 8G # 3-ply search with 8G TT
```
Note that the size of TT must be the power of 2, e.g., 4G, 8G, 16G, 64G, etc.

<details><summary>Show advanced options</summary><br>

Be sure to decide the size of TT carefully. The use of TT involves a lot of memory access, which may result in worse search speed especially when the search depth is less than 3-ply.

In addition, it is possible to allow the search to use a deeper TT cache when available. This may improve the strength especially for deep search. Use ```peek``` together with ```-c``` to enable this.
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 5p -c 64G peek # peeking the deeper TT cache
```

More specifically, if the search requires the 3-ply result of a puzzle, while TT only caches the 5-ply result, setting ```peek``` allows the search to directly obtain the 5-ply result for current use.
</details>

#### Tile-Downgrading

Tile-downgrading is a technique that searches states by translating them into downgraded states, which can improve the performance when reaching large tiles.

To enable testing with tile-downgrading, use ```-h``` the declare the threshold.
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -h 32768 # activate once 32768-tile is reached
./2048 -n 4x6patt -i 4x6patt.w -e 10 -h 49152 # threshold is 32768-tile + 16384-tile
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 2p -h 32768 # together with expectimax search
```

Note that this is necessary for 65536-tiles, since n-tuple networks do not recognize them.

#### Saving/Loading and Logging

The program uses ```-i``` for loading, ```-o``` for saving and logging.
For simplicity, ```-io``` toggles both ```-i``` and ```-o```, which is useful during long-term training.

Three types of files are currently support: n-tuple network (```.w```), TT cache (```.c```), and log file (```.x```). The file must be named with the correct extension.

Note that no error message will appear when an IO failure occurs.

<details><summary>Show some examples</summary><br>

To save the trained network as ```4x6patt.w``` and log to ```4x6patt-training.x```:
```bash
./2048 -n 4x6patt -t 1000 -o 4x6patt.w 4x6patt-training.x
```

To load from a trained network and log the results of expectimax search:
```bash
./2048 -n 4x6patt -e 1000 -d 3p -i 4x6patt.w -o 4x6patt-testing.x
```

To save snapshots regularly during a long-term training:
```bash
for i in {01..10}; do
    ./2048 -n 4x6patt -t 1000 -e 10 -io 4x6patt.w -o 4x6patt.x
    tar Jcvf 4x6patt.$i.tar.xz 4x6patt.w 4x6patt.x
done
```
</details>

<details><summary>Show advanced options</summary><br>

To select specific weights of a file when loading or saving, use ```[IDX]|``` prefix with file path as
```bash
./2048 -n 4x6patt -t 1000 -o '[0]|4x6patt-part.w' # save the first n-tuple
./2048 -n 8x6patt -t 1000 -o '[0-3]|8x6patt-part.w' # save the first 4 n-tuples
./2048 -n 4x6patt -e 10 -i '[0,1]|4x6patt-1.w' '[2,3]|4x6patt-2.w' -o 4x6patt.w
# load from two different weights, test and save the merged result
```

TDL2048+ supports ensemble learning by averaging n-tuple weights. To perform this, specify multiple weight files as input, weights with the same signature are automatically averaged.

In the following example of ensemble learning, two networks (```4x6patt-0.w``` and ```4x6patt-1.w```) are averaged, their ensemble result is then evaluated and stored as ```4x6patt.w```.
```bash
./2048 -n 4x6patt -e 1000 -i 4x6patt-0.w 4x6patt-1.w -o 4x6patt.x 4x6patt.w
```

</details>

#### Random Seed

The pseudo-random number generator is randomly initialized by default.
To specify a fixed seed, use ```-s``` with a custom string as follows.
```bash
./2048 -n 4x6patt -t 1000 -s Hello # use "Hello" to seed the PRNG
```

#### Parallel Execution

The program executes with only a single thread by default.
To enable parallel execution, use ```-p``` with the number of threads as follows.
```bash
./2048 -n 4x6patt -t 1000 -p 10 # use 10 threads for execution
```
If the number of threads is not provided, the program will use all available threads.

<details><summary>Show advanced options</summary><br>

Make sure that the number in ```-t``` or ```-e``` is larger than the parallelism.
In the following example, only thread #1 to thread #10 will work, while other threads will be idle.
```bash
./2048 -n 4x6patt -i 4x6patt.w -d 5p -c 64G -e 10 -p 20
```

To prevent this from happening, set the unit instead of using the default (1000) as follows.
```bash
./2048 -n 4x6patt -i 4x6patt.w -d 5p -c 64G -e 20x500 -p 20
```

In addition, the program will automatically toggle the use of [shared memory (SHM)](https://en.wikipedia.org/wiki/Shared_memory) on Linux platforms, since using ```fork``` with SHM performs better than using ```std::thread``` in speed.

To explicitly disable the SHM, add ```noshm``` with ```-p``` to tell the program to use ```std::thread```.
```bash
./2048 -n 4x6patt -i 4x6patt.w -d 5p -c 64G -e 20x500 -p 20 noshm
```

Note that on Linux platforms with SHM enabled, issuing both training and testing (```-t``` and ```-e```) in a single command with parallelism may lead to a slightly worse testing speed.
```bash
# issue training and testing in a single command
./2048 -n 4x6patt -t 1000 -e 1000 -p 10 -o 4x6patt.w # training, then testing
# issue training and testing in two different commands
./2048 -n 4x6patt -t 1000 -p 10 -o 4x6patt.w # training,
./2048 -n 4x6patt -e 1000 -p 10 -i 4x6patt.w # then testing
```

Due to a current limitation, the speed of testing in the former may be slightly slower than that in the latter. However, should still be faster than using ```std::thread```.

Finally, TDL2048+ has not been optimized to support [multiprocessing](https://en.wikipedia.org/wiki/Multiprocessing) with [non-uniform memory access (NUMA)](https://en.wikipedia.org/wiki/Non-uniform_memory_access) (i.e., multiple CPUs), [multi-die](https://www.hardwaretimes.com/amd-ccd-and-ccx-in-ryzen-processors-explained) (e.g., an AMD Ryzen 9 5950X processor has two CCDs), and similar [multi-chip](https://en.wikipedia.org/wiki/Multi-chip_module) architectures.

On such platforms, parallel execution may result in a significant loss of training speed. Therefore, it is recommended to use [```taskset```](https://man7.org/linux/man-pages/man1/taskset.1.html) to limit the execution on only a single processor (core die) for parallel training.

For examples, on a machine with two Intel E5-2698 v4, and on a machine with an AMD Ryzen 9 5950X, the parallel training speeds without and with ```taskset``` are as follows.
```bash
# Intel E5-2698 v4 x2 (20C/40T x2, 2 CPUs)
./2048 -n 4x6patt -t 400 -p 80 # execute across CPUs, 14M ops (slow!)
taskset -c 0-19,40-59 ./2048 -n 4x6patt -t 400 -p 40 # execute on CPU0 only, 28M ops
{ # run two recipes simultaneously to maximize the utilization
    taskset -c 0-19,40-59  ./2048 -n 4x6patt -t 400 -p 40 & # on CPU0 only
    taskset -c 20-39,60-79 ./2048 -n 4x6patt -t 400 -p 40 & # on CPU1 only
    wait
} # execute one recipe per CPU, 56M ops in total

# AMD Ryzen 9 5950X (16C/32T, 2 CCDs)
./2048 -n 4x6patt -t 160 -p 32 # execute across CCDs, 22M ops (slow!)
taskset -c 0-7,16-23 ./2048 -n 4x6patt -t 160 -p 16 # execute on CCD0 only, 34M ops
{ # run two recipes simultaneously to maximize the utilization
    taskset -c 0-7,16-23  ./2048 -n 4x6patt -t 160 -p 16 & # on CCD0 only
    taskset -c 8-15,24-31 ./2048 -n 4x6patt -t 160 -p 16 & # on CCD1 only
    wait
} # execute one recipe per CCD, 64M ops in total
```
</details>

#### Miscellaneous

<details><summary>Summary</summary>

By default, the summary is only printed for testing (```-e```).
Use ```-%``` to display it also for training; use ```-% none``` to hide it for both training and testing.
</details>

<details><summary>Winning Tile</summary>

To change the winning tile of statistics, specify ```-w``` with a tile value, e.g., ```-w 32768```.
</details>

<details><summary>Help Message</summary>

Use ```-?``` to print a brief CLI usage.
Use ```-v``` to print the program version.
</details>

### Log Format

The log of executing ```./2048 -n 4x6patt -t 10 -e 10``` is used as an example.

A log may contain version info, CLI arguments, parameters, statistic blocks, and summary blocks.

<details><summary>Show log structure</summary><br>

At the beginning, there is build info including build revision, compiler version, and build time.
```
TDL2048+ by Hung Guei
Develop Rev.6063c49 (GCC 11.2.0 C++201402 @ 2022-03-19 15:09:53)
```

The next several lines print the program arguments and some important parameters.
```
./2048 -n 4x6patt -t 10 -e 10
time = 2022-03-19 15:11:11.867
seed = aaa16f54
alpha = 0.1
lambda = 0, step = 1
stage = {0}, block = 65536
search = 1p, cache = none
thread = 1x
```

The current n-tuple network structure appears next.
```
012345[16M] : 012345 37bf26 fedcba c840d9 cdef89 fb73ea 321076 048c15
456789[16M] : 456789 26ae15 ba9876 d951ea 89ab45 ea62d9 7654ba 159d26
012456[16M] : 012456 37b26a fedba9 c84d95 cde89a fb7ea6 321765 048159
45689a[16M] : 45689a 26a159 ba9765 d95ea6 89a456 ea6d95 765ba9 15926a
```

Then, the training part comes with a ```optimize``` label, in which the details will be introduced later.
```
optimize: 10

001/010 89ms 5445820.22ops
local:  avg=7116 max=25384 tile=2048 win=0.30%
total:  avg=7116 max=25384 tile=2048 win=0.30%

002/010 111ms 6075603.60ops
local:  avg=10684 max=28456 tile=2048 win=1.60%
total:  avg=8900 max=28456 tile=2048 win=0.95%

... (some lines are skipped here) ...

009/010 191ms 6037989.53ops
local:  avg=20583 max=59268 tile=4096 win=33.10%
total:  avg=14832 max=61324 tile=4096 win=14.21%

010/010 200ms 6040250.00ops
local:  avg=21814 max=62996 tile=4096 win=39.60%
total:  avg=15530 max=62996 tile=4096 win=16.75%
```

Finally, the testing part comes with a ```evaluate``` label and ends with a summary block.
```
evaluate: 10 info

001/010 177ms 6903401.13ops
local:  avg=22022 max=72076 tile=4096 win=37.00%
total:  avg=22022 max=72076 tile=4096 win=37.00%

002/010 175ms 6940114.29ops
local:  avg=21870 max=72104 tile=4096 win=37.80%
total:  avg=21946 max=72104 tile=4096 win=37.40%

... (some lines are skipped here) ...

009/010 175ms 6898908.57ops
local:  avg=21765 max=63224 tile=4096 win=37.70%
total:  avg=21915 max=72104 tile=4096 win=38.13%

010/010 178ms 6919629.21ops
local:  avg=22253 max=68452 tile=4096 win=39.20%
total:  avg=21948 max=72104 tile=4096 win=38.24%

summary 1764ms 6902075.96ops
total:  avg=21948 max=72104 tile=4096 win=38.24%
tile     count   score    move     rate      win
128          6    1677     166    0.06%  100.00%
256         93    3952     319    0.93%   99.94%
512       1299    8712     595   12.99%   99.01%
1024      4778   17532    1038   47.78%   86.02%
2048      3650   31352    1635   36.50%   38.24%
4096       174   55087    2539    1.74%    1.74%
```
</details>

#### Statistic Block

A statistic block is the result of 1000 episodes by default, with three lines as follows.
```
002/010 175ms 6940114.29ops
local:  avg=21870 max=72104 tile=4096 win=37.80%
total:  avg=21946 max=72104 tile=4096 win=37.40%
```

The first line shows the index number (```002/010```, current/total), the time used for this block (```175ms```), and the measured speed in moves/s (```6940114.29ops```).
When using parallel execution, there is an additional ```[#]``` to display the executor id of this block.

The second line, ```local```, shows the statistic of this block, including the average score (```avg=21870```), the maximum score (```max=72104```), the maximum tile (```tile=4096```), and the win rate (```win=37.80%```).

The third line, ```total```, has the same format as the second line, but shows the statistics starting from the first index (```001/xxx```) of this session. Note that ```-t 10 -e 10``` are two separate sessions.

#### Summary Block

A summary block only be attached at the end of a testing session (```-e```) by default.
```
summary 1764ms 6902075.96ops
total:  avg=21948 max=72104 tile=4096 win=38.24%
tile     count   score    move     rate      win
128          6    1677     166    0.06%  100.00%
256         93    3952     319    0.93%   99.94%
512       1299    8712     595   12.99%   99.01%
1024      4778   17532    1038   47.78%   86.02%
2048      3650   31352    1635   36.50%   38.24%
4096       174   55087    2539    1.74%    1.74%
```

The first line shows the total time usage (```1764ms```) and the measured speed (```6902075.96ops```) of this session. The second line is the same as the ```total``` of the last statistic block.

The tile-specific statistic is printed starting from the third line, in which ```count``` shows how many episodes end with only ```tile```; ```score``` and ```move``` show the average score and average moves of such episodes; ```rate``` shows the percentage of such episodes; and ```win``` shows the reaching rate.

Take ```2048-tile``` as an example, there are ```3650``` games end with it, their average score and average moves are ```31352``` and ```1635```, respectively. Since this session has 10k games, the percentage of games ending with it is ```36.50%```; and the reaching rate is ```38.24%```, calculated from ```100% - (0.06% + 0.93% + 12.99% + 47.78%)```.

### Utilities

Some utilities written in Bash script are provided for benching the program or parsing the log.

<details><summary>2048-bench.sh - benchmark related functions</summary>

A full benchmark routine is provided in this script, for example:
```bash
./2048-bench.sh -D # bench default build on 4x6patt and 8x6patt
./2048-bench.sh -P # bench profiled build on 4x6patt and 8x6patt
```

In addition, you may use ```source 2048-bench.sh``` to initialize some functions.
The following are examples of provided functions, check the script for more details.
```bash
test ./2048 # test the training/testing speed
test-st ./2048 # test the single-thread training/testing speed
test-mt ./2048 # test the multi-thread training/testing speed
test-e-st ./2048 # test the single-thread testing speed
test-e-mt ./2048 # test the multi-thread testing speed
bench ./2048 # benchmark
compare ./2048-1 ./2048-2 # compare the speed of two executables
benchmark # perform a full benchmark
```
</details>

<details><summary>2048-parse.sh - log parser for statistic block</summary>

Extract AVG, MAX, TILE, and WIN from a log, which is useful for exporting statistics.
```bash
grep local 2048.x | ./2048-parse.sh # extract AVG, MAX, TILE, and WIN from each block
```
</details>

<details><summary>2048-winrate.sh - log parser for summary block</summary>

Extract win rates of large tiles from summary block. Check the script for more details.
```bash
./2048-winrate.sh < 2048.x # extract win rates from summary block and print them
```
</details>

<details><summary>2048-thick.sh - log concentrator for statistic block</summary>

Rearrange the statistics. To join every 10 statistic blocks and print the new statistics, do
```bash
grep local 2048.x | ./2048-parse.sh | ./2048-thick.sh 10 # average every 10 blocks
```
</details>

<details><summary>2048-stress.sh - stress test for CPU, cache, and memory</summary>

As TDL2048+ is highly optimized, it can be used as a tool for system stability testing, by stressing CPU, cache, and memory.
```bash
./2048-stress.sh $(nproc) # stress all available threads
```
</details>

## Development

TDL2048+ is written in C++14 with optimizations including [AVX2](https://en.wikipedia.org/wiki/Advanced_Vector_Extensions#Advanced_Vector_Extensions_2), [BMI2](https://en.wikipedia.org/wiki/Bit_manipulation_instruction_set#BMI2_(Bit_Manipulation_Instruction_Set_2)), [bitwise operations](https://en.wikipedia.org/wiki/Bitwise_operation), [function pointer](https://en.wikipedia.org/wiki/Function_pointer), [reinterpret cast](https://en.cppreference.com/w/cpp/language/reinterpret_cast), [template specialization](https://en.cppreference.com/w/cpp/language/template_specialization), [template with std::enable_if](https://en.cppreference.com/w/cpp/types/enable_if), [constexpr function](https://en.cppreference.com/w/cpp/language/constexpr), and so on. Development instructions will be released soon.

The following examples are some optimized code.
<details><summary>Show the efficient move generation</summary>

```cpp
inline void moves64(board& U, board& R, board& D, board& L) const {
#if defined(__AVX2__) && !defined(PREFER_LUT_MOVES)
    __m256i dst, buf, rbf, rwd, chk;

    // use left for all 4 directions, transpose and mirror first
    u64 x = raw;
    raw_cast<board>(x).transpose64();
    dst = _mm256_set_epi64x(raw, 0, 0, x); // L, 0, 0, U
    buf = _mm256_set_epi64x(0, x, raw, 0); // 0, D, R, 0
    dst = _mm256_or_si256(dst, _mm256_slli_epi16(buf, 12));
    dst = _mm256_or_si256(dst, _mm256_slli_epi16(_mm256_and_si256(buf, _mm256_set1_epi16(0x00f0)), 4));
    dst = _mm256_or_si256(dst, _mm256_srli_epi16(_mm256_and_si256(buf, _mm256_set1_epi16(0x0f00)), 4));
    dst = _mm256_or_si256(dst, _mm256_srli_epi16(buf, 12));

    // slide to left most
    buf = _mm256_and_si256(dst, _mm256_set1_epi16(0x0f00));
    chk = _mm256_and_si256(_mm256_cmpeq_epi16(buf, _mm256_setzero_si256()), _mm256_set1_epi16(0xff00));
    dst = _mm256_or_si256(_mm256_and_si256(chk, _mm256_srli_epi16(dst, 4)), _mm256_andnot_si256(chk, dst));
    buf = _mm256_and_si256(dst, _mm256_set1_epi16(0x00f0));
    chk = _mm256_and_si256(_mm256_cmpeq_epi16(buf, _mm256_setzero_si256()), _mm256_set1_epi16(0xfff0));
    dst = _mm256_or_si256(_mm256_and_si256(chk, _mm256_srli_epi16(dst, 4)), _mm256_andnot_si256(chk, dst));
    buf = _mm256_and_si256(dst, _mm256_set1_epi16(0x000f));
    chk = _mm256_cmpeq_epi16(buf, _mm256_setzero_si256());
    dst = _mm256_or_si256(_mm256_and_si256(chk, _mm256_srli_epi16(dst, 4)), _mm256_andnot_si256(chk, dst));

    // merge same tiles, slide if necessary
    buf = _mm256_srli_epi16(_mm256_add_epi8(dst, _mm256_set1_epi16(0x0010)), 4);
    rbf = _mm256_and_si256(dst, _mm256_set1_epi16(0x000f));
    chk = _mm256_and_si256(_mm256_srli_epi16(dst, 4), _mm256_set1_epi16(0x000f));
    chk = _mm256_andnot_si256(_mm256_cmpeq_epi16(rbf, _mm256_setzero_si256()), _mm256_cmpeq_epi16(rbf, chk));
    dst = _mm256_or_si256(_mm256_and_si256(chk, buf), _mm256_andnot_si256(chk, dst));
    chk = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_and_si256(chk, _mm256_set1_epi16(0x0001)), 0));
    rbf = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_add_epi16(rbf, _mm256_set1_epi16(0x0001)), 0));
    rwd = _mm256_sllv_epi32(chk, rbf);

    buf = _mm256_add_epi8(_mm256_srli_epi16(dst, 4), _mm256_set1_epi16(0x0010));
    rbf = _mm256_and_si256(buf, _mm256_set1_epi16(0x000f));
    chk = _mm256_and_si256(_mm256_srli_epi16(dst, 8), _mm256_set1_epi16(0x000f));
    chk = _mm256_andnot_si256(_mm256_cmpeq_epi16(rbf, _mm256_setzero_si256()), _mm256_cmpeq_epi16(rbf, chk));
    chk = _mm256_and_si256(chk, _mm256_set1_epi16(0xfff0));
    dst = _mm256_or_si256(_mm256_and_si256(chk, buf), _mm256_andnot_si256(chk, dst));
    chk = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_srli_epi16(chk, 15), 0));
    rbf = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_add_epi16(rbf, _mm256_set1_epi16(0x0001)), 0));
    rwd = _mm256_add_epi32(rwd, _mm256_sllv_epi32(chk, rbf));

    buf = _mm256_srli_epi16(_mm256_add_epi16(dst, _mm256_set1_epi16(0x1000)), 4);
    rbf = _mm256_srli_epi16(dst, 12);
    chk = _mm256_and_si256(_mm256_srli_epi16(dst, 8), _mm256_set1_epi16(0x000f));
    chk = _mm256_andnot_si256(_mm256_cmpeq_epi16(rbf, _mm256_setzero_si256()), _mm256_cmpeq_epi16(rbf, chk));
    chk = _mm256_and_si256(chk, _mm256_set1_epi16(0xff00));
    dst = _mm256_or_si256(_mm256_and_si256(chk, buf), _mm256_andnot_si256(chk, dst));
    chk = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_srli_epi16(chk, 15), 0));
    rbf = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(_mm256_add_epi16(rbf, _mm256_set1_epi16(0x0001)), 0));
    rwd = _mm256_add_epi32(rwd, _mm256_sllv_epi32(chk, rbf));

    // mirror and transpose back to original direction
    L = _mm256_extract_epi64(dst, 3); // L = left
    buf = _mm256_slli_epi16(dst, 12);
    buf = _mm256_or_si256(buf, _mm256_slli_epi16(_mm256_and_si256(dst, _mm256_set1_epi16(0x00f0)), 4));
    buf = _mm256_or_si256(buf, _mm256_srli_epi16(_mm256_and_si256(dst, _mm256_set1_epi16(0x0f00)), 4));
    buf = _mm256_or_si256(buf, _mm256_srli_epi16(dst, 12));
    R = _mm256_extract_epi64(buf, 1); // R = mirror left mirror

    buf = _mm256_blend_epi32(dst, buf, 0b11111100);
    rbf = _mm256_and_si256(_mm256_xor_si256(buf, _mm256_srli_epi64(buf, 12)), _mm256_set1_epi64x(0x0000f0f00000f0f0ull));
    buf = _mm256_xor_si256(buf, _mm256_xor_si256(rbf, _mm256_slli_epi64(rbf, 12)));
    rbf = _mm256_and_si256(_mm256_xor_si256(buf, _mm256_srli_epi64(buf, 24)), _mm256_set1_epi64x(0x00000000ff00ff00ull));
    buf = _mm256_xor_si256(buf, _mm256_xor_si256(rbf, _mm256_slli_epi64(rbf, 24)));
    U = _mm256_extract_epi64(buf, 0); // U = transpose left transpose
    D = _mm256_extract_epi64(buf, 2); // D = transpose mirror left mirror transpose

    // sum the final reward and check moved or not
    rwd = _mm256_add_epi64(rwd, _mm256_srli_si256(rwd, 8 /* bytes */));
    rwd = _mm256_add_epi64(rwd, _mm256_srli_epi64(rwd, 32));
    U.inf = _mm256_extract_epi32(rwd, 0);
    R.inf = _mm256_extract_epi32(rwd, 4);
    rwd = _mm256_set_epi64x(R.inf, U.inf, R.inf, U.inf);
    chk = _mm256_cmpeq_epi64(_mm256_set_epi64x(L, D, R, U), _mm256_set1_epi64x(raw));
    rwd = _mm256_or_si256(rwd, chk);
    U.inf = _mm256_extract_epi32(rwd, 0);
    R.inf = _mm256_extract_epi32(rwd, 2);
    D.inf = _mm256_extract_epi32(rwd, 4);
    L.inf = _mm256_extract_epi32(rwd, 6);

#else // if AVX2 is unavailable or disabled
    U = R = D = L = board();

    qrow16(0).moveh64<0>(L, R);
    qrow16(1).moveh64<1>(L, R);
    qrow16(2).moveh64<2>(L, R);
    qrow16(3).moveh64<3>(L, R);
    L.inf |= (L.raw ^ raw) ? 0 : -1;
    R.inf |= (R.raw ^ raw) ? 0 : -1;

    qcol16(0).movev64<0>(U, D);
    qcol16(1).movev64<1>(U, D);
    qcol16(2).movev64<2>(U, D);
    qcol16(3).movev64<3>(U, D);
    U.inf |= (U.raw ^ raw) ? 0 : -1;
    D.inf |= (D.raw ^ raw) ? 0 : -1;
#endif
}
```
</details>

<details><summary>Show the hierarchical pattern-based structures</summary>

```cpp
template<u32... patt>
inline constexpr u32 order() {
	if (sizeof...(patt) > 8 || sizeof...(patt) == 0) return -1;
	constexpr u32 x[] = { patt... };
	for (u32 i = 1; i < sizeof...(patt); i++) if (x[i] <= x[i - 1])     return 0; // unordered
#if defined(__BMI2__) && !defined(PREFER_LEGACY_INDEXPT_ORDER)
	for (u32 i = 1; i < sizeof...(patt); i++) if (x[i] != x[i - 1] + 1) return 1; // ordered
#else
	for (u32 i = 1; i < sizeof...(patt); i++) if (x[i] != x[i - 1] + 1) return 0; // ordered (fall back)
#endif
	return 2; // strictly ordered
}

template<u32... patt>
inline constexpr typename std::enable_if<order<patt...>() == 0, u64>::type indexpt(const board& b) {
	u32 index = 0, n = 0;
	for (u32 p : { patt... }) index += b.at(p) << (n++ << 2);
	return index;
}

#if defined(__BMI2__) && !defined(PREFER_LEGACY_INDEXPT_ORDER)
template<u32... patt>
inline constexpr typename std::enable_if<order<patt...>() == 1, u64>::type indexpt(const board& b) {
	u64 mask = 0;
	for (u64 p : { patt... }) mask |= 0xfull << (p << 2);
	return math::pext64(b, mask);
}
#elif !defined(PREFER_LEGACY_INDEXPT_ORDER) // specialize common ordered patterns when BMI2 is unavailable
template<> u64 indexpt<0x0,0x1,0x4,0x5>(const board& b) { return ((u64(b)) & 0x00ff) | ((u64(b) >> 8) & 0xff00); }
template<> u64 indexpt<0x1,0x2,0x5,0x6>(const board& b) { return ((u64(b) >> 4) & 0x00ff) | ((u64(b) >> 12) & 0xff00); }
template<> u64 indexpt<0x5,0x6,0x9,0xa>(const board& b) { return ((u64(b) >> 20) & 0x00ff) | ((u64(b) >> 28) & 0xff00); }
template<> u64 indexpt<0x0,0x1,0x2,0x4,0x5>(const board& b) { return ((u64(b)) & 0x00fff) | (((u64(b)) >> 4) & 0xff000); }
template<> u64 indexpt<0x4,0x5,0x6,0x8,0x9>(const board& b) { return ((u64(b) >> 16) & 0x00fff) | ((u64(b) >> 20) & 0xff000); }
template<> u64 indexpt<0x0,0x1,0x2,0x3,0x5>(const board& b) { return ((u64(b)) & 0x0ffff) | ((u64(b) >> 4) & 0xf0000); }
template<> u64 indexpt<0x4,0x5,0x6,0x7,0x9>(const board& b) { return ((u64(b) >> 16) & 0x0ffff) | ((u64(b) >> 20) & 0xf0000); }
template<> u64 indexpt<0x0,0x1,0x2,0x4,0x5,0x6>(const board& b) { return ((u64(b)) & 0x000fff) | (((u64(b)) >> 4) & 0xfff000); }
template<> u64 indexpt<0x4,0x5,0x6,0x8,0x9,0xa>(const board& b) { return ((u64(b) >> 16) & 0x000fff) | ((u64(b) >> 20) & 0xfff000); }
template<> u64 indexpt<0x2,0x3,0x4,0x5,0x6,0x9>(const board& b) { return ((u64(b) >> 8) & 0x0fffff) | ((u64(b) >> 16) & 0xf00000); }
template<> u64 indexpt<0x0,0x1,0x2,0x5,0x9,0xa>(const board& b) { return ((u64(b)) & 0x000fff) | ((u64(b) >> 8) & 0x00f000) | ((u64(b) >> 20) & 0xff0000); }
template<> u64 indexpt<0x1,0x3,0x4,0x5,0x6,0x7>(const board& b) { return ((u64(b) >> 4) & 0x00000f) | ((u64(b) >> 8) & 0xfffff0); }
template<> u64 indexpt<0x0,0x1,0x4,0x8,0x9,0xa>(const board& b) { return ((u64(b)) & 0x0000ff) | ((u64(b) >> 8) & 0x000f00) | ((u64(b) >> 20) & 0xfff000); }
#endif

template<u32 p, u32... x>
inline constexpr typename std::enable_if<order<p, x...>() == 2, u64>::type indexpt(const board& b) {
	return u32(u64(b) >> (p << 2)) & u32((1ull << ((sizeof...(x) + 1) << 2)) - 1);
}
```
</details>

<details><summary>Show the sophisticated built-in network structures</summary>

```cpp
template<typename mode = weight::segment>
struct isomorphic {
    constexpr inline operator method() { return { isomorphic::estimate, isomorphic::optimize }; }

    constexpr static inline_always numeric invoke(const board& iso, clip<feature> f) {
        register numeric esti = 0;
        for (register auto feat = f.begin(); feat != f.end(); feat += 8)
            esti += feat->at<mode>(iso);
        return esti;
    }
    constexpr static inline_always numeric invoke(const board& iso, numeric updv, clip<feature> f) {
        register numeric esti = 0;
        for (register auto feat = f.begin(); feat != f.end(); feat += 8)
            esti += (feat->at<mode>(iso) += updv);
        return esti;
    }

    template<estimator estim = isomorphic::invoke>
    constexpr static inline numeric estimate(const board& state, clip<feature> range = feature::feats()) {
        register numeric esti = 0;
        register board iso;
        esti += estim(({ iso = state;     iso; }), range);
        esti += estim(({ iso.flip();      iso; }), range);
        esti += estim(({ iso.transpose(); iso; }), range);
        esti += estim(({ iso.flip();      iso; }), range);
        esti += estim(({ iso.transpose(); iso; }), range);
        esti += estim(({ iso.flip();      iso; }), range);
        esti += estim(({ iso.transpose(); iso; }), range);
        esti += estim(({ iso.flip();      iso; }), range);
        return esti;
    }
    template<optimizer optim = isomorphic::invoke>
    constexpr static inline numeric optimize(const board& state, numeric updv, clip<feature> range = feature::feats()) {
        register numeric esti = 0;
        register board iso;
        esti += optim(({ iso = state;     iso; }), updv, range);
        esti += optim(({ iso.flip();      iso; }), updv, range);
        esti += optim(({ iso.transpose(); iso; }), updv, range);
        esti += optim(({ iso.flip();      iso; }), updv, range);
        esti += optim(({ iso.transpose(); iso; }), updv, range);
        esti += optim(({ iso.flip();      iso; }), updv, range);
        esti += optim(({ iso.transpose(); iso; }), updv, range);
        esti += optim(({ iso.flip();      iso; }), updv, range);
        return esti;
    }

    template<indexer::mapper... indexes>
    struct static_index {
        constexpr static std::array<indexer::mapper, sizeof...(indexes)> index = { indexes... };
        constexpr inline operator method() { return { static_index::estimate, static_index::optimize }; }

        template<indexer::mapper index, indexer::mapper... follow> constexpr static
        inline_always typename std::enable_if<(sizeof...(follow) != 0), numeric>::type invoke(const board& iso, clip<feature> f) {
            return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3].at<mode>(index(iso))) + invoke<follow...>(iso, f);
        }
        template<indexer::mapper index, indexer::mapper... follow> constexpr static
        inline_always typename std::enable_if<(sizeof...(follow) == 0), numeric>::type invoke(const board& iso, clip<feature> f) {
            return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3].at<mode>(index(iso)));
        }
        template<indexer::mapper index, indexer::mapper... follow> constexpr static
        inline_always typename std::enable_if<(sizeof...(follow) != 0), numeric>::type invoke(const board& iso, numeric updv, clip<feature> f) {
            return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3].at<mode>(index(iso)) += updv) + invoke<follow...>(iso, updv, f);
        }
        template<indexer::mapper index, indexer::mapper... follow> constexpr static
        inline_always typename std::enable_if<(sizeof...(follow) == 0), numeric>::type invoke(const board& iso, numeric updv, clip<feature> f) {
            return (f[(sizeof...(indexes) - sizeof...(follow) - 1) << 3].at<mode>(index(iso)) += updv);
        }

        constexpr static estimator estimate = isomorphic::estimate<invoke<indexes...>>;
        constexpr static optimizer optimize = isomorphic::optimize<invoke<indexes...>>;
    };

    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9>,
            index::indexpt<0x0,0x1,0x2,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x8,0x9,0xa>> idx4x6patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9>,
            index::indexpt<0x8,0x9,0xa,0xb,0xc,0xd>,
            index::indexpt<0x0,0x1,0x2,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x8,0x9,0xa>> idx5x6patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9>,
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5>,
            index::indexpt<0x2,0x3,0x4,0x5,0x6,0x9>,
            index::indexpt<0x0,0x1,0x2,0x5,0x9,0xa>,
            index::indexpt<0x3,0x4,0x5,0x6,0x7,0x8>> idx6x6patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9>,
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5>,
            index::indexpt<0x2,0x3,0x4,0x5,0x6,0x9>,
            index::indexpt<0x0,0x1,0x2,0x5,0x9,0xa>,
            index::indexpt<0x3,0x4,0x5,0x6,0x7,0x8>,
            index::indexpt<0x1,0x3,0x4,0x5,0x6,0x7>> idx7x6patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9>,
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5>,
            index::indexpt<0x2,0x3,0x4,0x5,0x6,0x9>,
            index::indexpt<0x0,0x1,0x2,0x5,0x9,0xa>,
            index::indexpt<0x3,0x4,0x5,0x6,0x7,0x8>,
            index::indexpt<0x1,0x3,0x4,0x5,0x6,0x7>,
            index::indexpt<0x0,0x1,0x4,0x8,0x9,0xa>> idx8x6patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9,0xa>> idx2x7patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5,0x6>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9,0xa>,
            index::indexpt<0x8,0x9,0xa,0xb,0xc,0xd,0xe>> idx3x7patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>> idx1x8patt;
    typedef typename isomorphic<mode>::template static_index<
            index::indexpt<0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7>,
            index::indexpt<0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb>> idx2x8patt;
};
```
</details>

## Authors

TDL2048+ is under development by [Hung Guei](mailto:hguei@moporgic.info) since June 2015.

## Acknowledgments

The author is with [Computer Games and Intelligence (CGI) Lab](https://cgilab.nctu.edu.tw/), [Department of Computer Science](https://www.cs.nycu.edu.tw/), [National Yang Ming Chiao Tung University (NYCU)](https://www.nycu.edu.tw/en/), under the supervision of Professor [I-Chen Wu](https://cgilab.nctu.edu.tw/~icwu/).

Some parts of this program, e.g., the 16-bit floating-point support, use other open sources.

## License

TDL2048+ is licensed under the MIT License.
<details><summary>Show the license</summary>

Copyright (c) 2021 Hung Guei

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
</details>

## References

* M. Szubert and W. Jaśkowski, “Temporal difference learning of N tuple networks for the game 2048,” in Proc. 2014 IEEE Conf. Comput. Intell. Games, Dortmund, Germany, 2014, pp. 1–8. DOI: [10.1109/CIG.2014.6932907](https://doi.org/10.1109/CIG.2014.6932907).
* K.-H. Yeh, I-C. Wu, C.-H. Hsueh, C.-C. Chang, C.-C. Liang, and H. Chiang, “Multistage temporal difference learning for 2048-like games,” IEEE Trans. Comput. Intell. AI Games, vol. 9, no. 4, pp. 369–380, Dec. 2017. DOI: [10.1109/TCIAIG.2016.2593710](https://doi.org/10.1109/TCIAIG.2016.2593710).
* W. Jaśkowski, “Mastering 2048 with delayed temporal coherence learning, multistage weight promotion, redundant encoding and carousel shaping,” IEEE Trans. Games, vol. 10, no. 1, pp. 3–14, Mar. 2018. DOI: [10.1109/TCIAIG.2017.2651887](https://doi.org/10.1109/TCIAIG.2017.2651887).
* K. Matsuzaki, “Systematic selection of N tuple networks with consideration of interinfluence for game 2048,” in Proc. 21st Int. Conf. Technol. Appl. Artif. Intell., Hsinchu, Taiwan, 2016, pp. 186–193. DOI: [10.1109/TAAI.2016.7880154](https://doi.org/10.1109/TAAI.2016.7880154).
* H. Guei, T.-H. Wei, and I-C. Wu, “2048-like games for teaching reinforcement learning,” ICGA J., vol. 42, no. 1, pp. 14–37, May 28, 2020. DOI: [10.3233/ICG-200144](https://doi.org/10.3233/ICG-200144).
* K. Matsuzaki, “Developing a 2048 player with backward temporal coherence learning and restart,” in Proc. 15th Int. Conf. Adv. Comput. Games, Leiden, The Netherlands, 2017, pp. 176–187. DOI: [10.1007/978-3-319-71649-7_15](https://doi.org/10.1007/978-3-319-71649-7_15).
* H. Guei, L.-P. Chen and I-C. Wu, "Optimistic Temporal Difference Learning for 2048," in IEEE Trans. Games, Sep 3, 2021. DOI: [10.1109/TG.2021.3109887](https://doi.org/10.1109/TG.2021.3109887).
* K.-H. Yeh. “2048 AI.” [Online]. Available: https://github.com/tnmichael309/2048AI.
* K.-C. Wu. “2048-c.” [Online]. Available: https://github.com/kcwu/2048-c.
* H. Guei. “moporgic/TDL2048+.” [Online]. Available: https://github.com/moporgic/TDL2048.