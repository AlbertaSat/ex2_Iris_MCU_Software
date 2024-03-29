/*
 * compression.c
 *
 *  Created on: Jul 18, 2022
 *      Author: Liam Droog
 *     	Adapted from: Marcus Geelnard
 *     			      https://github.com/MariadeAnton/bcl
 *      call test_compression for some compressiony tests
 */
#include "compression.h"
#include "iris_system.h"
#include "rle.h"
#include <stdlib.h>

unsigned int rle_compress(unsigned char *arr, unsigned int insize, unsigned char *outarr, uint8_t ecc) {

    unsigned int outsize, bufsize, k, err_count;
    err_count = 0;
    unsigned char *in, *out, *buf;
    /* Worst case output buffer size */
    bufsize = (insize * 104 + 50) / 100 + 384;

    /* Allocate memory */
    in = (unsigned char *)malloc(insize + 2 * bufsize);
    if (!in) {
        return;
    }

    /* copy contents to buffer */
    *in = arr;

    /* Pointers to compression buffer and output memory */
    buf = &in[insize];
    out = &buf[bufsize];
    outsize = RLE_Compress(in, buf, insize);

    if (ecc) {
        if (outsize > 0) {
            RLE_Uncompress(buf, out, outsize);
            /* Show compression result */
            iris_log("\n  Compression: Output: %d / input : %d bytes \r\n", outsize, insize);

            /* Compare input / output data */
            for (k = 0; k < insize; ++k) {
                if (in[k] != out[k]) {
                    if (err_count == 0)
                        iris_log("\n");
                    if (err_count == 30)
                        iris_log("    ...\n");
                    else if (err_count < 30) {
                        iris_log("    0x%x: 0x%x != 0x%x\n", k, out[k], in[k]);
                    }
                    ++err_count;
                }
            }
            /* Did it work? */
            if (err_count == 0) {
                iris_log(" - OK!\n");
            } else {
                iris_log("    *******************************\n");
                iris_log("    ERROR: %d faulty bytes\n", err_count);
                iris_log("    *******************************\n");
            }
        }
    }

    // TODO: fix this shit
    *outarr = *buf;
    return outsize;
}

// unsigned int rel_decompress(){
//	  RLE_Uncompress(buf, out, outsize );
// }

void _test_compression(unsigned char *arr, unsigned int insize) {

    unsigned int outsize, bufsize, k, err_count;
    err_count = 0;
    unsigned char *in, *out, *buf;
    /* Worst case output buffer size */
    bufsize = (insize * 104 + 50) / 100 + 384;

    /* Allocate memory */
    in = (unsigned char *)malloc(insize + 2 * bufsize);
    if (!in) {
        return;
    }

    /* copy contents to buffer */
    *in = arr;

    /* Pointers to compression buffer and output memory */
    buf = &in[insize];
    out = &buf[bufsize];
    outsize = RLE_Compress(in, buf, insize);
    RLE_Uncompress(buf, out, outsize);

    if (outsize > 0) {
        /* Show compression result */
        iris_log("\n  Compression: Output: %d / input : %d bytes \r\n", outsize, insize);

        /* Compare input / output data */
        for (k = 0; k < insize; ++k) {
            if (in[k] != out[k]) {
                if (err_count == 0)
                    iris_log("\n");
                if (err_count == 30)
                    iris_log("    ...\n");
                else if (err_count < 30) {
                    iris_log("    0x%x: 0x%x != 0x%x\n", k, out[k], in[k]);
                }
                ++err_count;
            }
        }
        /* Did we have success? */
        if (err_count == 0) {
            iris_log(" - OK!\n");
        } else {
            iris_log("    *******************************\n");
            iris_log("    ERROR: %d faulty bytes\n", err_count);
            iris_log("    *******************************\n");
        }
    }

    while (1) {
    }
}

void test_compression() {
    unsigned char array[] = {
        0x42, 0x4D, 0x16, 0xF8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
        0x1F, 0x01, 0x00, 0x00, 0x2D, 0x01, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF7,
        0x03, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0,
        0xFF, 0xFF, 0xC0, 0xFF, 0xFF, 0xC0, 0xFF, 0xC0, 0xFF,
    };

    _test_compression(&array, sizeof(array));
}
