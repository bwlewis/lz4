library(lz4)

# Test basic compression and decompression
data(airquality)
x <- airquality
y <- lzCompress(serialize(x,NULL))
z <- unserialize(lzDecompress(y))
if(!isTRUE(all.equal(x,z))) stop("decompressed serialized R object does not match original")

# Test character conversion
data(iris)
x <- capture.output(iris)
y <- lzCompress(x)
z <- readLines(textConnection(rawToChar(lzDecompress(y))))
if(!isTRUE(all.equal(x,z))) stop("decompressed text does not match original")

# Other compression modes
data(EuStockMarkets)
eu <- serialize(EuStockMarkets, NULL)
ans <- lapply(seq(0,9), function(j)
    object.size(lzCompress(eu, j))
  )

# Test basic compression and decompression with dummy type for good measure
data(airquality)
x <- airquality
y <- lzCompress(serialize(x,NULL), type="lz4")
z <- unserialize(lzDecompress(y, type="lz4"))
if(!isTRUE(all.equal(x,z))) stop("decompressed serialized R object does not match original")

# Test character conversion with dummy type for good measure
data(iris)
x <- capture.output(iris)
y <- lzCompress(x)
z <- readLines(textConnection(rawToChar(lzDecompress(y, type="lz4"))))
if(!isTRUE(all.equal(x,z))) stop("decompressed text does not match original")

# Other compression modes with dummy type for good measure
data(EuStockMarkets)
eu <- serialize(EuStockMarkets, NULL)
ans <- lapply(seq(0,9), function(j)
    object.size(lzCompress(eu, j, type="lz4"))
  )


# Version number
lz4_version()
