
#include <stdio.h>
// #include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <map>
#include <set>
#include <utility>

#include "htslib/sam.h"
#include "htslib/bgzf.h"


enum test_op {
    READ_COMPRESSED    = 1,
    WRITE_COMPRESSED   = 2,
    READ_CRAM          = 4,
    WRITE_CRAM         = 8,
    WRITE_UNCOMPRESSED = 16,
};


void insert_or_increment(std::map<int32_t, int32_t> & pos_map, int32_t rpos) {
    auto it = pos_map.find(rpos);
    if (it != pos_map.end()) {
        ++(it->second);
    }
    else {
        pos_map.insert(std::pair<int32_t, int32_t>(rpos, 1));
    }
}


int main(int argc, char *argv[])
{

    samFile *in;  // open input alignment file
    int flag = 0;
    int clevel = -1;  // compression level
    bam_hdr_t *input_header; // alignment header
    htsFile *out;
    char modew[800];
    int exit_code = 0;
    int coverage_limit = 100;
    const char *out_name = "-";

    int c;  // for parsing input arguments

    // add option to keep only "proper pairs" (both reads mapped)
    // add option to run strand specificlly 
    // add option for BAM index loading (+ generation if needed) to be able to run each chromosome on a seperate thread

    // while ((c = getopt(argc, argv, "DSIt:i:bCul:o:N:BZ:@:M")) >= 0) {
    while ((c = getopt(argc, argv, "c:o:")) >= 0) {
        switch(c) {
        // case 'b': ; break;
        case 'c': coverage_limit = atoi(optarg); break;
        case 'o': out_name = optarg; break;
        }
    }

    if (argc == optind) {  // missing input file, print help
        fprintf(stderr, "Usage: sift_bam_max_cov [-c value] <in.bam>|<in.bam>\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "-c: Max coverage value.\n");
        fprintf(stderr, "-o: Output file name. Default is to stdout.\n");
        fprintf(stderr, "File to process.\n");
        fprintf(stderr, "\n");
        return (1);
    }

    in = sam_open(argv[optind], "r");


    const htsFormat *in_format = hts_get_format(in);

    // Enable multi-threading (only effective when the library was compiled with -DBGZF_MT)
    // bgzf_mt(in, n_threads, 0);

    if (in == NULL) {
        fprintf(stderr, "Error opening \"%s\"\n", argv[optind]);
        return EXIT_FAILURE;
    }

    input_header = sam_hdr_read(in);
    if (input_header == NULL) {
        fprintf(stderr, "Couldn't read header for \"%s\"\n", argv[optind]);
        return EXIT_FAILURE;
    }

    strcpy(modew, "w");
    if (clevel >= 0 && clevel <= 9) sprintf(modew + 1, "%d", clevel);
    if (flag & WRITE_CRAM) strcat(modew, "c");
    else if (flag & WRITE_COMPRESSED) strcat(modew, "b");
    else if (flag & WRITE_UNCOMPRESSED) strcat(modew, "bu");

    // out = hts_open(out_name, modew);
    out = hts_open_format(out_name, modew, in_format);
    if (out == NULL) {
        fprintf(stderr, "Error opening standard output\n");
        return EXIT_FAILURE;
    }

    if (sam_hdr_write(out, input_header) < 0) {  // write header from input to output
        fprintf(stderr, "Error writing output header.\n");
        exit_code = 1;
    }


    // map for start/end positions of alignments that are already selected
    // std::map<uint_fast32_t, int32_t> starts;
    // std::map<uint_fast32_t, int32_t> ends;
    std::map<int32_t, int32_t> starts;
    std::map<int32_t, int32_t> ends;

    // keep a set of mate reads we decided to keep when encountering the first read
    // std::unordered_set<std::string>
    // std::set<std::pair<std::string, int32_t>> mates_to_keep = std::set<std::pair<std::string, int32_t>>();
    std::set<std::string> mates_to_keep[input_header->n_targets];

    bam1_t *aln = bam_init1(); //initialize an alignment
    int32_t current_rname_index; // index compared to header: input_header->target_name[current_rname_index]
    int32_t current_coverage = 0;
    // uint_fast32_t current_pos = 0;
    int32_t current_pos = 0;

    while(sam_read1(in, input_header, aln) > 0) {

        if (current_rname_index != aln->core.tid) {
            // should have finished writing reads from current_rname_index contig, so can just reset vars
            current_coverage = 0;
            // starts = std::map<uint_fast32_t, int>();
            starts.clear();
            // ends = std::map<uint_fast32_t, int>();
            ends.clear();
            // mates_to_keep.clear();

            current_rname_index = aln->core.tid;
        }

        // make sure the read is mapped
        if ((aln->core.flag & BAM_FUNMAP) != 0)
            continue;

        // make sure the alignment is not a secondary alignment or a supplementary alignment
        if ((aln->core.flag & BAM_FSECONDARY) != 0 or (aln->core.flag & BAM_FSUPPLEMENTARY) != 0)
            continue;

        if (current_pos != aln->core.pos) { // left most position, does NOT need adjustment for reverse strand if summing their coverage
            // while((aln->core.pos > starts.begin()->first) && (aln->core.pos > ends.begin()->first)) {
            //     if (starts.begin()->first <= ends.begin()->first) {
            //         current_coverage += starts.begin()->second;
            //         starts.erase(starts.begin());
            //     }
            //     else {
            //         current_coverage -= ends.begin()->second;
            //         ends.erase(ends.begin()):
            //     }
            // }

            // add the range we want and then erase all the matching entries at once rather than 1 by 1
            auto it = starts.begin();
            if (it->first <= aln->core.pos) {
                for (; it != starts.end(); ++it) {
                    if (it->first <= aln->core.pos) { // or equal because already selected reads take priority in coverage
                        current_coverage += it->second;
                    }
                    else break;
                }
                starts.erase(starts.begin(), it);
            }

            it = ends.begin();
            if (it->first <= aln->core.pos) {
                for (; it != ends.end(); ++it) {
                    if (it->first <= aln->core.pos) { // or equal because already selected reads take priority in coverage
                        current_coverage -= it->second;
                    }
                    else break;
                }
                ends.erase(ends.begin(), it);
            }
        }

        // if we are below the max coverage or the read has already been selected to keep through its pair
        if ((current_coverage < coverage_limit) || 
            (mates_to_keep[aln->core.tid].find(bam_get_qname(aln)) != mates_to_keep[aln->core.tid].end())) {
            // get cigar
            uint32_t *cigar = bam_get_cigar(aln);

            int32_t rpos = aln->core.pos;  // update position on the ref with cigar
            for (uint32_t k = 0; k < aln->core.n_cigar; ++k) {

                if ((bam_cigar_type(bam_cigar_op(cigar[k]))&2)) {  // consumes reference
                    if (bam_cigar_op(cigar[k]) == BAM_CREF_SKIP) {
                        insert_or_increment(ends, rpos);

                        rpos += bam_cigar_oplen(cigar[k]);

                        insert_or_increment(starts, rpos);

                    }
                    else {
                        rpos += bam_cigar_oplen(cigar[k]);
                    }
                }
            }

            insert_or_increment(ends, rpos);
            ++current_coverage;

            // TODO save pair mate
            // store just qname in a set, sets are per target id
            mates_to_keep[aln->core.tid].insert(bam_get_qname(aln));
            
            // TODO output the alignment
            if (sam_write1(out, input_header, aln) == -1) {
                fprintf(stderr, "Could not write selected record \"%s\"\n", bam_get_qname(aln));
                return EXIT_FAILURE;
            }

        }

        // if (aln->flag & BAM_FREVERSE) { // if reverse strand aln
        // }
        // aln->data->
    }

    int ret;
    
    ret = hts_close(in);
    if (ret < 0) {
        fprintf(stderr, "Error closing input.\n");
        exit_code = EXIT_FAILURE;
    }

    ret = hts_close(out);
    if (ret < 0) {
        fprintf(stderr, "Error closing output.\n");
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}