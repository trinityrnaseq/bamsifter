sift_bam_max_cov: sift_bam_max_cov.cpp
	g++ -Wall -I/usr/local/opt/zlib/include -lhts -O2 -o sift_bam_max_cov.exe sift_bam_max_cov.cpp

