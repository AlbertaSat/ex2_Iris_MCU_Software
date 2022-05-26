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
 * @file nand_m79a.h
 * @author Tharun Suresh
 * @date 2021-12-29
 * 
 * @brief Top NAND Controller Layer 
 * 
 * This is the header file to import in projects using Micron M79A NAND ICs.
 * 
 * This layer contains functions for managing the NAND IC, its storage levels, 
 * and reading and writing to the IC. Internal functions map logical addresses 
 * to physical locations using low level drivers.
 */

#ifndef NAND_M79A_H
#define NAND_M79A_H

#include "nand_m79a_lld.h"

// TODO:
// Write higher level functions such as:
//    NAND_Write(start_addr, *buffer, length)
//    NAND_Read(start_addr, end_addr, *buffer) 
//    NAND_Erase(start_addr, end_addr)

// Enable easy memory mapping of filled and available locations
// Manage writing appropriate amount of data to each page

// Map logical addresses from 0x0 to 0xMAX_STORAGE to page numbers, blocks inside the nand IC

// Manage bad blocks, ECC and locking. 
// Possibly more difficult features such as wear leveling

/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

NAND_ReturnType __map_logical_addr(NAND_Addr *address, PhysicalAddrs *addr_struct);


/******************************************************************************
 *                              List of APIs
 *****************************************************************************/

NAND_ReturnType NAND_Init(void);

NAND_ReturnType NAND_Read (NAND_Addr *address, uint16_t length, uint8_t *buffer);
NAND_ReturnType NAND_Write(NAND_Addr *address, uint16_t length, uint8_t *buffer);

/* A simple append-only file system API */

struct inode {
    uint32_t id;
    uint32_t length;
    uint32_t sblock;
};

/* The file handle is used to keep track of the next block for I/O. last_block is used
 * to remember EOF for reads.
 */
typedef struct file_handle {
    uint32_t next_block;
    uint32_t last_block;
    uint32_t total_len;
} FileHandle_t;

NAND_ReturnType NAND_File_Format(int reformat);

FileHandle_t* NAND_File_Create(uint32_t id);
FileHandle_t* NAND_File_Open(uint32_t relative_offset);

NAND_ReturnType NAND_File_Write(FileHandle_t *fh, uint16_t length, uint8_t *buffer);
NAND_ReturnType NAND_File_Read(FileHandle_t *fh, uint16_t *length, uint8_t *buffer);

NAND_ReturnType NAND_File_Write_Close(FileHandle_t *fh);
NAND_ReturnType NAND_File_Read_Close(FileHandle_t *fh);

uint32_t NAND_File_Length(uint32_t relative_offset);

int NAND_File_List_First(uint32_t relative_offset, struct inode *inode);
int NAND_File_List_Next(int cookie, struct inode *inode);

#endif /* NAND_M79A_H */
