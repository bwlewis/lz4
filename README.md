# DEFUNCT

This package will not be maintained. Instead, use the better 'fst' package, see
http://www.fstpackage.org/, which now includes faster and better in-memory
compression methods `fst::compress_fst` and `fst::decompress_fst` covered here,
as well as several file-based compression options.

Note that neither this package nor the fst package include proper streaming
options to arbitrary connections...something someone should add to fst!

October, 2018: there were some severe internal lz4 bugs that apparently also affect the fst package. I re-activated this repository to update lz4 for those who are still using it, and will file bug reports against fst...


# lz4 Data Compression/Decompression Functions 

## Installation

Easiest to install with the `devtools` package:
```r
devtools::install_github("bwlewis/lz4")

library(lz4)
?lzCompress
```

## TODO

Add data streaming functions! ANYONE WANT TO HELP ADD THESE?

## Overview of the lz4 algorithms

LZ4 is lossless compression algorithm, providing compression speed at 400 MB/s
per core (0.16 Bytes/cycle). It features an extremely fast decoder, with speed
in multiple GB/s per core (0.71 Bytes/cycle).


## References

At least two other R packages, the
http://www.bioconductor.org/packages/release/bioc/html/gdsfmt.html package from
the Bioconductor, and https://github.com/fstPackage, use the lz4 library.
However both packages are sufficiently specialized implementation that I
decided to write this much simpler, limited package with a focus only on
compression to promote software modularity.

See http://www.lz4.org for more on lz4.


## Status
<a href="https://travis-ci.org/bwlewis/lz4">
<img src="https://travis-ci.org/bwlewis/lz4.svg?branch=master" alt="Travis CI status"></img>
</a>
<a href="https://codecov.io/gh/bwlewis/lz4/branch/master">
<img src="https://codecov.io/gh/bwlewis/lz4/branch/master/graph/badge.svg"/>
</a>

### The code coverage is too damn low!

Because we're using a tiny fraction of the lz4 library capabilities, but
checking all library lines of code. This is just an incentive to implement more
of the lz4 functionality at the R level!
