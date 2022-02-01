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
 * @param[in] hspi  HAL SPI Handle
 * @return NAND_ReturnType
 */
NAND_ReturnType NAND_Init(SPI_HandleTypeDef *hspi) {
    NAND_ID dev_ID;

    /* Store the HAL SPI Handle for all transactions*/
    NAND_SPI_Init(hspi);

    /* Wait for T_POR = 1.25ms after power on */
    NAND_Wait(T_POR);

    /* Reset NAND flash during initialization. May not be necessary though (page 50) */
    if (NAND_Reset() != Ret_Success) {
        return Ret_ResetFailed;
    } else {
        /* check if device ID is same as expected */
        NAND_Read_ID(&dev_ID);
        if (dev_ID.manufacturer_ID != NAND_ID_MANUFACTURER || dev_ID.device_ID != NAND_ID_DEVICE) {
            return Ret_WrongID;
        } else {
            return Ret_Success;
        }
    }
}

/******************************************************************************
 *                              Reads and Writes
 *****************************************************************************/

/**
 * @brief Work in progress; Read an arbitrary amount of bytes from the NAND
 * 
 * @param[in] address   pointer to the NAND address
 * @param[in] length    number of bytes to read
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Read(NAND_Addr *address, uint16_t length) {
    PhysicalAddrs addr_i;
    uint8_t data[PAGE_SIZE];

    // TODO: can't just be any address. start address has to be page start. and max len must be page end.
    // handle writing between pages.
    
    /* Convert logical address to physical internal addresses to send to NAND */
    __map_logical_addr(address, &addr_i);

    NAND_Page_Read(&addr_i, length, data);

    return Ret_Success;

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
