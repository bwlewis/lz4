#include <stdio.h>
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

#define lzframe_chunksize 16777216  // 2^24 bytes

SEXP
do_lzCompress (SEXP FROM, SEXP LEVEL)
{
  char *buf;
  SEXP ans;
  int level = *(INTEGER(LEVEL));
  size_t input_size, buffer_size, ret;
  LZ4F_preferences_t prefs;

  if (TYPEOF(FROM) != RAWSXP) error("'from' must be raw or character");
  input_size = (size_t) xlength(FROM);

  /* Set compression parameters */
  memset (&prefs, 0, sizeof (prefs));
  prefs.autoFlush = 1;
  prefs.compressionLevel = level;       /* 0...16 */
  prefs.frameInfo.blockMode = 0;        /* blockLinked, blockIndependent ; 0 == default */
  prefs.frameInfo.blockSizeID = 0;      /* max64KB, max256KB, max1MB, max4MB ; 0 == default */
  prefs.frameInfo.contentChecksumFlag = 1;      /* noContentChecksum, contentChecksumEnabled ; 0 == default  */
  prefs.frameInfo.contentSize = (long long)input_size;  /* for reference */

  buffer_size = LZ4F_compressFrameBound(input_size, &prefs);
  buf = (char *) R_alloc(buffer_size, sizeof(char));

  ret = LZ4F_compressFrame (buf, buffer_size, (char *)RAW(FROM), input_size, &prefs);
  if (LZ4F_isError(ret)) error("internal error %d in lz4_compress", ret);
  if (ret > buffer_size) error("lz4 compression buffer size mismatch");
  ans = allocVector(RAWSXP, ret);
  memcpy(RAW(ans), buf, ret);
  return ans;
}


SEXP
lz4_version()
{
  return ScalarInteger(LZ4_versionNumber());
}

static size_t get_block_size(const LZ4F_frameInfo_t* info) {
    switch (info->blockSizeID) {
        case LZ4F_default:
        case LZ4F_max64KB:  return 1 << 16;
        case LZ4F_max256KB: return 1 << 18;
        case LZ4F_max1MB:   return 1 << 20;
        case LZ4F_max4MB:   return 1 << 22;
        default: error ("Impossible with expected frame specification (<=v1.6.1)\n");
            exit(1);
    }
}


SEXP
do_lzDecompress (SEXP FROM)
{
  SEXP ANS;
  LZ4F_dctx* dctx;
  LZ4F_frameInfo_t info;
  char *from;
  char *ans;
  void *src;
  size_t m, n, output_size, input_size = xlength(FROM);
  size_t ibuf, obuf, icum, ocum;
  if(TYPEOF(FROM) != RAWSXP) error("'from' must be raw or character");
  from = (char *)RAW(FROM);

  { size_t const err = LZ4F_createDecompressionContext(&dctx, LZ4F_VERSION);
    if (LZ4F_isError(err)) error("LZ4F_createDecompressionContext");
  }
  m   = input_size;
  { size_t const err   = LZ4F_getFrameInfo(dctx, &info, (void *)from, &input_size);
    if (LZ4F_isError (err)) error("LZ4F_getFrameInfo");
  }

  output_size = (size_t) info.contentSize; 
  if (output_size == 0) error ("lzDecompress currently requires lz4 compressions with --content-size option");
  ANS = allocVector(RAWSXP, output_size);
  ans = (char *)RAW(ANS);
  obuf = get_block_size(&info);

  src = from + input_size; // lz4 frame header offset
  input_size = m - input_size;
  icum = 0;
  ibuf = lzframe_chunksize;
  if(ibuf > input_size) ibuf = input_size;
  ocum = 0;
  obuf = output_size;

  for(;;)
  {
    n = LZ4F_decompress(dctx, ans, &obuf, src, &ibuf, NULL); 
    if (LZ4F_isError (n)) error("LZ4F_decompress");
    icum = icum + ibuf;
    ocum = ocum + obuf;
    if(icum >= input_size || ibuf == 0) break;
    ans = ans + obuf;
    src = src + ibuf;
    ibuf = lzframe_chunksize;
    if(ibuf > (input_size - icum)) ibuf = input_size - icum;
    obuf = output_size - ocum;
  }
  LZ4F_freeDecompressionContext(dctx);

  return ANS;
}
