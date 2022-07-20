
/*
 * tar.c
 *
 *  Created on: Jul. 20, 2022
 *      Author: liam
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "microtar.h"

void test_tar_write() {
    // writing
    mtar_t tar;
    /* Open archive for writing */
    mtar_open(&tar, "test.tar", "w");

    struct stat st;

    // image 1
    stat("09-26-10.jpg", &st);

    // get image file
    FILE *img;
    img = fopen("09-26-10.jpg", "rb");

    // initialize array
    char *data = (char *)malloc(st.st_size);

    // read data into array
    size_t rtn = fread(data, 1, st.st_size, img);
    mtar_write_file_header(&tar, "image1.jpg", st.st_size);
    mtar_write_data(&tar, data, st.st_size);
    fclose(img);

    // image 2
    stat("meme.png", &st);

    // get image file
    img = fopen("meme.png", "rb");
    free(data);
    // initialize array
    data = (char *)malloc(st.st_size);

    // read data into array
    rtn = fread(data, 1, st.st_size, img);
    mtar_write_file_header(&tar, "image2.png", st.st_size);
    mtar_write_data(&tar, data, st.st_size);
    fclose(img);
    /* Finalize -- this needs to be the last thing done before closing */
    mtar_finalize(&tar);

    /* Close archive */
    mtar_close(&tar);
}

void test_tar_read() {
    mtar_header_t h;
    char *p;

    /* Open archive for reading */
    mtar_open(&tar, "test.tar", "r");

    /* Print all file names and sizes */
    while ((mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD) {
        printf("%s (%d bytes)\n", h.name, h.size);
        mtar_next(&tar);
    }

    /* Load and print contents of file "test.txt" */
    mtar_find(&tar, "test.txt", &h);
    p = calloc(1, h.size + 1);
    mtar_read_data(&tar, p, h.size);
    printf("%s", p);
    free(p);

    /* Close archive */
    mtar_close(&tar);
}
