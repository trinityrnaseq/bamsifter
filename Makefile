
sift_bam_max_cov: sift_bam_max_cov.cpp htslib/version.h
	g++ -std=c++11 -o bamsifter sift_bam_max_cov.cpp -Wall -O2 -L./htslib/build/lib/ -I./htslib/build/include -Wl,-Bstatic -lhts -Wl,-Bdynamic -lbz2  -Wl,-Bstatic -lhts -Wl,-Bdynamic -lm -lpthread -lz -llzma -lcurl -lcrypto

htslib/version.h : htslib/version.sh 
	./build_htslib.sh


