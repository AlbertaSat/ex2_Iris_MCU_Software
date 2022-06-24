/*
 * nandfs_tests.c
 *
 *  Created on: Jun. 22, 2022
 *      Author: robert
 */

#include "nandfs.h"
#include "nand_m79a_lld.h"

void set_buffer(void *buffer, int size, char c) {
	uint8_t *b = (uint8_t *)buffer;
	for (int i = 0; i < size; i++) {
		b[i] = c;
	}
}

int flash_write_to_every_block () {
	int hadfail = 0;
	PhysicalAddrs addr = {0};
	uint8_t buffer[2048];
	NANDfs_format();
	set_buffer(buffer, 2048, 0xa5);
	for (int i = 0; i < NUM_BLOCKS; i++) {
		addr.block = i;
		NAND_Page_Program(&addr, 2048, buffer);
	}
	for (int i = 0; i < NUM_BLOCKS; i++) {
		set_buffer(buffer, 2048, 0);

		addr.block = i;
		NAND_Page_Read(&addr, 2048, buffer);
		for (int j = 0; j < 2048; j++) {
			if (buffer[j] != 0xa5) {
				hadfail = 1;
			}
		}
	}
	return hadfail;
}

int nand_flash_test() {
	NANDfs_format();
	extern int sample_png_size;
    extern char sample_png;
    NAND_FILE *fd = NANDfs_create();
    int size_remaining = sample_png_size;
    char *image = &sample_png;
    while (size_remaining > 0) {
    	int size_to_write = size_remaining > 2048 ? 2048 : size_remaining;
    	NANDfs_write(fd, size_to_write, image);
    	image += size_to_write;
    	size_remaining -= size_to_write;
    }
    int file_id = fd->node.id;
	NANDfs_close(fd);
	fd = NANDfs_open(file_id);
	int file_size = fd->node.file_size;
	if (file_size != sample_png_size) {
		return -1;
	}
	char array[2048] = {0};
	size_remaining = file_size;
	image = &sample_png;
	while (size_remaining > 0) {
		int size_to_read = size_remaining > 2048 ? 2048 : size_remaining;
		NANDfs_read(fd, size_to_read, array);
		for (int i = 0; i < 2048; i++) {
			if (image[i] != array[i]) {
				continue;
			}
		}
		image += size_to_read;
		size_remaining -= size_to_read;
	}
	return 0;
}
