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
 * nandfs.c
 *
 *  Created on: Jun 14, 2022
 *      Author: Robert Taylor
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "nandfs.h"
#include "nand_errno.h"
#include "nand_m79a_lld.h"
#include "nand_core.h"

int nand_errno = 0;

FileHandle_t handles[FILEHANDLE_COUNT];

/*
 * Private function. Returns pointer to unopened file handle
 */
static FileHandle_t *_get_handle() {
    for (int i = 0; i < FILEHANDLE_COUNT; i++) {
        if (handles[i].open) {
            continue;
        } else {
            return &(handles[i]);
        }
    }
    return -1;
}

int NANDfs_init() { NANDfs_Core_Init(); }

int NANDfs_delete(int fileid) { return NANDfs_core_delete(fileid); }

NAND_FILE *NANDfs_create() {
    FileHandle_t *handle = _get_handle();
    if (handle == -1) {
        nand_errno = NAND_EMFILE;
        return -1;
    }
    int ret = NANDfs_core_create(handle);
    if (ret == -1) {
        return -1;
    }
    return handle;
}

/*
 * Close cements the file in the filesystem
 * It will write the size of the file to its start inode
 * The file will no longer be available for writing
 */
int NANDfs_close(NAND_FILE *file) {
    FileHandle_t *fd = (FileHandle_t *)file;
    if (file->readonly) {
        return NANDfs_core_close_rdonly(fd);
    } else {
        return NANDfs_core_close_wronly(fd);
    }
}

/*
 * Open a file for reading or appending
 */
NAND_FILE *NANDfs_open(int fileid) {
    FileHandle_t *handle = _get_handle();
    if (NANDfs_core_open(fileid, handle) == -1) {
        return (NAND_FILE *)-1;
    }
    return handle;
}

int NANDfs_opendir() {
    nand_errno = NAND_ENOSYS;
    return -1;
}

DIRENT NANDfs_readdir(int dd) {
    nand_errno = NAND_ENOSYS;
    return (DIRENT)-1;
}

int NANDfs_closedir(DIRENT dir) {
    nand_errno = NAND_ENOSYS;
    return -1;
}

int NANDfs_read(NAND_FILE *fd, int size, void *buf) {
    FileHandle_t *file = fd;
    return NANDfs_core_read(file, size, buf);
}

int NANDfs_write(NAND_FILE *fd, int size, void *buf) {
    FileHandle_t *file = (FileHandle_t *)fd;
    return NANDfs_core_write(file, size, buf);
}

int NANDfs_format() { return NANDfs_core_format(); }

#ifdef __cplusplus
}
#endif
