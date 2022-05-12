/*
 * Copyright (C) 2015  University of Alberta
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

/**
 * @file nand_m79a.c
 * @author Tharun Suresh
 * @date 2021-12-29
 * 
 * @brief Top NAND Controller Layer
 * 
 * This layer contains functions for managing the NAND IC, its storage levels,
 * and reading and writing to the IC. Internal functions map logical addresses
 * to physical locations using low level drivers.
 */

#include <string.h>
#include "nand_m79a.h"

/******************************************************************************
 *                              Initialization
 *****************************************************************************/

/**
 * @brief Initializes the NAND. Steps: Reset device and check for correct device IDs.
 * @note  This function must be called first when powered on.
 * 
 * @param[in] None
 * @return NAND_ReturnType
 */
NAND_ReturnType NAND_Init(void) {
    NAND_ID dev_ID;

    NAND_Wait(T_POR);       /* Wait for T_POR = 1.25ms after power on */
    NAND_Send_Dummy_Byte(); /* Initializes SPI clock settings */

    /* Reset NAND flash after power on. May not be necessary though (page 50) */
    if (NAND_Reset() != Ret_Success) {
        return Ret_ResetFailed;
    } 

    /* check if device ID is same as expected */
    NAND_Read_ID(&dev_ID);
    if (dev_ID.manufacturer_ID != NAND_ID_MANUFACTURER || dev_ID.device_ID != NAND_ID_DEVICE) {
        return Ret_WrongID;
    }

    /* Unlock the block locks (page 38) */
    if (NAND_Set_Features(SPI_NAND_BLKLOCK_REG_ADDR, 0) != Ret_Success) {
        return Ret_Failed;
    }

    // TODO:
    // build bad block table
    // finally, run power on self test (POST)

    return Ret_Success;

}

/* A simple append-only file system. The SUPER_BLOCK keeps the start block of each file
 * in an inode, which also keeps a 4-byte user-specified file identifier. The extent of
 * each file is from the sblock to the next inode's sblock (minus 1). Files are written
 * by writing to the next PAGE_DATA_SIZE block, wrapping around to SUPER_BLOCK+1 when the
 * end of the device is reached. Reads are normally from the last file written, which is
 * at offset 0 from the head of the circular list of inodes. However, all the old files
 * remain until they are over-written, so the open API allows any inode to be specified.
 */

/* Use this block for the Super Block. Note that it may have to be changed if we wear it out! */
#define SUPER_BLOCK 0
static uint8_t Super_Block[PAGE_DATA_SIZE];

/* Use this to mark the file system as formatted. */
static const char* Magic = "NFS 0.2";
#define MAGIC_LEN 8

/* Super block contents. The inodes array is a circular list with head at next_inode. */
struct super_block {
    char magic[MAGIC_LEN];
    uint32_t inode_cnt;
    uint32_t next_inode;
    struct inode inodes[0];
};

/* The file blocks can also be viewed as an array managed as a circular list.
 * The address of the last block on the device is MAX_BLOCK_PAGE-1.
 */
#define MAX_BLOCK_PAGE (NUM_BLOCKS * NUM_PAGES_PER_BLOCK)

static inline int check_magic(const struct super_block *sb) {
    return memcmp(sb->magic, Magic, MAGIC_LEN);
}

/**
 * @brief Initialize the File System by configuring the Super Block.
 * @note  This function must be called before the File System is used for the first time.
 *
 * @param[in] reformat     force reformatting by setting this parameter to non-zero 
 * @return NAND_ReturnType Ret_Success if the Super Block was written successfully
 */
NAND_ReturnType NAND_File_Format(int reformat) {
    NAND_ReturnType rc = Ret_Success;
    PhysicalAddrs paddr = { 0 };
    struct super_block *sblock = (struct super_block *) Super_Block;
    
    paddr.rowAddr = SUPER_BLOCK;
    if ((rc = NAND_Page_Read(&paddr, PAGE_DATA_SIZE, Super_Block)) != Ret_Success) {
        return rc;
    }

    /* If the Magic number is already present, then the disk has already been formatted).
     * You can reformat the chip by setting the reformat argument to non-zero.
     */
    if (reformat == 0) {
        if (check_magic(sblock) == 0) {
            return Ret_Success;
        }
    }

    /* Initialize the super block */
    memset(Super_Block, 0, sizeof(Super_Block));
    memcpy(sblock->magic, Magic, MAGIC_LEN);

    sblock->inode_cnt = (PAGE_DATA_SIZE - sizeof(struct super_block))/sizeof(struct inode);
    /* initialize the first inode */
    int ix = 0;
    sblock->next_inode = ix;
    sblock->inodes[ix].id = ix;
    sblock->inodes[ix].sblock = SUPER_BLOCK + 1;

    /* Format the file system by writing the super block. */
    return NAND_Page_Program(&paddr, sizeof(Super_Block), Super_Block);    
}

/* Using a static FP makes the API look a bit like fopen(), but does restrict us to one open
 * file at a time.
 */
static struct file_handle fh;

/**
 * @brief A file is created by getting a FileHandle_t* that points to the next block to write.
 *
 * @param[in] id         An optional 4 byte "name" for the file.
 * @return FileHandle_t* An opaque cookie used for each write.
 */
FileHandle_t* NAND_File_Create(uint32_t id) {
    struct super_block *sblock = (struct super_block *) Super_Block;

    if (check_magic(sblock) != 0) {
        return 0; // FS is not formatted
    }

    uint32_t ix = sblock->next_inode;
    sblock->inodes[ix].id = id;
    fh.next_block = sblock->inodes[ix].sblock;
    fh.total_len = 0;

    return &fh;
}

/**
 * @brief Update the super block to remember the file location.
 *
 * @param[in] fh           The file handle returned by NAND_File_Create ()
 * @return NAND_ReturnType Ret_Success if the Super Block was written successfully
 */
NAND_ReturnType NAND_File_Write_Close(FileHandle_t *fh) {
    struct super_block *sblock = (struct super_block *) Super_Block;
    uint32_t ix = sblock->next_inode;

    /* Update the current on-flash inode with the length */
    sblock->inodes[ix].length = fh->total_len;

    /* Write the end of this file as the start block of the next file */
    ix += 1;
    if (ix >= sblock->inode_cnt) {
        ix = 0; // reached the end of the super-block - wrap around
    }
    sblock->next_inode = ix;
    sblock->inodes[ix].sblock = fh->next_block;
    sblock->inodes[ix].length = 0;
    
    /* Save the new super block contents. */
    PhysicalAddrs paddr = { .rowAddr = SUPER_BLOCK, .colAddr = 0 };
    return NAND_Page_Program(&paddr, sizeof(Super_Block), Super_Block);    
}

/**
 * @brief A file is opened for reading by finding its start block in the inode list
 *
 * @param[in] relative_offset Use the inode of the file this far back from the head
 *                            of the inode list. For example, 0 is the current head,
 *                            which is the most recent file, 1 is the one before that etc.
 * @return FileHandle_t* An opaque cookie used for each read.
 */
FileHandle_t* NAND_File_Open(uint32_t relative_offset) {
    struct super_block *sblock = (struct super_block *) Super_Block;

    if (check_magic(sblock) != 0) {
        return 0; // FS is not formatted
    }

    if (relative_offset >= sblock->inode_cnt) {
        return 0; // relative offset can't wrap around
    }

    /* Caveat emptor: don't go too far back, i.e. to uninitialized inodes! */
    int32_t ix = sblock->next_inode - 1;
    ix -= relative_offset;
    if (ix < 0) {
        ix += sblock->inode_cnt;
    }
    fh.total_len = sblock->inodes[ix].length;
    fh.next_block = sblock->inodes[ix].sblock;
    if (ix == (sblock->inode_cnt - 1)) {  // wrap around
        fh.last_block = sblock->inodes[0].sblock;
    }
    else {
        fh.last_block = sblock->inodes[ix + 1].sblock;
    }

    if (fh.next_block >= fh.last_block) {
        /* The only way we can start with the last block less than the first block
         * is if the file wraps around the end of the device. Unwrap it for now, by
         * adding MAX_BLOCK_PAGE, then adjust later when the read wraps.
         */
        fh.last_block += MAX_BLOCK_PAGE;
    }

    return &fh;
}

NAND_ReturnType NAND_File_Read_Close(FileHandle_t *fh) {
    /* We don't have to update the super block on a read, so close is just for symmetry */
    return Ret_Success;
}

/******************************************************************************
 *                              Reads and Writes
 *****************************************************************************/


/**
 * @brief Read the next block in the file
 * @note All reads should be length PAGE_DATA_SIZE except the last (partial) page
 * 
 * @param[in,out] fh        pointer to file handle returned by NAND_Open
 * @param[in,out] length    number of bytes to read - see note
 * @param[out]    buffer    pointer to the start of the data read from NAND
 * @return NAND_ReturnType  Ret_Success if the block was read successfully
 */
NAND_ReturnType NAND_File_Read(FileHandle_t *fh, uint16_t *length, uint8_t *buffer) {
    if (fh->next_block >= fh->last_block) {
        *length = 0;
        return Ret_Success; // End of File reached
    }
    if (*length > PAGE_DATA_SIZE) {
        return Ret_Failed; // Can only read one block at a time
    }

    PhysicalAddrs paddr = { .rowAddr = fh->next_block, .colAddr = 0 };
    NAND_ReturnType rc;
    if ((rc = NAND_Page_Read(&paddr, *length, buffer)) != Ret_Success) {
        return rc; // An actual read failure
    }

    fh->total_len -= *length;
    fh->next_block += 1;
    if (fh->next_block >= MAX_BLOCK_PAGE) {
        /* We've reached the end of the device. Wrap around to the beginning,
         * but step over the super block. Also re-wrap the end_block.
         */
        fh->next_block = SUPER_BLOCK + 1;
        fh->last_block -= MAX_BLOCK_PAGE;
    }
    return rc;
 }

 /**
 * @brief Write the next block in the file
 * @note All writess should be length PAGE_DATA_SIZE except the last (partial) page
 * 
 * @param[in,out] fh       pointer to file handle returned by NAND_Open
 * @param[in] length       number of bytes to write - see note
 * @param[in] buffer       pointer to the start of the data to write to NAND
 * @return NAND_ReturnType Ret_Success if the block was written successfully
 */
NAND_ReturnType NAND_File_Write(FileHandle_t *fh, uint16_t length, uint8_t *buffer) {
    PhysicalAddrs paddr = { .rowAddr = fh->next_block, .colAddr = 0 };
    NAND_ReturnType rc;

    if (length > PAGE_DATA_SIZE) {
        return Ret_Failed; // Can only write one block at a time
    }

    if ((rc = NAND_Page_Program(&paddr, length, buffer)) != Ret_Success) {
        return rc; // An actual write failure
    }

    fh->total_len += length;
    fh->next_block += 1;
    if (fh->next_block >= MAX_BLOCK_PAGE) {
        /* We've reached the end of the device. Wrap around to the beginning,
         * but step over the super block.
         */
        fh->next_block = SUPER_BLOCK + 1;
    }
    return rc;
 }


/**
 * @brief Get the length of a file
 *
 * @param[in] relative_offset Use the inode of the file this far back from the head
 *                            of the inode list. For example, 0 is the current head,
 *                            which is the most recent file, 1 is the one before that etc.
 * @return uint32_t           The length of the file specified or 0 if there was an error          
 */
uint32_t NAND_File_Length(uint32_t relative_offset) {
    struct super_block *sblock = (struct super_block *) Super_Block;

    if (check_magic(sblock) != 0) {
        return 0; // FS is not formatted
    }

    if (relative_offset >= sblock->inode_cnt) {
        return 0; // relative offset can't wrap around
    }

    /* Caveat emptor: don't go too far back, i.e. to uninitialized inodes! */
    int32_t ix = sblock->next_inode - 1;
    ix -= relative_offset;
    if (ix < 0) {
        ix += sblock->inode_cnt;
    }

    return sblock->inodes[ix].length;
}    

/**
 * @brief Get the inode of a particular file. The caller provides a struct inode
 * to fill in, and the index of the inode is returned. This index can be used as
 * input to NAND_File_List_Next to list the "next" file.
 *
 * @param[in] relative_offset Use the inode of the file this far back from the head
 *                            of the inode list. For example, 0 is the current head,
 *                            which is the most recent file, 1 is the one before that etc.
 * @param[out] inode          The address of a struct inode for the result
 * @return int                inode index - a negative number indicates error
 */
int NAND_File_List_First(uint32_t relative_offset, struct inode *inode) {
    struct super_block *sblock = (struct super_block *) Super_Block;

    if (check_magic(sblock) != 0) {
        return -2; // FS is not formatted
    }

    if (relative_offset >= sblock->inode_cnt) {
        return -3; // relative offset can't wrap around
    }

    /* Caveat emptor: don't go too far back, i.e. to uninitialized inodes! */
    int32_t ix = sblock->next_inode - 1;
    ix -= relative_offset;
    if (ix < 0) {
        ix += sblock->inode_cnt;
    }

    if (inode) {       
        *inode = sblock->inodes[ix];
    }
    if (sblock->inodes[ix].length == 0) {
        return -1;
    }
    return ix;
}    
    
/**
 * @brief Given a cookie containing the inode index from a prior call to this
 * function or NAND_File_List_First(), return the "next" file listing (inode).
 *
 * @param[in] cookie          the inode index returned by a previous list call
 * @param[out] inode          The address of a struct inode for the result
 * @return int                inode index - a negative number indicates error
 */
int NAND_File_List_Next(int cookie, struct inode *inode) {
    struct super_block *sblock = (struct super_block *) Super_Block;

    uint32_t ix;
    if (cookie == 0) {
        ix = sblock->inode_cnt - 1;
    }
    else {
        ix = cookie - 1;
    }

    if (ix == sblock->next_inode || sblock->inodes[ix].length == 0) {
        /* Either wrapped around or hit the last file */
        if (inode) {
            memset(inode, 0, sizeof(*inode));
        }
        return -1;
    }
    if (inode) {
        *inode = sblock->inodes[ix];
    }
    return ix;
}

/**
 * @brief Work in progress; Read an arbitrary amount of bytes from the NAND
 * 
 * @param[in] address   pointer to the NAND address
 * @param[in] length    number of bytes to read
 * @param[out] buffer   pointer to the start of the data read from NAND
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Read(NAND_Addr *address, uint16_t length, uint8_t *buffer) {
    
    PhysicalAddrs addr_i;
    NAND_ReturnType read_status = Ret_Success;

    /* Address to start reading has to be page start (for now)*/ 
    if ((*address % PAGE_DATA_SIZE) != 0) {
        return Ret_AddressInvalid;
    }
    
    /* Convert logical address to physical internal locations */
    __map_logical_addr(address, &addr_i);

    /* read the requested data from NAND page by page */
	/* figure out how many pages to read */
	uint8_t num_pages;
	uint16_t last_page_read_length = length % PAGE_DATA_SIZE;
	if (last_page_read_length != 0) {
		num_pages = (length / PAGE_DATA_SIZE) + 1;
	} else {
		num_pages = length / PAGE_DATA_SIZE;
		last_page_read_length = PAGE_DATA_SIZE;
	}

	/* iterate through pages */
	uint8_t page = 1;
	uint16_t read_length = PAGE_DATA_SIZE;
	NAND_Addr nand_addr = *address;

	while (page <= num_pages) {
		if (read_status != Ret_Success) {
			return Ret_ReadFailed;
		} else {
			if (page == num_pages) { // if last page, read appropriate number of bytes
				read_length = last_page_read_length;
			}
			read_status = NAND_Page_Read(&addr_i, read_length, buffer);

			/* update loop variables for next iteration */
			page += 1;
			nand_addr += PAGE_DATA_SIZE;
			buffer += PAGE_DATA_SIZE;
			__map_logical_addr(&nand_addr, &addr_i);
		}
	}
    return read_status;
}


/**
 * @brief Work in progress; Write an arbitrary amount of bytes to the NAND
 * 
 * @param[in] address   pointer to the NAND address
 * @param[in] length    number of bytes to write
 * @param[in] buffer    pointer to the data that needs to be written to NAND
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Write(NAND_Addr *address, uint16_t length, uint8_t *buffer) {
    
    PhysicalAddrs addr_i;
    NAND_ReturnType write_status = Ret_Success;

    /* Address to start writing has to be page start (for now)*/ 
    if ((*address % PAGE_DATA_SIZE) != 0) {
        return Ret_AddressInvalid;
    }
    
    /* Convert logical address to physical internal locations */
    __map_logical_addr(address, &addr_i);

    /* write the requested data to NAND page by page */
	/* figure out how many pages to write */
	uint8_t num_pages;
	uint16_t last_page_write_length = length % PAGE_DATA_SIZE;
	if (last_page_write_length != 0) {
		num_pages = (length / PAGE_DATA_SIZE) + 1;
	} else {
		num_pages = length / PAGE_DATA_SIZE;
		last_page_write_length = PAGE_DATA_SIZE;
	}

	/* iterate through pages */
	uint8_t page = 1;
	uint16_t write_length = PAGE_DATA_SIZE;
	NAND_Addr nand_addr = *address;

	while (page <= num_pages) {
		if (write_status != Ret_Success) {
			return Ret_WriteFailed;
		} else {
			if (page == num_pages) { // if last page, read appropriate number of bytes
				write_length = last_page_write_length;
			}
			write_status = NAND_Page_Program(&addr_i, write_length, buffer);

			/* update loop variables for next iteration */
			page += 1;
			nand_addr += PAGE_DATA_SIZE;
			buffer += PAGE_DATA_SIZE;
			__map_logical_addr(&nand_addr, &addr_i);
		}
	}
    return write_status;
}

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

/**
 * @brief Maps logical addresses to physical locations within the NAND
 * 
 * @param[in]   address       pointer to logical address [0x0 to 0xFLASH_SIZE_BYTES]
 * @param[out]  addr_struct   pointer to struct with row, col addresses
 * @return NAND_ReturnType 
 */
NAND_ReturnType __map_logical_addr(NAND_Addr *address, PhysicalAddrs *addr_struct) {
    addr_struct -> plane    = ADDRESS_2_PLANE(*address);
    addr_struct -> block    = ADDRESS_2_BLOCK(*address);
    addr_struct -> page     = ADDRESS_2_PAGE(*address);
    addr_struct -> rowAddr  = 0 || ((ADDRESS_2_BLOCK(*address) << ROW_ADDRESS_PAGE_BITS) | ADDRESS_2_PAGE(*address));
    addr_struct -> colAddr  = 0 || ((ADDRESS_2_PLANE(*address) << COL_ADDRESS_BITS) | ADDRESS_2_COL(*address));

    return Ret_Success;
}


NAND_ReturnType __run_POST(void) {

    // small read write test

    return Ret_Success;
}

//NAND_ReturnType __build_bad_block_table(blocktable *table) {

/*  reference: TN-29-17 */

    // /*Read for Bad blocks and set up bad block table. */
    // for (i=0; i<NAND_BLOCK_COUNT; i++) {
    //     rc = NAND_ReadPage(i*64, 2048, 1, ucPageReadBuf);/* Read Block i, Page 0, Byte 2048 */
    //     if(ucPageReadBuf[0] == 0xFF){
    //         rc = NAND_ReadPage(i*64+1, 2048, 1, ucPageReadBuf);/* Read Block i, Page 1, Byte 2048*/
    //         if(ucPageReadBuf[0] == 0xFF) {
    //             bb[i]=1;/*block is good */
    //         } else {
    //             bb[i]=0;/*block is bad */
    //         }
    //     } else {
    //         bb[i]=0;/*block is bad */
    //     }
    // }



//    return Ret_Success;
//}
