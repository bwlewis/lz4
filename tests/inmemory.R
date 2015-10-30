library(lz4)

x <- airquality
y <- lzCompress(serialize(x,NULL))
z <- deserialize(lzDecompress(y))
if(!isTRUE(all.equzl(x,z))) stop("decompressed object does not match original")
