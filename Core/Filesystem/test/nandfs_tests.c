/*
 * nandfs_tests.c
 *
 *  Created on: Jun. 22, 2022
 *      Author: robert
 */

#include "nandfs.h"
#include "nand_m79a_lld.h"
#include <string.h>
#include "debug.h"

#define PAGE_LEN 2048

void set_buffer(void *buffer, int size, char c) {
    uint8_t *b = (uint8_t *)buffer;
    for (int i = 0; i < size; i++) {
        b[i] = c;
    }
}

// Increments seek by one page
void _increment_seek2(PhysicalAddrs *addr, int size) {
    (void)size; // TODO: Silence warnings, size not implemented
    addr->page++;
    if (addr->page >= NUM_PAGES_PER_BLOCK) {
        addr->page = 0;
        addr->block++;
        if (addr->block >= NUM_BLOCKS) {
            // TODO: Add bad block skipover
            addr->block = 0;
        }
    }
}

int flash_write_to_every_block() {
    int hadfail = 0;
    PhysicalAddrs addr = {0};
    uint8_t buffer[PAGE_LEN];
    NANDfs_format();
    set_buffer(buffer, PAGE_LEN, 0xa5);
    addr.page = 60;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        addr.block = i;
        NAND_Page_Program(&addr, PAGE_LEN, buffer);
    }
    for (int i = 0; i < NUM_BLOCKS; i++) {
        set_buffer(buffer, PAGE_LEN, 0);

        addr.block = i;
        NAND_Page_Read(&addr, PAGE_LEN, buffer);
        for (int j = 0; j < PAGE_LEN; j++) {
            if (buffer[j] != 0xa5) {
                hadfail = 1;
            }
        }
    }
    return hadfail;
}

int nand_indep_image_test() {
    NANDfs_format();
    extern int sample_png_size;
    extern char sample_png;
    PhysicalAddrs addr = {0};
    int size_remaining = sample_png_size;
    uint8_t image[PAGE_LEN];
    char *sample = &sample_png;
    addr.block = 201;
    while (size_remaining > 0) {
        int size_to_write = size_remaining > PAGE_LEN ? PAGE_LEN : size_remaining;
        memcpy(image, sample, size_to_write);
        sample += size_to_write;
        NAND_Page_Program(&addr, size_to_write, image);
        size_remaining -= size_to_write;
        _increment_seek2(&addr, 0);
    }
    memset(&addr, 0, sizeof(addr));
    uint8_t array[PAGE_LEN] = {0};
    size_remaining = sample_png_size;
    char *comp = &sample_png;
    addr.block = 201;
    while (size_remaining > 0) {
        int size_to_read = size_remaining > PAGE_LEN ? PAGE_LEN : size_remaining;
        set_buffer(array, PAGE_LEN, 0);
        NAND_Page_Read(&addr, PAGE_LEN, array);
        _increment_seek2(&addr, 0);

        for (int i = 0; i < PAGE_LEN; i++) {
            if (comp[i] != array[i]) {
                continue;
            }
        }
        comp += size_to_read;
        size_remaining -= size_to_read;
    }
    return 0;
}

int image_with_filesystem_test() {
    NANDfs_format();
    extern int sample_png_size;
    extern char sample_png;
    char *sample = &sample_png;
    NAND_FILE *fd = NANDfs_create();
    int size_remaining = sample_png_size;
    uint8_t image[PAGE_LEN];
    memcpy(image, sample, PAGE_LEN);
    sample += PAGE_LEN;
    while (size_remaining > 0) {
        int size_to_write = size_remaining > PAGE_LEN ? PAGE_LEN : size_remaining;
        NANDfs_write(fd, size_to_write, &image);
        memcpy(image, sample, size_to_write);
        sample += size_to_write;
        size_remaining -= size_to_write;
    }
    int file_id = fd->node.id;
    NANDfs_close(fd);
    fd = NANDfs_open(file_id);
    int file_size = fd->node.file_size;
    if (file_size != sample_png_size) {
        return -1;
    }
    char array[PAGE_LEN] = {0};
    size_remaining = file_size;
    char *comp = &sample_png;
    while (size_remaining > 0) {
        int size_to_read = size_remaining > PAGE_LEN ? PAGE_LEN : size_remaining;
        set_buffer(array, PAGE_LEN, 0);

        NANDfs_read(fd, size_to_read, &array);
        for (int i = 0; i < size_to_read; i++) {
            if (comp[i] != array[i]) {
                continue;
            }
        }
        comp += size_to_read;
        size_remaining -= size_to_read;
    }
    return 0;
}

uint8_t page[PAGE_DATA_SIZE];

#define NUM_PAGES 10

int pattern_with_filesystem_test(int page_cnt) {
    int rc, count;
    NAND_FILE *fd = NANDfs_create();
    if (!fd) {
        DBG_PUT("create failed: %d\r\n", nand_errno);
        return -1;
    }

    for (count = 0; count < page_cnt; count++) {
        memset(page, count, PAGE_DATA_SIZE);

        if ((rc = NANDfs_write(fd, PAGE_DATA_SIZE, page))) {
            DBG_PUT("write page %d failedr\n", count);
            break;
        }
    }

    int file_id = fd->node.id;
    if ((rc = NANDfs_close(fd))) {
        DBG_PUT("close file %d failed: %d\r\n", file_id, nand_errno);
    }

    DBG_PUT("wrote file id %d\r\n", file_id);

    fd = NANDfs_open_latest();
    if (!fd) {
        DBG_PUT("open file %d failed: %d\r\n", file_id, nand_errno);
        return -1;
    }

    int file_size = fd->node.file_size;
    if (file_size != PAGE_DATA_SIZE * page_cnt) {
        DBG_PUT("wrong len %d\r\n", file_size);
        NANDfs_close(fd);
        return -2;
    }

    for (count = 0; count < page_cnt; count++) {
        memset(page, 0, PAGE_DATA_SIZE);

        NANDfs_read(fd, PAGE_DATA_SIZE, page);

        int error_reported = 0;
        for (int i = 0; i < PAGE_DATA_SIZE; i++) {
            if (page[i] != count && !error_reported) {
                DBG_PUT("page %d: bad byte %x at offset %d\r\n", count, page[i], i);
                break;
            }
        }
    }

    NANDfs_close(fd);
    return 0;
}

int nand_flash_test() {
    // return flash_write_to_every_block ();
    return nand_indep_image_test();
    return image_with_filesystem_test();
}
