/*
 * Copyright (C) 2022  University of Alberta
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/*
 * nandfs.h
 *
 *  Created on: Jun. 13, 2022
 *      Author: Robert Taylor
 */

// The first page of a block stores the inode of the file contained in that block (63 pages)
// The inodes are not rewriteable. Once a file handle is closed, the file is no longer changeable
// Files are stored in purely sequential blocks
// When attempting to write a file that extends past a block, if the next block contains a file, it will be erased
// Implemented as a circular buffer. Will wrap around to the beginning
// Inode IDs increment from the start of the flash being formatted
// When a new file is created, it will be placed in either the first empty block found,
// or the next block will be deleted

#ifndef NANDFS_H_
#define NANDFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "nand_m79a_lld.h"
#include "nand_types.h"

#define FILEHANDLE_COUNT 2
#define DIRHANDLE_COUNT 2

extern int nand_errno;

int NANDfs_init();

int NANDfs_delete(int fileid);

NAND_FILE *NANDfs_create();

NAND_FILE *NANDfs_open(int fileid);

int NANDfs_close(NAND_FILE *file);

// Only one directory exists, but the name is used for familliarity
NAND_DIR *NANDfs_opendir();

DIRENT NANDfs_readdir(NAND_DIR *dir);

int NANDfs_closedir(NAND_DIR *dir);

int NANDfs_read(NAND_FILE *fd, int size, void *buf);

int NANDfs_write(NAND_FILE *fd, int size, void *buf);

void NANDfs_format();

#ifdef __cplusplus
}
#endif

#endif /* NANDFS_H_ */
