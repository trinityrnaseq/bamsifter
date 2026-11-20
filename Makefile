
cwd = $(shell pwd)

CXX ?= g++

# By default, bamsifter is statically linked against the bundled htslib release.
# To link against an existing htslib installation instead (e.g. for conda packaging), set
#     HTSLIB_PREFIX=/path/to/htslib/prefix    (expects include/htslib/sam.h and lib/libhts.*)

ifdef HTSLIB_PREFIX

_sift_bam_max_cov: sift_bam_max_cov.cpp
	$(CXX) -std=c++11 -o _sift_bam_max_cov sift_bam_max_cov.cpp -Wall -O2 $(CXXFLAGS) -I$(HTSLIB_PREFIX)/include $(LDFLAGS) -L$(HTSLIB_PREFIX)/lib -Wl,-rpath,$(HTSLIB_PREFIX)/lib -lhts

else

HTSLIB_STATIC = htslib/build/lib/libhts.a

_sift_bam_max_cov: sift_bam_max_cov.cpp $(HTSLIB_STATIC)
	$(CXX) -std=c++11 -o _sift_bam_max_cov sift_bam_max_cov.cpp -Wall -O2 -I./htslib/build/include $(HTSLIB_STATIC) -lz -lbz2 -llzma -lpthread -lm

$(HTSLIB_STATIC):
	./build_htslib.sh

endif

clean:
	rm -f _sift_bam_max_cov
	rm -rf htslib

.PHONY: clean
