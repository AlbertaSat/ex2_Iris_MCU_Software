/*
 * nand_types.h
 *
 *  Created on: Jun 14, 2022
 *      Author: Robert Taylor
 */

#ifndef NAND_TYPES_H_
#define NAND_TYPES_H_

#define MAGIC 0x50BAB10C

#include "nand_m79a_lld.h"

typedef struct {
    uint32_t magic;
    uint32_t id;
    uint32_t file_size;
    uint16_t start_block;
    uint8_t isfirst;
    uint8_t *file_name;
} inode_t;

typedef inode_t DIRENT;

typedef struct {
    uint8_t open; // 0 is not open
    uint8_t readonly;
    PhysicalAddrs seek;
    inode_t node;
} FileHandle_t;

typedef FileHandle_t NAND_FILE;

typedef struct {
    uint8_t open; // 0 is not open
    inode_t current;
    inode_t first;
    PhysicalAddrs seek;
} DirHandle_t;

typedef DirHandle_t NAND_DIR;

#endif /* NAND_TYPES_H_ */
