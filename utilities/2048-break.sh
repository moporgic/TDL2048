#!/bin/bash
#Program: log fixer of TDL2048+
#History: 2018/06/07 moporgic

#A record consists of 3 lines
#001/500 1765ms 596549.01ops [8]
#local:  avg=18580 max=51140 tile=4096 win=28.60%
#total:  avg=18580 max=51140 tile=4096 win=28.60%

#Some bad lines caused by std::cout << data << std::endl;
#total:  avg=41465 max=163240 tile=8192 win=74.09%007/500 4391ms 632054.20ops [16]
#local:  avg=64134 max=167596 tile=8192 win=90.80%total:  avg=47786 max=169948 tile=8192 win=80.66%
#011/500 5079ms 598854.50ops [9]010/500 5828ms 524772.82ops [4]
#019/500 5188ms 688024.48ops [12]local:  avg=82798 max=234900 tile=16384 win=95.20%

sed -E 's/(%|])([^[:space:]])/\1\n\2/g'