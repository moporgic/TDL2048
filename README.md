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

As of April 2021, TDL2048+ is the most efficient framework for 2048 in terms of speed.
Its single-thread (ST) and multi-thread (MT) speed (moves/s) are summarized as follows.

|Processor         |ST Training|ST  Testing|MT Training|MT  Testing|
|------------------|----------:|----------:|----------:|----------:|
|Core i9-7980XE    |  3,789,824|  4,307,902| 34,947,198| 67,576,154|
|Xeon E5-2698 v4   |  2,911,945|  3,551,124| 30,013,758| 49,707,145|
|Core i7-6950X     |  3,237,049|  3,860,228| 22,646,286| 35,324,773|
|Core i7-6900K     |  3,080,106|  3,679,456| 17,707,134| 27,486,805|
|Xeon E5-2696 v3   |  1,776,048|  1,983,475| 16,294,325| 21,588,230|
|Xeon E5-2683 v3 x2|  1,442,213|  1,670,918| 14,483,794| 30,096,929|
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
|Raspberry Pi 2B|Arch Linux 5.10    |GCC 10.2|

</details>

## Compilation

The provided makefile is designed for most common use, which is covered in this subsection.

### Building

By default, TDL2048+ is built with ```-O3 -mtune=native```.
```bash
make # build TDL2048+ with default settings
```
After a successful build, a binary file ```./2048``` should be generated.

<details><summary>Show advanced options</summary><br>

To use another file name instead of ```./2048```, set the ```OUTPUT``` as follow.
```bash
make OUTPUT="run" # the output binary will be ./run instead of ./2048
```

If other optimization level is needed, set the ```OLEVEL``` as follow.
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
Building with PGO may significantly improve the program speed of certain scenarios, at the expense of overall speed. However, it is still worthy especially if a long-term training is required.

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

To maximize the training speed (however, with poor testing speed), set the ```PGO_EVAL``` as follow.
```bash
make 4x6patt PGO_EVAL="0" # profile with only training routine
```

Similarly, to maximize the testing speed, set the ```PGO_OPTI``` as follow.
```bash
make 4x6patt PGO_OPTI="0" # profile with only testing routine
```

In addition, the above profiling are all with TD. To profile with TC, set the ```PGO_ALPHA``` as follow.
```bash
make 4x6patt PGO_ALPHA="0 coherence" # profile with TC learning
```

To profile the expectimax search, disable the training with ```PGO_OPTI``` and set the depth with ```PGO_FLAGS``` as follow.
```bash
make 4x6patt PGO_OPTI="0" PGO_FLAGS="-d 2p" # profile with 2-ply search
```

#### Profiling with Custom Recipes

If the pre-defined profiling recipes do not meet your requirements, a script ```make-profile.sh``` can be used for custom profiling. First, define your own profiling recipe in ```make-profile.sh``` like
```bash
./2048 -n 4x6patt -i 4x6patt.w -a 0 fixed -t 1x10000 -e 1x10000 -% none -s
```

Then, make with target ```profile``` as follow.
```bash
make profile # profile with make-profile.sh
```

To profile with multithreading (```-p```), set ```-fprofile-update=atomic``` explicitly as follow.
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

On some machines, such as the AMD Ryzen 3000 series, you may want to explicitly disable BMI2 because of their [slow implementation](http://www.talkchess.com/forum3/viewtopic.php?f=7&t=72538). This can be done as follow.
```bash
make BMI2="no" # build with default settings but disable the BMI2 optimization
```

#### Specify Default Target

Note that target ```default``` is used when making ```dump```, ```profile```, ```4x6patt```, ..., and ```8x6patt```.
To specify another target, such as ```static```, set ```TARGET="static"``` as follow.

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

Similarly, an example of testing is as follow, in which a pre-trained network ```4x6patt.w``` is loaded and tested with 1000k episodes, and the log is written to ```4x6patt.x```.
```bash
./2048 -n 4x6patt -e 1000 -i 4x6patt.w -o 4x6patt.x
```

More CLI options will be introduced in the following subsection.

#### N-Tuple Network

Use ```-n``` to specify the target network structure as follow. TDL2048+ supports any pattern-based structures with no more than 8-tuple, in which  common pattern-based designs are per-defined for simplicity, such as ```4x6patt```, ```5x6patt```, ```56x6patt```, ```7x6patt```, ```8x6patt```, and so on.
```bash
./2048 -n 8x6patt -t 1000 # use the 8x6patt network
```

<details><summary>Show advanced options</summary><br>

Pattern-based structures are defined with cell locations, using characters from '0', '1', ... 'f' to indicate the cell from upper left corner to lower right corner, respectively.

Below is the mapping of built-in aliases and corresponding patterns.
```
4x6patt             = an alias of 4x6patt/khyeh
5x6patt             = an alias of 5x6patt/42-33
2x4patt             = an alias of 2x4patt/4
5x4patt             = an alias of 5x4patt/4-22
8x4patt             = an alias of 8x4patt/legacy
1x8patt             = an alias of 1x8patt/44
2x8patt             = an alias of 2x8patt/44
3x8patt             = an alias of 3x8patt/44-4211
4x8patt             = an alias of 4x8patt/44-332-4211
2x7patt             = an alias of 2x7patt/43
3x7patt             = an alias of 3x7patt/43
6x6patt             = an alias of 6x6patt/k.matsuzaki
7x6patt             = an alias of 7x6patt/k.matsuzaki
8x6patt             = an alias of 8x6patt/k.matsuzaki

4x6patt/khyeh       = 012345 456789 012456 45689a
5x6patt/42-33       = 012345 456789 89abcd 012456 45689a
2x4patt/4           = 0123 4567
5x4patt/4-22        = 0123 4567 0145 1256 569a
8x4patt/legacy      = 0123 4567 89ab cdef 048c 159d 26ae 37bf
9x4patt/legacy      = 0145 1256 2367 4589 569a 67ab 89cd 9ade abef
1x8patt/44          = 01234567
2x8patt/44          = 01234567 456789ab
3x8patt/44-332      = 01234567 456789ab 01245689
3x8patt/44-4211     = 01234567 456789ab 0123458c
4x8patt/44-332-4211 = 01234567 456789ab 01245689 0123458c
2x7patt/43          = 0123456 456789a
3x7patt/43          = 0123456 456789a 89abcde
4x6patt/k.matsuzaki = 012456 456789 012345 234569
5x6patt/k.matsuzaki = 012456 456789 012345 234569 01259a
6x6patt/k.matsuzaki = 012456 456789 012345 234569 01259a 345678
7x6patt/k.matsuzaki = 012456 456789 012345 234569 01259a 345678 134567
8x6patt/k.matsuzaki = 012456 456789 012345 234569 01259a 345678 134567 01489a
```

The most common structures, ```4x6patt```, ```5x6patt```, ```6x6patt```, ```7x6patt```, ```8x6patt```, ```2x7patt```, ```3x7patt```, ```1x8patt```, and ```2x8patt```, are with the sophisticated optimization enabled by default.

All pattern-based networks are defined with isomorphisms, e.g., a pattern ```012456``` actually involves ```012345 37bf26 fedcba c840d9 cdef89 fb73ea 321076 048c15```.
To define a structure with custom patterns, specify the patterns in the ```-n``` command as following example. Note that custom patterns is less efficient than the built-in patterns.
```bash
./2048 -n 012345 456789 012456 45689a -t 1000
```
</details><br>

In addition, some statistic-based designs are also supported, such as the monotonicity (```mono```), and the number of tiles (```num```, ```num@lt```, ```num@st```). You may use them with pattern-based designs.
```bash
./2048 -n 4x6patt mono num@st # use the 4x6patt, mono, and # of large tiles
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

#### More Training Methods

The default training mode is TD(0).

To enable n-step TD(0), use ```-N``` to specify a step size.
```bash
./2048 -n 4x6patt -t 1000 -N 10 # use 10-step TD(0) training
```

To enable TD(λ), use ```-l``` to specify a lambda value.
```bash
./2048 -n 4x6patt -t 1000 -l 0.5 # use TD(0.5) training
```

<details><summary>Show advanced options</summary><br>

The three training modes, TD, n-step TD, and TD(λ), are with both forward and backward training variants. By default, TD and n-step TD use forward, while TD(λ) uses backward training.

Compared with forward training, backward training for TD and n-step is slightly inefficient in terms of speed, but it achieves a higher average score with the same learned episodes.

Option ```-tt``` specifies an advanced training mode as follow.
```bash
./2048 -n 4x6patt -t 1000 -tt backward # backward TD(0)
./2048 -n 4x6patt -t 1000 -tt step-backward -N 5 # backward 5-step TD(0)
```

You may enable the forward TD(λ) with ```-tt lambda-forward``` with a step size with ```-N``` as follow.
```bash
./2048 -n 4x6patt -t 1000 -tt lambda-forward -l 0.5 -N 5 # forward 5-step TD(0.5)
```
</details>

#### Learning Rate and TC

The learning rate is 0.1 by default, but you may want to manually modulate the value especially when the network is saturated. This can be done by using ```-a``` as follow.
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

The learning rate is distributed to each n-tuple feature weight. For example, the ```4x6patt``` network has 32 feature weights, so a weight is actually adjusted with a rate of 0.01 if ```-a 0.32``` is set.

However, you may use ```norm``` together with ```-a``` to override the default behaviour as
```bash
./2048 -n 4x6patt -t 1000 -a 0.0025 norm=1 # each weight is adjusted a rate of 0.0025/1
```
</details>

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

Sometimes, you may want to limit the maximum search depth when there are too many empty cells on the puzzle. This can be set by using ```limit``` with ```-d``` as follow.
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 5p limit=5p,5p,5p,5p,4p,4p,4p,4p,3p
```
In the above example, the search starts with 5-ply at root, in which the depth is limited to 5-ply if there are 0 to 3 empty cells; 4-ply if there are 4 to 7 empty cells; and 3-ply if there are 8 or more empty cells. Note that ```limit=``` accepts at most 16 values, corresponding to 0 to 15 empty cells.
</details><br>

To speed up the search, a transposition table (TT) can be enabled with ```-c``` as
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 3p -c 8G # 3-ply search with 8G TT
```
The size of TT must be the power of 2, e.g., 4G, 8G, 16G, 64G, etc.

<details><summary>Show advanced options</summary><br>

Note that using TT involves a lot of memory access, which may result in worse search speed in some cases especially when the search depth is less than 3-ply.

In addition, for deep search such as 5-ply, you may want to allow the search to use a "deeper" TT result, which may improve the strength and can be set by using ```peek``` with ```-c``` as follow.
```bash
./2048 -n 4x6patt -i 4x6patt.w -e 10 -d 5p -c 64G peek
```

To be more specific, if there is only a result with 5-ply for a given puzzle but you need a result with 3-ply, setting ```peek``` allows the search to take the 5-ply result directly.
</details>

#### Saving/Loading and Logging

The program uses ```-i``` for loading, ```-o``` for saving and logging.
For simplicity, ```-io``` toggles both ```-i``` and ```-o```, which is useful during long-term training.

Three types of files are currently support: n-tuple network (```.w```), TT cache (```.c```), and log file (```.x```). The file must be named with the correct extension.

Note that no error message will appear when an IO failure occurs.

<details><summary>Show some examples</summary><br>

To log to ```4x6patt-training.x``` and save the trained network as ```4x6patt.w```:
```bash
./2048 -n 4x6patt -t 1000 -o 4x6patt.w 4x6patt-training.x
```

To load from a trained network and log the results of expectimax search:
```bash
./2048 -n 4x6patt -e 1000 -d 3p -i 4x6patt.w -o 4x6patt-testing.x
```

To save snapshots regularly and prevent power failure during a long-term training:
```bash
for i in {01..10}; do
    ./2048 -n 4x6patt -t 1000 -e 10 -io 4x6patt.w -o 4x6patt.x
    tar Jcvf 4x6patt.$i.tar.xz 4x6patt.w 4x6patt.x
done
```
</details>

#### Random Seed

The pseudo-random number generator is randomly initialized by default.
To specify a fixed seed, use ```-s``` with a custom string as follow.
```bash
./2048 -n 4x6patt -t 1000 -s Hello # use "Hello" to seed the PRNG
```

#### Parallel Execution

The program executes with only single thread by default.
To enable parallel execution, use ```-p``` with the number of threads as follow.
```bash
./2048 -n 4x6patt -t 1000 -p 10 # use 10 threads for execution
```
If the number of threads is not provided, the program will use all available threads.

<details><summary>Show advanced options</summary><br>

Make sure that the number in ```-t``` or ```-e``` is larger than the parallism.
In the following example, only thread 1 to thread 10 will work, while other threads will be idle.
```bash
./2048 -n 4x6patt -i 4x6patt -d 5p -c 64G -e 10 -p 20
```

To prevent this from happening, set the unit instead of using the default (1000) as follow.
```bash
./2048 -n 4x6patt -i 4x6patt -d 5p -c 64G -e 20x500 -p 20
```

In addition, the program will automatically toggle the use of [shared memory (SHM)](https://en.wikipedia.org/wiki/Shared_memory) on Linux platforms, since using ```fork``` performs better than using ```std::thread``` in speed.

To explicitly disable the SHM, add ```noshm``` with ```-p``` to tell the program to use ```std::thread```.
```bash
./2048 -n 4x6patt -i 4x6patt -d 5p -c 64G -e 20x500 -p 20 noshm
```

Note that on Linux platforms, if you keep the SHM option unchanged, specifying both ```-t``` and ```-e``` in a same command with parallelism leading to a slightly worse testing speed.
```bash
./2048 -n 4x6patt -t 1000 -e 1000 -p 10 -o 4x6patt.w # training, then testing
```

```bash
./2048 -n 4x6patt -t 1000 -p 10 -o 4x6patt.w # training,
./2048 -n 4x6patt -e 1000 -p 10 -i 4x6patt.w # then testing
```

Due to a current limitation, the speed of ```-e 1000``` in the former is slower than that in the latter. However, should still be faster than using ```std::thread```.
</details>

#### Miscellaneous

<details><summary>Summary</summary>

By default, the summary is only printed for testing (```-e```).
Use ```-%``` to display it also for training; use ```-% none``` to hide the it for testing.
</details>

<details><summary>Winning Tile</summary>

To change the winning tile of statistics, specify ```-w``` with a tile value, e.g., ```-w 32768```.
</details>

<details><summary>Help Message</summary>

Use ```-?``` to print a brief CLI usage.
</details>

### Log Format

The log of executing ```./2048 -n 4x6patt -t 10 -e 10``` is used as an example.

A log may contain version info, CLI arguments, parameters, statistic blocks, and summary blocks.

<details><summary>Show log structure</summary><br>

At the beginning, there is build info including build name, C++ version, and build time.
```
TDL2048+ by Hung Guei
Develop (GCC 10.2.0 C++201402 @ 2021-01-26 16:02:47)
```

The next several lines print the program arguments and some important parameters.
```
./2048 -n 4x6patt -t 10 -e 10
time = 2021-03-04 21:39:54.142
seed = d1b2759358
alpha = 0.1
lambda = 0, step = 1
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

001/010 115ms 4465086.96ops
local:  avg=7628 max=27088 tile=2048 win=0.40%
total:  avg=7628 max=27088 tile=2048 win=0.40%

002/010 152ms 4447631.58ops
local:  avg=10757 max=28876 tile=2048 win=2.40%
total:  avg=9193 max=28876 tile=2048 win=1.40%

... (some lines are skipped here) ...

009/010 271ms 4231693.73ops
local:  avg=20394 max=58952 tile=4096 win=31.80%
total:  avg=14859 max=60812 tile=4096 win=14.07%

010/010 290ms 4227275.86ops
local:  avg=22122 max=62668 tile=4096 win=36.90%
total:  avg=15585 max=62668 tile=4096 win=16.35%
```

Finally, the testing part comes with a ```evaluate``` label, and ends with a summary block.
```
evaluate: 10 info

001/010 261ms 4688563.22ops
local:  avg=22151 max=70588 tile=4096 win=39.20%
total:  avg=22151 max=70588 tile=4096 win=39.20%

002/010 264ms 4674011.36ops
local:  avg=22325 max=65016 tile=4096 win=40.00%
total:  avg=22238 max=70588 tile=4096 win=39.60%

... (some lines are skipped here) ...

009/010 265ms 4662464.15ops
local:  avg=22390 max=74996 tile=4096 win=39.60%
total:  avg=22437 max=76716 tile=4096 win=39.49%

010/010 264ms 4671030.30ops
local:  avg=22339 max=72336 tile=4096 win=40.40%
total:  avg=22427 max=76716 tile=4096 win=39.58%

summary 2648ms 4671990.18ops
total:  avg=22427 max=76716 tile=4096 win=39.58%
tile     count   score    move     rate      win
64           1     712      95    0.01%  100.00%
128          8    2081     201    0.08%   99.99%
256         84    3931     318    0.84%   99.91%
512       1207    8660     593   12.07%   99.07%
1024      4742   17511    1037   47.42%   87.00%
2048      3688   31325    1633   36.88%   39.58%
4096       270   55217    2536    2.70%    2.70%
```
</details>

#### Statistic Block

A statistic block is the result of 1000 episodes by default, with three lines as follows.
```
002/010 264ms 4674011.36ops
local:  avg=22325 max=65016 tile=4096 win=40.00%
total:  avg=22238 max=70588 tile=4096 win=39.60%
```

The first line shows the index number (```002/010```, current/total), the time used for this block (```264ms```), and the measured speed in moves/s (```4674011.36ops```).
When using parallel execution, there is an additional ```[#]``` to display the executor id of this block.

The second line, ```local```, shows the statistic of this block, including the average score (```avg=22325```), the maximum score (```max=65016```), the maximum tile (```tile=4096```), and the win rate (```win=40.00%```).

The third line, ```total```, has the same format as the second line, but shows the statistics starting from the first index (```001/xxx```) of this session. Note that ```-t 10 -e 10``` are two separate sessions.

#### Summary Block

A summary block only be attached at the end of a testing session (```-e```) by default.
```
summary 2648ms 4671990.18ops
total:  avg=22427 max=76716 tile=4096 win=39.58%
tile     count   score    move     rate      win
64           1     712      95    0.01%  100.00%
128          8    2081     201    0.08%   99.99%
256         84    3931     318    0.84%   99.91%
512       1207    8660     593   12.07%   99.07%
1024      4742   17511    1037   47.42%   87.00%
2048      3688   31325    1633   36.88%   39.58%
4096       270   55217    2536    2.70%    2.70%
```

The first line shows the total time usage (```2648ms```) and the measured speed (```4671990.18ops```) of this session. The second line is the same as the ```total``` of the last statistic block.

The tile-specific statistic is printed starting from the third line, in which ```count``` shows how many episodes end with only ```tile```; ```score``` and ```move``` show the average score and average moves of such episodes; ```rate``` shows the percentage of such episodes; and ```win``` shows the reaching rate.

Take ```2048-tile``` as an example, there are ```3688``` games end with it, their average score and average moves are ```31325``` and ```1633```, respectively. Since this session has 10k games, the percentage of games ending with it is ```36.88%```; and the reaching rate is ```39.58%```, calculated from ```100% - (0.01% + 0.08% + 0.84% + 12.07% + 47.42%)```.

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

Extract AVG, MAX, TILE, and WIN from a log, which is useful when exporting statistics.
```bash
grep local 2048.x | ./2048-arse.sh # extract AVG, MAX, TILE, and WIN from each block
```
</details>

<details><summary>2048-winrate.sh - log parser for summary block</summary>

Extract win rates of large tiles and print them. Check the script for more details.
```bash
./2048-winrate.sh < 2048.x # extract win rates and print them
```
</details>

<details><summary>2048-thick.sh - log concentrator for statistic block</summary>

This script helps you rearrange a log of statistic blocks.
For example, follow below steps to join 10 statistic blocks and print the new statistics.
```bash
./2048-thick.sh 10 < 2048.x
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

The author in with [Computer Games and Intelligence (CGI) Lab](https://cgilab.nctu.edu.tw/), [NYCU, Taiwan](https://www.nycu.edu.tw/en/), under the supervision of Professor [I-Chen Wu](https://cgilab.nctu.edu.tw/~icwu/).

Some parts of this program, e.g., the 16-bit floating point support, use other open source.

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
* K.-H. Yeh. “2048 AI.” [Online]. Available: https://github.com/tnmichael309/2048AI.
* K.-C. Wu. “2048-c.” [Online]. Available: https://github.com/kcwu/2048-c.