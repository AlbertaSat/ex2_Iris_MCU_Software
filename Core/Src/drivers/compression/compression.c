/*
 * compression.c
 *
 *  Created on: Jul 18, 2022
 *      Author: liam
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
            DBG_PUT("\n  Compression: Output: %d / input : %d bytes \r\n", outsize, insize);

            /* Compare input / output data */
            for (k = 0; k < insize; ++k) {
                if (in[k] != out[k]) {
                    if (err_count == 0)
                        DBG_PUT("\n");
                    if (err_count == 30)
                        DBG_PUT("    ...\n");
                    else if (err_count < 30) {
                        DBG_PUT("    0x%x: 0x%x != 0x%x\n", k, out[k], in[k]);
                    }
                    ++err_count;
                }
            }
            /* Did it work? */
            if (err_count == 0) {
                DBG_PUT(" - OK!\n");
            } else {
                DBG_PUT("    *******************************\n");
                DBG_PUT("    ERROR: %d faulty bytes\n", err_count);
                DBG_PUT("    *******************************\n");
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

void test_compression(unsigned char *arr, unsigned int insize) {

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
        DBG_PUT("\n  Compression: Output: %d / input : %d bytes \r\n", outsize, insize);

        /* Compare input / output data */
        for (k = 0; k < insize; ++k) {
            if (in[k] != out[k]) {
                if (err_count == 0)
                    DBG_PUT("\n");
                if (err_count == 30)
                    DBG_PUT("    ...\n");
                else if (err_count < 30) {
                    DBG_PUT("    0x%x: 0x%x != 0x%x\n", k, out[k], in[k]);
                }
                ++err_count;
            }
        }
        /* Did we have success? */
        if (err_count == 0) {
            DBG_PUT(" - OK!\n");
        } else {
            DBG_PUT("    *******************************\n");
            DBG_PUT("    ERROR: %d faulty bytes\n", err_count);
            DBG_PUT("    *******************************\n");
        }
    }

    while (1) {
    }
}
