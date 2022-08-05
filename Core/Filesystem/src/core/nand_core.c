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
#include "debug.h"

static inode_t lowest_inode;
static inode_t highest_inode;

static void _increment_seek(PhysicalAddrs *addr, int size);
static void _increment_block(PhysicalAddrs *addr);
static void find_good_block(PhysicalAddrs *addr);

#define RESERVED_BLOCK_CNT 1

uint16_t _next_free_block(inode_t *inode) {
    if (inode->magic != MAGIC) {
        // freshly formatted file system, i.e. no files yet
        return RESERVED_BLOCK_CNT;
    }

    uint16_t seek_block = inode->start_block;
    int remaining_data = inode->file_size;
    // There is always one inode page per block
    int bytes_per_block = BLOCK_SIZE - PAGE_DATA_SIZE;
    int file_blocks = 0;
    while (remaining_data >= bytes_per_block) {
        remaining_data -= bytes_per_block;
        ++file_blocks; // completely filled blocks
    }
    if (remaining_data > 0) { // partially filled blocks
        ++file_blocks;
    }

    seek_block += file_blocks;
    if (seek_block >= NUM_BLOCKS) {
        seek_block = (seek_block - NUM_BLOCKS) + RESERVED_BLOCK_CNT;
    }
    return seek_block;
}

/*
 * Finds the PhysicalAddr of the first block of an inode by its ID
 */
static int _find_inode(int inodeid, inode_t *inode, PhysicalAddrs *paddr) {
    PhysicalAddrs search = {0};
    inode_t snode = {0};

    for (int i = RESERVED_BLOCK_CNT; i < NUM_BLOCKS; i++) {
        search.block = i;
        if (NAND_is_Bad_Block(search.block)) {
            continue;
        }
        if (NAND_Page_Read(&search, sizeof(snode), (uint8_t *)&snode) != Ret_Success) {
            continue; // Might have hit a bad block, not really sure
        }

        if (snode.id == inodeid && snode.isfirst) {
            *inode = snode;
            *paddr = search;
            return 0;
        }
    }
    return -1;
}

int _find_lowest_inode(inode_t *inode) {
    uint32_t lowest = UINT32_MAX;
    PhysicalAddrs search = {0};
    inode_t node = {0};
    NAND_ReturnType status = Ret_Failed;
    int found_valid_file = 0; // Sanity check, if no files found, we will return 0;
    for (uint16_t i = RESERVED_BLOCK_CNT; i < NUM_BLOCKS; i++) {
        search.block = i;
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
            *inode = node;
        }
    }
    if (found_valid_file) {
#if NAND_DEBUG
        DBG_PUT("lowest inode id %d, start %d\r\n", inode->id, inode->start_block);
#endif
        return lowest;
    }
    memset(inode, 0, sizeof(inode_t));
    return 0;
}

int _find_highest_inode(inode_t *inode) {
    uint32_t highest = 0;
    PhysicalAddrs search = {0};
    inode_t node = {0};
    NAND_ReturnType status = Ret_Failed;
    int found_valid_file = 0; // Sanity check, if no files found, we will return 0;
    for (uint16_t i = RESERVED_BLOCK_CNT; i < NUM_BLOCKS; i++) {
        search.block = i;
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
            *inode = node;
        }
    }
    if (found_valid_file) {
#if NAND_DEBUG
        DBG_PUT("highest inode id %d, start %d\r\n", inode->id, inode->start_block);
#endif
        return highest;
    }
    memset(inode, 0, sizeof(inode_t));
    return 0;
}

/*
 * Uses the highest_inode to determine where the next file should start.
 * Checks that that block is indeed erased.
 * @Return:
 * block, success - blank space found at block
 * 0, partial success - there is an old file at the next block
 * -1, failed. Could be I/O or full device, see nand_errno for details
 */
static int _find_blank(inode_t *next_inode) {
    PhysicalAddrs search = {.block = _next_free_block(&highest_inode)};
    inode_t node = {0};
    NAND_ReturnType status;
    status = NAND_Page_Read(&search, sizeof(node), (uint8_t *)&node);
    if (status != Ret_Success) {
        nand_errno = NAND_EIO;
        return -1;
    }
    if (node.magic == MAGIC) {
        *next_inode = node;
        return 0;
    }

    return search.block;
}

int NANDfs_Core_Init() {
    NAND_Init();

    _find_lowest_inode(&lowest_inode);
    _find_highest_inode(&highest_inode);
    return 0;
}

int NANDfs_core_create(FileHandle_t *handle) {
    inode_t node;
    int rc;
    PhysicalAddrs addr = {0};

    // First, find a blank space for the file
    int start_block = _find_blank(&node);

    if (start_block == -1) {
        // We can't even determine what the start block should be
        return -1;
    }
    if (start_block == 0) {
        /* _find_blank found an inode at the next file. That should mean that
         * we have wrapped around and are encountering old files. Erase the
         * block and reuse its starting block.
         */
        DBG_PUT("erasing file %d at block %d\r\n", node.id, node.start_block);

        addr.block = node.start_block;
        if ((rc = NANDfs_core_erase(&node))) {
            return rc;
        }
        if (node.id == lowest_inode.id) {
            // Just deleted the oldest file. Fine the new oldest file.
            _find_lowest_inode(&lowest_inode);
        }
    } else {
        addr.block = start_block;
        if (NAND_Block_Erase(&addr) != Ret_Success) {
            return -1;
        }
    }

    node.magic = MAGIC;
    node.id = highest_inode.id + 1;
    node.isfirst = 1;
    node.start_block = addr.block;
    node.file_size = 0; // Set it to 0 now so the writes are accurate

    _increment_seek(&addr, PAGE_DATA_SIZE); // Data starts on the next page

    handle->seek = addr;
    handle->node = node;
    handle->open = 1;

    highest_inode = node;
    if (lowest_inode.id == 0) {
        // This is the first file created, so it is both lowest and highest inode.
        lowest_inode = node;
    }
#if NAND_DEBUG
    DBG_PUT("Creating file %d at block %d\r\n", handle->node.id, handle->node.start_block);
#endif
    return 0;
}

NAND_ReturnType _NANDfs_core_erase_block(int block) {
    PhysicalAddrs addr = {0};
    addr.block = block;
    if (NAND_is_Bad_Block(block)) { // Skip bad block
        return Ret_EraseFailed;
    }
    NAND_ReturnType ret = NAND_Block_Erase(&addr);
    if (ret == Ret_EraseFailed) {
#if NAND_DEBUG
        DBG_PUT("failed to erase block %d, ret:%d\r\n", addr.block, ret);
#endif
        NAND_Mark_Bad_Block(block);
        return Ret_EraseFailed;
    }
    return ret;
}

int NANDfs_core_format() {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        _NANDfs_core_erase_block(i);
    }
    memset(&lowest_inode, 0, sizeof(lowest_inode));
    memset(&highest_inode, 0, sizeof(highest_inode));
    lowest_inode.start_block = RESERVED_BLOCK_CNT;
    highest_inode.start_block = RESERVED_BLOCK_CNT;

    return 0;
}

int NANDfs_core_erase(inode_t *inode) {
    PhysicalAddrs addr = {.block = inode->start_block};
    inode_t fnode = {0};
    do {
        _NANDfs_core_erase_block(addr.block);
        addr.block++;
        if (addr.block >= NUM_BLOCKS) {
            addr.block = RESERVED_BLOCK_CNT;
        }
        if (NAND_is_Bad_Block(addr.block)) {
            continue;
        }
        NAND_ReturnType status = NAND_Page_Read(&addr, sizeof(inode_t), (uint8_t *)&fnode);
        if (status != Ret_Success) {
            return -1;
        }
    } while (fnode.magic == MAGIC && fnode.id == inode->id);

    return 0;
}

int NANDfs_core_delete(uint32_t inodeid) {
    inode_t node = {0};
    PhysicalAddrs addr = {0};
    int ret = _find_inode(inodeid, &node, &addr);
    if (ret == -1) {
        nand_errno = NAND_EINVAL;
        return -1;
    }
    return NANDfs_core_erase(&node);
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
            NAND_ReturnType status;

            if ((status = NAND_Page_Read(seek, sizeof(node), (uint8_t *)&node))) {
                nand_errno = NAND_EIO;
                return -1;
            }
            if (node.magic == MAGIC) {          // This is a valid inode
                if (node.id == file->node.id) { // Oh no, we hit our tail
                    nand_errno = NAND_EFBIG;
                    return -1;
                }
                NANDfs_core_erase(&node);
                if (NAND_is_Bad_Block(seek->block))
                    find_good_block(seek);
            } else { // Erase for good measure
                NAND_ReturnType status = _NANDfs_core_erase_block(seek->block);
                if ((status == Ret_EraseFailed)) {
                    NAND_Mark_Bad_Block(seek->block);
                    find_good_block(seek);
                }
            }
            // Now, write our inode to the first page
            node = file->node;
            node.isfirst = 0;
#if NAND_DEBUG
            DBG_PUT("writing intermediate inode %d at <%d,%d>\r\n", node.id, seek->block, seek->page);
#endif

            if (NAND_Page_Program(seek, sizeof(inode_t), (uint8_t *)&node)) {
                nand_errno = NAND_EFUBAR;
                return -1;
            }
            _increment_seek(seek, PAGE_DATA_SIZE);
#if NAND_DEBUG
            DBG_PUT("first seek of new block: <%d,%d>\r\n", seek->block, seek->page);
#endif
        }

        if (NAND_Page_Program(seek, size, (uint8_t *)buf) != Ret_Success) {
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
            if (status != Ret_Success) {
                nand_errno = NAND_EIO;
                return -1;
            }
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
        if (ret != Ret_Success) {
            nand_errno = NAND_EIO;
            return -1;
        }
        _increment_seek(seek, size);
    }
    return 0;
}

int NANDfs_core_open(int fileid, FileHandle_t *file) {
    PhysicalAddrs addr = {0};
    inode_t node = {0};
    int ret = 0;

    if (fileid == 0) {
        /* Open the most recently created file */
        if (highest_inode.id == 0) {
            ret = -1;
        } else {
            node = highest_inode;
            addr.block = highest_inode.start_block;
        }
    } else {
        ret = _find_inode(fileid, &node, &addr);
    }
    if (ret == -1) {
        nand_errno = NAND_ENOENT;
        return -1;
    }
    file->readonly = 1;
    file->open = 1;
    file->node = node;
    if (node.start_block != addr.block) {
#if NAND_DEBUG
        DBG_PUT("file %d start block mismatch %d != %d\r\n", fileid, node.start_block, addr.block);
#endif
    }

    /* Move the current offset past the inode page */
    _increment_seek(&addr, PAGE_DATA_SIZE);
    file->seek = addr;

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

    // We are closing a file that was just created. Update its first inode with the information.
    PhysicalAddrs addr = {.block = file->node.start_block};
#if NAND_DEBUG
    DBG_PUT("closing file %d at block %d\r\n", file->node.id, addr.block);
#endif

    NAND_ReturnType status = NAND_Page_Program(&addr, sizeof(inode_t), (uint8_t *)&file->node);
    if (status != Ret_Success) {
        nand_errno = NAND_EIO;
        return -1;
    }

    if (file->node.id == highest_inode.id) {
        // Update highest inode with size, etc.
#if NAND_DEBUG
        DBG_PUT("updating highest_inode to id %d\r\n", file->node.id);
#endif
        highest_inode = file->node;
    }
    if (file->node.id == lowest_inode.id) {
        // Update lowest inode with size, etc.
#if NAND_DEBUG
        DBG_PUT("updating lowest_inode to id %d\r\n", file->node.id);
#endif
        lowest_inode = file->node;
    }

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

    if (lowest_inode.id == 0) {
        nand_errno = NAND_ENOENT;
        return -1;
    }

    dir->first = lowest_inode;
    dir->current = lowest_inode;
    dir->open = 1;

    return 0;
}

int NANDfs_Core_nextdir(DirHandle_t *dir) {
    if (!dir->open) {
        nand_errno = NAND_EBADF;
        return -1;
    }

    if (dir->current.id == highest_inode.id) {
        /* Already at the last inode - all done! */
        return 0;
    }

    /* addr should be the inode of the next file */
    PhysicalAddrs addr = {.block = _next_free_block(&dir->current)};
#if NAND_DEBUG
    DBG_PUT("nextdir: id %d, block %d, size %d; next inode at %d\r\n", dir->current.id, dir->current.start_block,
            dir->current.file_size, addr.block);
#endif
    inode_t node = {0};
    NAND_ReturnType status = NAND_Page_Read(&addr, sizeof(inode_t), (uint8_t *)&node);
    if (status != Ret_Success) {
        nand_errno = NAND_EIO;
        return -1;
    }
    if (node.magic != MAGIC || !node.isfirst) {
#if NAND_DEBUG
        DBG_PUT("no inode at %d, magic %x\r\n", addr.block, node.magic);
#endif
        nand_errno = NAND_EFUBAR;
        return -1;
    }

    dir->current = node;
    return node.id;
}

static void _increment_block(PhysicalAddrs *addr) {
    addr->block++;
    if (addr->block >= NUM_BLOCKS)
        addr->block = RESERVED_BLOCK_CNT;
}

static void find_good_block(PhysicalAddrs *addr) {
    do {
        _increment_block(addr);
    } while (NAND_is_Bad_Block(addr->block));
}

// Increments seek by one page
static void _increment_seek(PhysicalAddrs *addr, int size) {
    (void)size; // TODO: Silence warnings, size not implemented
    addr->page++;
    if (addr->page >= NUM_PAGES_PER_BLOCK) {
        addr->page = 0;
        find_good_block(addr);
    }
}
