#' The lz4 compression library.
#'
#' The \code{lz4} package defines a \code{memCompress}-like interface
#' to the lz4 compression library written by Yann Collet. lz4 generally
#' favors speed, especially de-compression speed, over compression.
#' Use lz4 in speed-sensitive situations that can tolerate slightly
#' larger compressed objects than available with gzip.
#'
#' @name lz4
#' 
#' @useDynLib lz4
#' @seealso \code{\link{lzCompress}}, \code{\link{lzDecompress}}
#' @docType package
NULL
