sift_bam_max_cov: sift_bam_max_cov.cpp
	g++ -Wall -lhts -O2 -I../htslib -L../htslib -o sift_bam_max_cov sift_bam_max_cov.cpp
