/*
 * Simple LZ4 R interface
 * Based on the LZ4 fast compression/decompression library by Yann Collet.
 * Copyright (C) 2015, Bryan W Lewis (Simple R interface)
 * Copyright (C) 2011-2015, Yann Collet (LZ4 library).
 *
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <R.h>
#define USE_RINTERNALS
#include <Rinternals.h>

#include "xxhash.h"
#include "lz4frame.h"
#include "lz4.h"


/* NOTE! We replicate the otherwise internal LZ4F_dctx_s structure
 * here for arcane memory management issues further described below
 * in the do_lzDecompress function. IF YOU UPDATE THE VERSION OF
 * lz4 USED BY THIS PACKAGE BEWARE OF THIS AND ALSO UPDATE THIS!
 */
typedef struct LZ4F_dctx_s
{   
    LZ4F_frameInfo_t frameInfo;
    uint32_t version;
    uint32_t dStage;
    uint64_t frameRemainingSize;
    size_t maxBlockSize;
    size_t maxBufferSize;
    const char* srcExpect;
    char*  tmpIn;
    size_t tmpInSize;
    size_t tmpInTarget;
    char*  tmpOutBuffer;
    const char*  dict;
    size_t dictSize;
    char*  tmpOut;
    size_t tmpOutSize;
    size_t tmpOutStart;
    XXH32_state_t xxh;
    char   header[16];
} LZ4F_dctx_t;


/* lz4_compress support function
 * @param src constant input memory buffer of size size to be compressed
 * @param srz_size input memory buffer size
 * @param dest ouput memory buffer of size size in which to place results
 * @param dest_size pointer to a variable that contains the size of dest on input
 *        and is updated to contain the size of compressed data in dest on out
 * @param level compression level from 1 (low/fast) to 9 (high/slow)
 * Returns an error code, 0 for success non-zero otherwise.
 */
int
lz4_compress (void *src, size_t src_size, void *dest, size_t * dest_size,
              int level)
{
  size_t n, len = 0;
  LZ4F_errorCode_t err;
  LZ4F_preferences_t prefs;
  LZ4F_compressionContext_t ctx;
  int retval = 0;

  /* Set compression parameters */
  memset (&prefs, 0, sizeof (prefs));
  prefs.autoFlush = 1;
  prefs.compressionLevel = level;       /* 0...16 */
  prefs.frameInfo.blockMode = 0;        /* blockLinked, blockIndependent ; 0 == default */
  prefs.frameInfo.blockSizeID = 0;      /* max64KB, max256KB, max1MB, max4MB ; 0 == default */
  prefs.frameInfo.contentChecksumFlag = 1;      /* noContentChecksum, contentChecksumEnabled ; 0 == default  */
  prefs.frameInfo.contentSize = (long long)src_size;  /* for reference */

  /* create context */
  err = LZ4F_createCompressionContext (&ctx, LZ4F_VERSION);
  if (LZ4F_isError (err))
    return -1;

  n = LZ4F_compressFrame (dest, *dest_size, src, src_size, &prefs);
  if (LZ4F_isError (n))
    {
      retval = -2;
      goto cleanup;
    }
  /* update the compressed buffer size */
  *dest_size = n;

cleanup:
  if (ctx)
    {
      err = LZ4F_freeCompressionContext (ctx);
      if (LZ4F_isError (err))
        return -5;
    }

  return retval;
}


SEXP
do_lzCompress (SEXP FROM, SEXP LEVEL)
{
  char *buf;
  int ret;
  SEXP ans;
  int level = *(INTEGER(LEVEL));
  size_t input_size = (size_t) XLENGTH(FROM);
  size_t buffer_size = input_size*1.1 + 100;
  if(TYPEOF(FROM) != RAWSXP) error("'from' must be raw or character");
  buf = (char *) R_alloc(buffer_size, sizeof(char));
  ret = lz4_compress((void *)RAW(FROM), input_size, (void *)buf, &buffer_size, level);
  if(ret<0) error("internal error %d in lz4_compress",ret);
  ans = allocVector(RAWSXP, buffer_size);
  memcpy(RAW(ans), buf, buffer_size);
  return ans;
}


SEXP
lz4_version()
{
  return ScalarInteger(LZ4_versionNumber());
}

SEXP
do_lzDecompress (SEXP FROM)
{
  SEXP ANS;
  LZ4F_decompressionContext_t ctx;

  LZ4F_frameInfo_t info;
  char *from;
  void *src;
  size_t m, n, output_size, input_size = (size_t) XLENGTH(FROM);
  if(TYPEOF(FROM) != RAWSXP) error("'from' must be raw or character");
  from = (char *)RAW(FROM);

/* An implementation following the standard API would do this:
 *   LZ4F_errorCode_t err = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
 *   if (LZ4F_isError (err)) error("could not create LZ4 decompression context");
 *   ...
 *   LZ4F_freeDecompressionContext(ctx);
 * The problem with that approach is that LZ4F_createDecompressionContext
 * allocates memory with calloc internally. Later, if R's allocVector fails,
 * for example, or if R interrupts this function somewhere in '...' then
 * those internal allocations in LZ4F_createDecompressionContext leak--that is
 * they aren't ever de-allocated.
 *
 * We explicitly allocat the LZ4F_decompressionContext_t pointer using a
 * replica of the internal LZ4F_dctx_t structure defined near the top of this
 * file, see the note and corresponding warning! We allocate on the heap (via
 * R_alloc) here instead of a seemingly simpler stack allocation because LZ4
 * indicates that the address needs to be aligned to 8-byte boundaries which is
 * provided by R_alloc, see:
 * https://cran.r-project.org/doc/manuals/r-release/R-exts.html#Transient-storage-allocation
 */
  LZ4F_dctx_t *dctxPtr = (LZ4F_dctx_t *)R_alloc(1, sizeof(LZ4F_dctx_t));
  memset(dctxPtr, 0, sizeof(LZ4F_dctx_t));
  dctxPtr->version = LZ4F_VERSION;
  ctx = (LZ4F_decompressionContext_t)dctxPtr;
  m   = input_size;
  n   = LZ4F_getFrameInfo(ctx, &info, (void *)from, &input_size);
  if (LZ4F_isError (n)) error("LZ4F_getFrameInfo");
  src = from + input_size; // lz4 frame header offset
  output_size = (size_t)info.contentSize; 
  ANS = allocVector(RAWSXP, output_size);

  input_size = m - input_size;
  n = LZ4F_decompress(ctx, RAW(ANS), &output_size, src, &input_size, NULL); 
  if (LZ4F_isError (n)) error("LZ4F_decompress");

  return ANS;
}
