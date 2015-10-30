# lz4 Data Compression/Decompression Functions 

This simple package provides in-memory data compression and decompression using
the superb lz4 library following R's `memCompress`/`memDecompress` model.

As implemented here, the lz4 functions are not quite as good at compression
as the default available R compression functions gzip, bzip2 and xz. But
compression and, especially, decompression speeds are much much faster!

Use the lz4 functions in speed-sensitive situations.

## Installation

Easiest to install with the `devtools` package:
```r
devtools::install_github("bwlewis/lz4")

library(lz4)
?lzCompress
```

## TODO

Add data streaming functions!

## Overview of the lz4 algorithms

LZ4 is lossless compression algorithm, providing compression speed at 400 MB/s
per core (0.16 Bytes/cycle). It features an extremely fast decoder, with speed
in multiple GB/s per core (0.71 Bytes/cycle).


## References

At least one other R package, the http://www.bioconductor.org/packages/release/bioc/html/gdsfmt.html
package from the Bioconductor, uses the lz4 library. However that package has a sufficiently
specialized implementation that I decided to write this much simpler and more general one anyway.

See http://www.lz4.org for more on lz4.


## Status
<a href="https://travis-ci.org/bwlewis/lz4">
<img src="https://travis-ci.org/bwlewis/lz4.svg?branch=master" alt="Travis CI status"></img>
</a>
[![codecov.io](https://codecov.io/github/bwlewis/lz4/coverage.svg?branch=master)](https://codecov.io/github/bwlewis/lz4?branch=master)
[![CRAN version](http://www.r-pkg.org/badges/version/lz4)](http://cran.rstudio.com/web/packages/lz4/index.html)
![](http://cranlogs-dev.r-pkg.org/badges/lz4)
### The code coverage is too damn low!
Because we're using a tiny fraction of the lz4 library capabilities, but checking all library lines of code. This is just an incentive to implement more of the lz4 functionality at the R level!
