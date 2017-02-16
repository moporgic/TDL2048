all:
	g++ -std=c++1y -O3 2048.cpp -o 2048
nproc:
	g++ -std=c++1y -j$(nproc) -O3 2048.cpp -o 2048
clean:
	[ -e 2048 ] && rm 2048