all:
	g++ -std=c++1y -O3 -Wall -fmessage-length=0 -o 2048 2048.cpp
clean:
	rm 2048
dump:
	g++ -std=c++1y -O3 -Wall -fmessage-length=0 -g -o 2048 2048.cpp
	objdump -S 2048 > 2048.dump