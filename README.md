# bamsifter
Normalizes coverage depth by position-specific read sampling in a bam file to give a target max depth

### Usage:

    bamsifter [-c max_coverage] [-i max_identical_cigar_pos] [-o out.bam] [--FLAGS] <in.bam>

#### Options:
    -c                    Max coverage value
    -o                    Output file name. Default is stdout
    -i                    Max number of reads with an identical cigar starting at the some position to keep
    --keep_unmapped       Keep unmapped reads (0x4 flag)
    --keep_secondary      Keep alignments flagged as secondary (0x100 flag)
    --keep_supplementary  Keep alignments flagged as supplementary (0x800 flag)
    --keep_chimeric       Keep chimeric alignments (SA: tag)

***

engineered by Christophe Georgescu @ BroadInst

