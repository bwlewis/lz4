#' lzCompress In-memory Compression and Decompression
#'
#' In-memory compression or decompression for raw vectors, cf.
#' \code{\link{memCompress}}.
#' @param from A raw or character vector to be compressed (for \code{lzCompress}), 
#' or a raw vector  to be decompressed (for \code{lzDecompress}). Character vectors will be
#'          converted to raw vectors with character strings separated by
#'          \code{"\n"} in the \code{lzCompress} case.
#' @param level An integer between 0 (fast but less compression)
#' to 9 (slow but more compression), inclusive.
#'
#' @return A raw vector representing the compressed object.
#'
#' @note The functions are compatible with the command-line lz4 format
#' for easy interoparability with other applications. See the examples.
#'
#' @examples
#' x <- head(airquality)
#' y <- lzCompress(serialize(x, NULL))
#' z <- unserialize(lzDecompress(y))
#' all.equal(x, z)
#'
#' \dontrun{
#' # Compatibility with the lz4 file format, this example assumes that
#' # the 'lz4' program is in your system PATH.
#' # Compress some text in R and save to a file:
#' compressed_file <- tempfile()
#' writeBin(lzCompress(capture.output(head(state.x77))), compressed_file)
#'
#' # Now decompress those data with command-line lz, saving to another file:
#' decompressed_file <- tempfile()
#' system2("lz4", args=c("-d", compressed_file, decompressed_file))
#'
#' # Examine the output:
#' readLines(decompressed_file, warn=FALSE)
#' }
#'
#' @seealso \code{\link{memCompress}}, \code{\link{memDecompress}}
#' @export
lzCompress <- function(from, level=0L)
{
  if (is.character(from)) 
      from <- charToRaw(paste(from, collapse = "\n"))
  else if (!is.raw(from)) 
      stop("'from' must be raw or character")
  level <- as.integer(level)
  if(is.na(level)) stop("'level' mist be an integer between 0 and 9")
  .Call("do_lzCompress", from, level)
}

#' @rdname lzCompress
#' @export
lzDecompress <- function(from)
{
  if (!is.raw(from)) 
      stop("'from' must be raw or character")
  .Call("do_lzDecompress", from)
}