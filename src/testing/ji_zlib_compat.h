/**
 * JiUI - Declarative UI Framework for C/C++
 * Copyright (c) 2026 Ali A Alwahed (https://github.com/Ali-A-2026)
 *                    Hyder61112
 * Licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0)
 *
 * @file ji_zlib_compat.h
 * @brief Built-in minimal zlib replacement — zero external dependencies.
 *
 * Provides just enough of the zlib API (compress, uncompress, compressBound)
 * for the screenshot PNG I/O.  Compression uses DEFLATE stored blocks (no
 * actual compression — data is stored verbatim).  Decompression handles
 * stored blocks only, which is sufficient because our own compress() only
 * ever produces stored blocks.
 *
 * This is NOT a full zlib replacement — it covers only what JiUI needs.
 */

#ifndef JIUI_ZLIB_COMPAT_H
#define JIUI_ZLIB_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
 * zlib-compatible types and constants
 * -------------------------------------------------------------------------- */

typedef unsigned long uLong;
typedef unsigned int  uInt;

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NO_FLUSH      0
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)

/* --------------------------------------------------------------------------
 * Compression (stored DEFLATE blocks only — no actual compression)
 * -------------------------------------------------------------------------- */

/**
 * Returns an upper bound on the compressed size for `sourceLen` bytes.
 * Stored blocks: 5-byte header per 65535-byte block + 2-byte zlib wrapper
 * + 4-byte Adler-32 checksum.
 */
static inline uLong ji_compressBound(uLong sourceLen)
{
    uLong blocks = (sourceLen + 65534) / 65535;
    if (blocks == 0) blocks = 1;
    return sourceLen + blocks * 5 + 2 + 4 + 16; /* +16 for safety */
}

/**
 * Compress data into a zlib stream using stored (uncompressed) DEFLATE blocks.
 * Output: 2-byte zlib header + stored blocks + 4-byte Adler-32.
 *
 * @param dest      Output buffer (must be >= compressBound(sourceLen))
 * @param destLen   In: capacity of dest. Out: actual compressed length.
 * @param source    Input data.
 * @param sourceLen Input data length.
 * @return Z_OK on success, Z_BUF_ERROR if dest too small.
 */
static inline int ji_compress(unsigned char *dest, uLong *destLen,
                              const unsigned char *source, uLong sourceLen)
{
    if (!dest || !destLen || (!source && sourceLen > 0)) return Z_MEM_ERROR;

    uLong out_idx = 0;
    uLong cap = *destLen;

    /* zlib header: CMF=0x78 (deflate, 32K window), FLG=0x01 (check bits) */
    if (out_idx + 2 > cap) return Z_BUF_ERROR;
    dest[out_idx++] = 0x78;
    dest[out_idx++] = 0x01;

    /* Write stored DEFLATE blocks */
    uLong src_idx = 0;
    while (src_idx < sourceLen) {
        uLong chunk = sourceLen - src_idx;
        if (chunk > 65535) chunk = 65535;

        unsigned char is_final = (src_idx + chunk >= sourceLen) ? 1 : 0;

        if (out_idx + 5 > cap) return Z_BUF_ERROR;
        dest[out_idx++] = is_final; /* BFINAL + BTYPE=00 (stored) */

        /* LEN (little-endian) */
        dest[out_idx++] = (unsigned char)(chunk & 0xFF);
        dest[out_idx++] = (unsigned char)((chunk >> 8) & 0xFF);
        /* NLEN = ~LEN (little-endian) */
        dest[out_idx++] = (unsigned char)(~chunk & 0xFF);
        dest[out_idx++] = (unsigned char)((~chunk >> 8) & 0xFF);

        if (out_idx + chunk > cap) return Z_BUF_ERROR;
        memcpy(dest + out_idx, source + src_idx, chunk);
        out_idx += chunk;
        src_idx += chunk;
    }

    /* If source was empty, write a single empty final stored block */
    if (sourceLen == 0) {
        if (out_idx + 5 > cap) return Z_BUF_ERROR;
        dest[out_idx++] = 1; /* BFINAL=1, BTYPE=00 */
        dest[out_idx++] = 0; dest[out_idx++] = 0;
        dest[out_idx++] = 0xFF; dest[out_idx++] = 0xFF;
    }

    /* Adler-32 checksum of the uncompressed data (big-endian in stream) */
    uint32_t a = 1, b = 0;
    for (uLong i = 0; i < sourceLen; i++) {
        a = (a + source[i]) % 65521;
        b = (b + a) % 65521;
    }
    uint32_t adler = (b << 16) | a;

    if (out_idx + 4 > cap) return Z_BUF_ERROR;
    dest[out_idx++] = (unsigned char)((adler >> 24) & 0xFF);
    dest[out_idx++] = (unsigned char)((adler >> 16) & 0xFF);
    dest[out_idx++] = (unsigned char)((adler >> 8) & 0xFF);
    dest[out_idx++] = (unsigned char)(adler & 0xFF);

    *destLen = out_idx;
    return Z_OK;
}

/* --------------------------------------------------------------------------
 * Decompression (stored blocks only)
 * -------------------------------------------------------------------------- */

/**
 * Decompress a zlib stream containing only stored DEFLATE blocks.
 * This is sufficient to decompress output produced by ji_compress().
 *
 * @param dest      Output buffer.
 * @param destLen   In: capacity. Out: actual decompressed length.
 * @param source    Compressed zlib stream.
 * @param sourceLen Length of compressed data.
 * @return Z_OK on success, Z_DATA_ERROR on corrupt data, Z_BUF_ERROR on overflow.
 */
static inline int ji_uncompress(unsigned char *dest, uLong *destLen,
                                const unsigned char *source, uLong sourceLen)
{
    if (!dest || !destLen || (!source && sourceLen > 0)) return Z_MEM_ERROR;

    uLong src_idx = 0;
    uLong out_idx = 0;
    uLong cap = *destLen;

    /* Skip zlib header (2 bytes) */
    if (sourceLen < 2) return Z_DATA_ERROR;
    src_idx = 2;

    int final_block = 0;

    while (!final_block) {
        if (src_idx + 1 > sourceLen) return Z_DATA_ERROR;

        unsigned char hdr = source[src_idx++];
        final_block = hdr & 1;
        unsigned btype = (hdr >> 1) & 3;

        if (btype == 0) {
            /* Stored block */
            if (src_idx + 4 > sourceLen) return Z_DATA_ERROR;
            uint16_t len  = (uint16_t)source[src_idx] | ((uint16_t)source[src_idx + 1] << 8);
            uint16_t nlen = (uint16_t)source[src_idx + 2] | ((uint16_t)source[src_idx + 3] << 8);
            src_idx += 4;

            if ((uint16_t)~len != nlen) return Z_DATA_ERROR;

            if (out_idx + len > cap) return Z_BUF_ERROR;
            if (src_idx + len > sourceLen) return Z_DATA_ERROR;
            memcpy(dest + out_idx, source + src_idx, len);
            out_idx += len;
            src_idx += len;
        } else {
            /* Fixed/dynamic Huffman not supported — only stored blocks */
            return Z_DATA_ERROR;
        }
    }

    *destLen = out_idx;
    return Z_OK;
}

/* --------------------------------------------------------------------------
 * Compatibility macros — allow existing code to use zlib names unchanged
 * -------------------------------------------------------------------------- */

#define compressBound   ji_compressBound
#define compress        ji_compress
#define uncompress      ji_uncompress

#ifdef __cplusplus
}
#endif

#endif /* JIUI_ZLIB_COMPAT_H */
