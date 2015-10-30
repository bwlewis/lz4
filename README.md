# lz4 Data Compression/Decompression Functions 

This simple package provides in-memory data compression and decompression using
the superb lz4 library following R's `memCompress`/`memDecompress` model.

As implemented here, the lz4 functions are not quite as good at compression
as the default available R compression functions gzip, bzip2 and xz. But
compression and, especially, decompression speeds are much much faster!

Use the lz4 functions in speed-sensitive situations.

## TODO

Add data streaming functions!

## Overview of the lz4 algorithms

LZ4 is lossless compression algorithm, providing compression speed at 400 MB/s
per core (0.16 Bytes/cycle). It features an extremely fast decoder, with speed
in multiple GB/s per core (0.71 Bytes/cycle).

## References

* http://www.lz4.org


## Status
<a href="https://travis-ci.org/bwlewis/irlba">
<img src="https://travis-ci.org/bwlewis/irlba.svg?branch=master" alt="Travis CI status"></img>
</a>
[![codecov.io](https://codecov.io/github/bwlewis/irlba/coverage.svg?branch=master)](https://codecov.io/github/bwlewis/irlba?branch=master)
[![CRAN version](http://www.r-pkg.org/badges/version/irlba)](http://cran.rstudio.com/web/packages/irlba/index.html)
![](http://cranlogs-dev.r-pkg.org/badges/irlba)
