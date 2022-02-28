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

    // TODO:
    // build bad block table
    // finally, run power on self test (POST)

    return Ret_Success;

}

/******************************************************************************
 *                              Reads and Writes
 *****************************************************************************/

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
