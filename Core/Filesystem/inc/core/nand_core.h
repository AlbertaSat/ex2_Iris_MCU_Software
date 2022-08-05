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
 * nand_core.h
 *
 *  Created on: Jun 14, 2022
 *      Author: robert
 */

#ifndef NAND_CORE_H_
#define NAND_CORE_H_

#include "nand_m79a_lld.h"

int NANDfs_core_format();
int NANDfs_Core_Init();
int NANDfs_core_create(FileHandle_t *handle);
int NANDfs_core_open(int fileid, FileHandle_t *file);
int NANDfs_core_write(FileHandle_t *file, int size, void *buf);
int NANDfs_core_read(FileHandle_t *file, int size, void *buf);
int NANDfs_core_close_rdonly(FileHandle_t *file);
int NANDfs_core_close_wronly(FileHandle_t *file);
int NANDfs_core_delete(uint32_t inodeid);
int NANDfs_core_erase(inode_t *inode);
int NANDfs_Core_opendir(DirHandle_t *dir);
int NANDfs_Core_nextdir(DirHandle_t *dir);

#endif /* NAND_CORE_H_ */
