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
 * nand_core.c
 *
 *  Created on: Jun 14, 2022
 *      Author: Robert Taylor
 */
#include <stdint.h>
#include <string.h>
#include "nandfs.h"
#include "nand_core.h"
#include "nand_m79a_lld.h"
#include "nand_types.h"
#include "nand_errno.h"

static int lowest_inode = 0;
static int highest_inode = 0;

void _increment_seek(PhysicalAddrs *addr, int size);

/*
 * Finds the first unformatted block
 * @Return:
 * 0, success, blank space found
 * -1, failed. Could be I/O or full device, No distinction made
 */
int _find_blank(PhysicalAddrs *addr) {
    PhysicalAddrs search = {0};
    inode_t node = {0};
    NAND_ReturnType status = Ret_Failed;
    for (uint16_t i = 0; i < NUM_BLOCKS; i++) {
        search.block = i;
        search.plane = i & 1;
        status = NAND_Page_Read(&search, sizeof(node), (uint8_t *)&node);
        if (status != Ret_Success) {
            continue; // Might have hit a bad block, not really sure
        }
        if (node.magic != MAGIC) {
            memcpy(addr, &search, sizeof(PhysicalAddrs));
            return 0;
        }
    }
    return -1;
}

/*
 * Finds the PhysicalAddr of the first block of an inode by its ID
 */
static int _find_inode(PhysicalAddrs *addr, int inodeid) {
    PhysicalAddrs search = {0};
    inode_t node = {0};
    NAND_ReturnType status = Ret_Failed;
    for (uint16_t i = 0; i < NUM_BLOCKS; i++) {
        search.block = i;
        search.plane = i & 1;
        status = NAND_Page_Read(&search, sizeof(node), (uint8_t *)&node);
        if (status != Ret_Success) {
            continue; // Might have hit a bad block, not really sure
        }
        if (node.id == inodeid && node.isfirst) {
            memcpy(addr, &search, sizeof(PhysicalAddrs));
            return 0;
        }
    }
    return -1;
}

int _find_lowest_inode() {
    uint32_t lowest = UINT32_MAX;
    PhysicalAddrs search = {0};
    inode_t node = {0};
    NAND_ReturnType status = Ret_Failed;
    int found_valid_file = 0; // Sanity check, if no files found, we will return 0;
    for (uint16_t i = 0; i < NUM_BLOCKS; i++) {
        search.block = i;
        search.plane = i & 1;
        status = NAND_Page_Read(&search, sizeof(node), (uint8_t *)&node);
        if (status != Ret_Success) {
            continue; // Might have hit a bad block, not really sure
        }
        if (node.magic != MAGIC) {
            continue;
        }
        found_valid_file = 1;
        if (node.id < lowest) {
            lowest = node.id;
        }
    }
    if (found_valid_file) {
        return lowest;
    }
    return 0;
}

int _find_highest_inode() {
    uint32_t highest = 0;
    PhysicalAddrs search = {0};
    inode_t node = {0};
    NAND_ReturnType status = Ret_Failed;
    int found_valid_file = 0; // Sanity check, if no files found, we will return 0;
    for (uint16_t i = 0; i < NUM_BLOCKS; i++) {
        search.block = i;
        search.plane = i & 1;
        status = NAND_Page_Read(&search, sizeof(node), (uint8_t *)&node);
        if (status != Ret_Success) {
            continue; // Might have hit a bad block, not really sure
        }
        if (node.magic != MAGIC) {
            continue;
        }
        found_valid_file = 1;
        if (node.id > highest) {
            highest = node.id;
        }
    }
    if (found_valid_file) {
        return highest;
    }
    return 0;
}

int NANDfs_Core_Init() {
    NAND_Init();

    lowest_inode = _find_lowest_inode();
    highest_inode = _find_highest_inode();
    return 0;
}

int NANDfs_core_create(FileHandle_t *handle) {
    PhysicalAddrs addr = {0};
    inode_t node;

    // First, find a blank space for the file
    int ret = _find_blank(&addr);

    while (ret != 0) { // Couldn't find any blank space. Delete the oldest file instead
        NANDfs_core_delete(lowest_inode);
        lowest_inode = _find_lowest_inode();
        ret = _find_blank(&addr);
    }
    node.magic = MAGIC;
    node.id = highest_inode + 1;
    node.isfirst = 1;
    node.file_size = UINT32_MAX; // So we don't accidentally drop some 1's to 0's and mess up the size
    NAND_ReturnType status = NAND_Page_Program(&addr, sizeof(node), (uint8_t *)&node);
    if (status != Ret_Success) {
        nand_errno = NAND_EIO;
        return -1;
    }
    _increment_seek(&addr, 2048);
    node.file_size = 0; // Set it to 0 now so the writes are accurate
    memcpy(&(handle->seek), &addr, sizeof(addr));
    memcpy(&(handle->node), &node, sizeof(node));
    handle->open = 1;
    highest_inode++;
    return 0;
}

int NANDfs_core_format() {
    PhysicalAddrs addr = {0};
    for (int i = 0; i < NUM_BLOCKS; i++) {
        addr.block = i;
        NAND_Block_Erase(&addr);
    }
    lowest_inode = 0;
    highest_inode = 0;
}

// Increments seek by one page
void _increment_seek(PhysicalAddrs *addr, int size) {
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

int NANDfs_core_delete(uint32_t inodeid) {
    PhysicalAddrs addr = {0};
    inode_t node = {0};
    int ret = _find_inode(&addr, inodeid);
    if (ret == -1) {
        nand_errno = NAND_EINVAL;
        return -1;
    }
    do {
        NAND_Block_Erase(&addr);
        addr.block++;
        if (addr.block >= NUM_BLOCKS) {
            // TODO: Add bad block skipover
            addr.block = 0;
        }
        NAND_ReturnType status = NAND_Page_Read(&addr, sizeof(node), (uint8_t *)&node);
        if (status == Ret_Failed) {
            return -1; // TODO: Add handling for bad block
        }
    } while (node.magic == MAGIC && node.id == inodeid);
    return 0;
}

/*
 * Writes happen in sizes of PAGE_DATA_SIZE unless it's the last partial page
 */
int NANDfs_core_write(FileHandle_t *file, int size, void *buf) {
    if (file->open == 0) {
        nand_errno = NAND_EINVAL;
        return -1;
    }
    if (file->readonly) {
        nand_errno = NAND_EROFS;
        return -1;
    }
    if (size == 0) {
        nand_errno = NAND_EINVAL;
        return -1;
    }
    int pages_to_write = size / PAGE_DATA_SIZE;
    if (pages_to_write == 0) {
        pages_to_write = 1;
    }
    PhysicalAddrs *seek = &(file->seek);

    for (int i = 0; i < pages_to_write; i++) {
        // If we reached the end of a block, page will be set to 0,
        // This means we must delete the file that's in the way and add our own inode
        if (seek->page == 0) {
            inode_t node;
            NAND_ReturnType status = NAND_Page_Read(seek, sizeof(node), (uint8_t *)&node);
            if (node.magic == MAGIC) {         // This is a valid inode
                if (node.id = file->node.id) { // Oh no, we hit our tail
                    nand_errno = NAND_EFBIG;
                    return -1;
                }
                NANDfs_core_delete(node.id);
            } else {
                status = NAND_Block_Erase(seek); // Erase for good measure
            }
            // Now, write our inode to the first page
            memcpy(&node, &(file->node), sizeof(node));
            node.isfirst = 0;
            status = NAND_Page_Program(seek, sizeof(node), (uint8_t *)&node);
            _increment_seek(seek, PAGE_DATA_SIZE);
        }
        NAND_ReturnType ret = NAND_Page_Program(seek, size, (uint8_t *)buf);
        if (ret == Ret_Failed) {
            nand_errno = NAND_EIO;
            return -1;
        }
        file->node.file_size += size;
        _increment_seek(seek, size);
    }
    return 0;
}
/*
 * READS happen in PAGE_DATA_SIZE right now except for the last partial page read
 */
int NANDfs_core_read(FileHandle_t *file, int size, void *buf) {
    // TODO: Add some sort of size checking
    if (file->open == 0) {
        nand_errno = NAND_EINVAL;
        return -1;
    }
    if (size == 0) {
        nand_errno = NAND_EINVAL;
        return -1;
    }
    int pages_to_read = size / PAGE_DATA_SIZE;
    if (pages_to_read == 0) {
        pages_to_read = 1;
    }
    PhysicalAddrs *seek = &(file->seek);

    for (int i = 0; i < pages_to_read; i++) {
        // If we reached the end of a block, page will be set to 0,
        // This means we must ensure we don't start reading a different file
        if (seek->page == 0) {
            inode_t node;
            NAND_ReturnType status = NAND_Page_Read(seek, sizeof(node), (uint8_t *)&node);
            if (node.magic != MAGIC) { // This is a valid inode
                nand_errno = NAND_EINVAL;
                return -1;
            }
            if (node.id != file->node.id) {
                nand_errno = NAND_EINVAL;
                return -1;
            }

            _increment_seek(seek, PAGE_DATA_SIZE);
        }
        NAND_ReturnType ret = NAND_Page_Read(seek, size, (uint8_t *)buf);
        if (ret == Ret_Failed) {
            nand_errno = NAND_EIO;
            return -1;
        }
        _increment_seek(seek, size);
    }
    return 0;
}

int NANDfs_core_open(int fileid, FileHandle_t *file) {
    PhysicalAddrs addr = {0};
    int ret = _find_inode(&addr, fileid);
    if (ret == -1) {
        nand_errno = NAND_ENOENT;
        return -1;
    }
    file->readonly = 1;
    file->open = 1;
    NAND_ReturnType status = NAND_Page_Read(&addr, sizeof(file->node), (uint8_t *)&(file->node));
    if (status != Ret_Success) {
        nand_errno = NAND_EIO;
        return -1;
    }
    _increment_seek(&addr, PAGE_DATA_SIZE);
    memcpy(&(file->seek), &addr, sizeof(addr));
    return 0;
}

int NANDfs_core_close_rdonly(FileHandle_t *file) {
    if (file->open == 0) {
        nand_errno = NAND_EBADF;
        return -1;
    }
    if (file->readonly == 0) { // This function is not meant to close a writeable file
        nand_errno = NAND_EBADF;
        return -1;
    }
    memset(file, 0, sizeof(FileHandle_t));
    return 0;
}

int NANDfs_core_close_wronly(FileHandle_t *file) {
    if (file->open == 0) {
        nand_errno = NAND_EBADF;
        return -1;
    }
    if (file->readonly == 1) {
        nand_errno = NAND_EBADF;
        return -1;
    }
    PhysicalAddrs addr = {0};
    // We are closing a file that was just created. Update its first inode with the information.
    int ret = _find_inode(&addr, file->node.id);
    if (ret == -1) {
        nand_errno = NAND_ENOENT;
        return -1;
    }
    inode_t newnode = {0};
    NAND_ReturnType status = NAND_Page_Read(&addr, sizeof(newnode), (uint8_t *)&newnode);
    newnode.file_size = file->node.file_size;
    status = NAND_Page_Program(&addr, sizeof(newnode), (uint8_t *)&newnode);
    memset(file, 0, sizeof(FileHandle_t));
    return 0;
}

int NANDfs_Core_opendir(DirHandle_t *dir) {
    // Loop through blocks on flash chip
    // Find the first inode that's defined
    // If looped to NUM_BLOCKS blocks and no inode, error
    if (dir->open) {
        nand_errno = NAND_EBADF;
        return -1;
    }
    PhysicalAddrs addr = {0};
    inode_t node = {0};
    for (int i = 0; i < NUM_BLOCKS; i++) {
        addr.block = i;
        NAND_ReturnType status = NAND_Page_Read(&addr, sizeof(node), (uint8_t *)&node);
        if (status != Ret_Success) {
            continue; // Might have hit a bad block
        }
        if (node.magic == MAGIC && node.isfirst) { // Find the first node in the flash. Don't care about order
            memcpy(&(dir->first), &node, sizeof(node));
            memcpy(&(dir->current), &node, sizeof(node));
            memcpy(&(dir->seek), &addr, sizeof(addr));
            dir->open = 1;
            return 0;
        }
    }
    nand_errno = NAND_ENOENT;
    return -1;
}

int NANDfs_Core_readdir(DirHandle_t *dir, inode_t *node) {
    // If the inode is the first inode we read, write 0 in to the node
    // else copy the inode at the current seek to the output node
    // Set the seek to the next inode in the flash
    if (!dir->open) {
        nand_errno = NAND_EBADF;
        return -1;
    }
    NAND_ReturnType status = NAND_Page_Read(&(dir->seek), sizeof(node), (uint8_t *)node);
    if (status != Ret_Success) {
        nand_errno = NAND_EIO;
        node->id = 0;
        return -1;
    }
    if (node->magic != MAGIC) {
        nand_errno = NAND_EFUBAR;
        return -1;
    }
    if (node->id == dir->first.id) {
        node->id = 0;
        return 0;
    }
    // find the next inode, if we can.
    inode_t search;
    int num_checked;
    while (num_checked++ < NUM_BLOCKS) { // Prevent infinite loop
        dir->seek.block++;
        if (dir->seek.block >= NUM_BLOCKS) {
            dir->seek.block = 0;
        }
        NAND_ReturnType status = NAND_Page_Read(&(dir->seek), sizeof(inode_t), (uint8_t *)&search);
        if (status != Ret_Success) {
            continue; // Might have hit a bad block, not sure
        }
        if (search.magic == MAGIC && search.isfirst) {
            return 0; // Stop when we find the next node, we'll leave checking it for the next call
        }
    }
    return 0;
}
