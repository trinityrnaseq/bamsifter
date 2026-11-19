
cwd = $(shell pwd)

CXX ?= g++

HTSLIB_STATIC = htslib/build/lib/libhts.a

_sift_bam_max_cov: sift_bam_max_cov.cpp $(HTSLIB_STATIC)
	$(CXX) -std=c++11 -o _sift_bam_max_cov sift_bam_max_cov.cpp -Wall -O2 -I./htslib/build/include $(HTSLIB_STATIC) -lz -lbz2 -llzma -lpthread -lm

$(HTSLIB_STATIC):
	./build_htslib.sh

clean:
	rm -f _sift_bam_max_cov
	rm -rf htslib

.PHONY: clean
