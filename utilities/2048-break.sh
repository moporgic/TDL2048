#!/bin/bash
#Program: log fixer of TDL2048+
#History: 2018/06/07 moporgic
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin

#001/500 1765ms 596549.01ops [8]
#local:  avg=18580 max=51140 tile=4096 win=28.60%
#total:  avg=18580 max=51140 tile=4096 win=28.60%

sed -E 's/(%)([^[:space:]])/\1\n\2/g' | sed -E 's/(])([^[:space:]])/\1\n\2/g'