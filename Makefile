sift_bam_max_cov: sift_bam_max_cov.cpp
	g++ -std=c++11 sift_bam_max_cov.cpp -Wall -lhts -O2 -I../htslib -L../htslib -o bamsifter
