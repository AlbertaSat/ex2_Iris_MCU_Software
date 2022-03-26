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
 * @file nand_m79a_lld.c
 * @author Tharun Suresh
 * @date 2021-12-29
 * 
 * @brief Low level NAND Driver Layer 
 * 
 * This layer contains Low-level driver functions for reading and writing 
 * to M79a NAND Flash via SPI.
 */

#include "nand_m79a_lld.h"

/******************************************************************************
 *                              Status Operations
 *****************************************************************************/

/**
 * @brief Sends command to reset the NAND Flash chip.
 * @note Transaction length: 1 byte; Returns success when Flash is ready for further instructions.
 * 
 * @param[in] None
 * @return NAND_ReturnType
 */
NAND_ReturnType NAND_Reset(void) {

    uint8_t command = SPI_NAND_RESET;
    SPI_Params transmit = { .buffer = &command, .length = 1 };

    NAND_SPI_ReturnType SPI_Status = NAND_SPI_Send(&transmit);
    NAND_Wait(T_POR);	// wait for T_POR = 1.25 ms after reset

    if (SPI_Status != SPI_OK) {
        return Ret_ResetFailed;
    } else {
        // wait until OIP bit resets again (Flash is ready for further instructions)
        return NAND_Wait_Until_Ready();
    }
}

/**
 * @brief Waits until device is ready for further instructions
 * @note Waits until OIP bit in the Status Register resets again, indicating
 *  that the NAND Flash is ready for further instructions. If OIP = 1,
 *  operation is ongoing, i.e. device is busy.
 * 
 *  Assumption: the device keeps outputting the status register contents
 *  until another command is issued. This is shown in pages 17 and 31.
 *  TODO: confirm with an oscilloscope.
 * 
 * @param[in] None
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Wait_Until_Ready(void) {
    uint8_t timeout_counter = 0;
    uint8_t max_attempts = 100;

    /* SPI Transaction set up */
    uint8_t data_rx;
    SPI_Params rx = { .buffer = &data_rx, .length = 1 };

    /* check once if any operations in progress */
    NAND_ReturnType status = NAND_Check_Busy();

    /* if busy, keep polling for until reaching max_attempts. if still busy, return busy */
    if (status == Ret_NANDBusy) {
        while (CHECK_OIP(data_rx)) {
            if (timeout_counter < max_attempts) {
                NAND_SPI_Receive(&rx);
                timeout_counter += 1;
            } else {
                return Ret_NANDBusy;
            }
        }
    }
    return Ret_Success;
}

/**
 * @brief Send a dummy byte to NAND via SPI
 * 
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Send_Dummy_Byte(void) {
    uint8_t dummy = DUMMY_BYTE;
    
    SPI_Params tx = { .buffer = &dummy, .length = 1 };
    NAND_SPI_ReturnType status = NAND_SPI_Send(&tx);

    if (status != SPI_OK) {
        return Ret_Failed;
    } else {
        return Ret_Success;
    }
}

/******************************************************************************
 *                      Identification Operations
 *****************************************************************************/

/**
 * @brief Sends command to read manufacturer and device ID of NAND flash chip
 * @note Transaction length: 4 bytes (2 each way)
 * 
 * @param nand_ID[out]  Pointer to ID structure
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Read_ID(NAND_ID *nand_ID) {

    uint8_t data_tx[] = {SPI_NAND_READ_ID, 0}; // second byte is dummy byte
    uint8_t data_rx[2]; // data buffer for received data

    SPI_Params tx = { .buffer = data_tx, .length = 2 };
    SPI_Params rx = { .buffer = data_rx, .length = 2 };

    NAND_SPI_SendReceive(&tx, &rx);

    nand_ID -> manufacturer_ID = data_rx[0]; // second last byte from transmission
    nand_ID -> device_ID       = data_rx[1]; // last byte

    return Ret_Success;
}

/******************************************************************************
 *                              Feature Operations
 *****************************************************************************/

/**
 * @brief Returns Ret_NANDBusy if there are any operations in progress.
 * @note 
 *      Sends command to read status register bit 1 (OIP).
 *      Transaction length: 3 bytes (2 to transmit, 1 to receive)
 * 
 * @param[in] None
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Check_Busy(void) {
    uint8_t status_reg;
    
    NAND_Get_Features(SPI_NAND_STATUS_REG_ADDR, &status_reg);
    if (CHECK_OIP(status_reg)) { // if OIP bit is set
        return Ret_NANDBusy;
    } else {
        return Ret_Success;
    }
}

/**
 * @brief Read one of four registers.
 * @note
 *      The register address must be one of:
 *          SPI_NAND_BLKLOCK_REG_ADDR = 0xA0,
 *          SPI_NAND_CFG_REG_ADDR     = 0xB0,
 *          SPI_NAND_STATUS_REG_ADDR  = 0xC0,
 *          SPI_NAND_DIE_SEL_REG_ADDR = 0xD0
 * 
 *      Transaction length: 3 bytes (2 to transmit, 1 to receive)
 *
 * @param reg_addr[in]  Address of type RegisterAddr
 * @param reg[out]      Pointer to register contents
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Get_Features(RegisterAddr reg_addr, uint8_t *reg) {
    uint8_t command[] = {SPI_NAND_GET_FEATURES, reg_addr};
    SPI_Params tx = { .buffer = command, .length = 2 };
    SPI_Params rx = { .buffer = reg,     .length = 1 };

    NAND_SPI_ReturnType status = NAND_SPI_SendReceive(&tx, &rx);

    if (status == SPI_OK) {
        return Ret_Success;
    } else {
        return Ret_Failed;
    }
}

/**
 * @brief Write to one of three registers. Can not write to the status register.
 * @note
 *      The register address must be one of:
 *          SPI_NAND_BLKLOCK_REG_ADDR = 0xA0,
 *          SPI_NAND_CFG_REG_ADDR     = 0xB0,
 *          SPI_NAND_DIE_SEL_REG_ADDR = 0xD0
 * 
 *      Transaction length: 3 bytes (2 to transmit, 1 to receive)
 * 
 * @param reg_addr[in]  Address of type RegisterAddr
 * @param reg[out]      Pointer to register contents
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Set_Features(RegisterAddr reg_addr, uint8_t reg) {
    if (reg_addr == SPI_NAND_STATUS_REG_ADDR) {
        return Ret_RegAddressInvalid;
    }
    uint8_t command[] = {SPI_NAND_SET_FEATURES, reg_addr, reg};
    SPI_Params tx = { .buffer = command, .length = 3 };

    NAND_SPI_ReturnType status = NAND_SPI_Send(&tx);

    if (status == SPI_OK) {
        return Ret_Success;
    } else {
        return Ret_Failed;
    }
}


/******************************************************************************
 *                              Read Operations
 *****************************************************************************/

/**
 * @brief Read bytes stored in a page.
 * @note Command sequence:
 *          1) Send page read command to read data from page to cache
 *          2) Wait until OIP bit resets in status register
 *          3) Read data from cache
 * 
 * @param addr[in]      Pointer to PhysicalAddrs struct
 * @param length[in]    Number of bytes to read
 * @param buffer[out]   Pointer to contents read from page
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Page_Read(PhysicalAddrs *addr, uint16_t length, uint8_t *buffer) {
    
    NAND_SPI_ReturnType status;

    if (length > PAGE_DATA_SIZE) {
        return Ret_ReadFailed;
    }

    /* Command 1: PAGE READ. See datasheet page 16 for details */
    uint32_t row = addr->rowAddr;
    uint8_t command_page_read[4] = {SPI_NAND_PAGE_READ, BYTE_2(row), BYTE_1(row), BYTE_0(row)};

    SPI_Params tx_page_read = {.buffer = command_page_read, .length = 4};

    status = NAND_SPI_Send(&tx_page_read);

    if (status != SPI_OK) {
        return Ret_ReadFailed;
    }

    /* Command 2: Wait for data to be loaded into cache */
    if (NAND_Wait_Until_Ready() != Ret_Success) {
        return Ret_ReadFailed;
    }

    /* Command 3: READ FROM CACHE. See datasheet page 18 for details */
    uint32_t col = addr->colAddr;
    uint8_t command_cache_read[4] = {SPI_NAND_READ_CACHE_X1, BYTE_1(col), BYTE_0(col), DUMMY_BYTE};

    SPI_Params tx_cache_read = {.buffer = command_cache_read, .length = 4};
    SPI_Params rx_cache_read = {.buffer = buffer, .length = length};
    // TODO: Check if we can read just 2048 data bits per page. Datasheet shows 2176 bytes including the spare locations.  

    status = NAND_SPI_SendReceive(&tx_cache_read, &rx_cache_read);

    if (status != SPI_OK) {
        return Ret_ReadFailed;
    }

    return Ret_Success;
}


/******************************************************************************
 *                              Write Operations
 *****************************************************************************/

/* TODO:
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 */
/**
 * @brief Write data to a page.
 * @note Command sequence:
 *          1) WRITE ENABLE
 *          2) PROGRAM LOAD : load data into cache register
 *          3) PROGRAM EXECUTE : transfers data from cache to main array and waits until OIP bit is cleared
 *          4) WRITE DISABLE
 * 
 * @param addr[in]      Pointer to PhysicalAddrs struct
 * @param length[in]    Number of bytes to write
 * @param buffer[in]    Pointer to contents read from page
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Page_Program(PhysicalAddrs *addr, uint16_t length, uint8_t *buffer) {

    NAND_SPI_ReturnType status;

    if (length > PAGE_DATA_SIZE) {
        return Ret_WriteFailed;
    }

    /* Command 1: WRITE ENABLE */
    __write_enable();

    /* Command 2: PROGRAM LOAD. See datasheet page 30 for details */
    uint32_t col = addr->colAddr;
    uint8_t command_load[3] = {SPI_NAND_PROGRAM_LOAD_X1, BYTE_1(col), BYTE_0(col)};

    SPI_Params tx_cmd = {.buffer = command_load, .length = 3};
    SPI_Params tx_data = {.buffer = buffer, .length = PAGE_DATA_SIZE}; 
    // TODO: Check if we can write just 2048 data bits per page. Datasheet shows 2176 bytes including the spare locations.  

    status = NAND_SPI_Send_Command_Data(&tx_cmd, &tx_data);

    if (status != SPI_OK) {
        return Ret_WriteFailed;
    }

    /* Command 3: PROGRAM EXECUTE. See datasheet page 31 for details */
    uint32_t row = addr->rowAddr;
    uint8_t command_exec[4] = {SPI_NAND_PROGRAM_EXEC, BYTE_2(row), BYTE_1(row), BYTE_0(row)};

    SPI_Params exec_cmd = {.buffer = command_exec, .length = 4};

    status = NAND_SPI_Send(&exec_cmd);

    if (status != SPI_OK) {
        return Ret_WriteFailed;
    }

    /* Make sure the device is ready and then disable writes. */
    if (NAND_Wait_Until_Ready() != Ret_Success) {
        return Ret_WriteFailed;
    }

    /* Command 4: WRITE DISABLE */
    __write_disable();

    // TODO: check program fail bit in status register and return that. 

    return Ret_Success;
}


/******************************************************************************
 *                              Erase Operations
 *****************************************************************************/


/* TODO:
 * The first spare area location in each bad block contains the bad-block mark (0x00).
 * System software should initially check the first spare area location (byte 2048) for non-FFh data on
 * the first page of each block prior to performing any program or erase operations on the
 * NAND Flash device.
 *
 */

/**
 * @brief Erases an entire block (136 KB) at a time.
 * @note Command sequence:
 *          1) WRITE ENABLE
 *          2) BLOCK ERASE
 *          3) Wait for OIP bit to clear
 *          4) WRITE DISABLE
 * 
 * @param addr[in]  Pointer to struct with block number
 * @return NAND_ReturnType 
 */
NAND_ReturnType NAND_Block_Erase(PhysicalAddrs *addr) {

    /* Command 1: WRITE ENABLE */
    __write_enable();

    /* Command 2: BLOCK ERASE. See datasheet page 35 for details */
    uint32_t block = addr->block; // TODO: datasheet simply specifies block address. assuming that's the 11-bit block number padded with dummy bits. check.
    uint8_t command[4] = {SPI_NAND_BLOCK_ERASE, BYTE_2(block), BYTE_1(block), BYTE_0(block)};

    SPI_Params tx_cmd = {.buffer = command, .length = 4};
    if (NAND_SPI_Send(&tx_cmd) != SPI_OK) {
        return Ret_EraseFailed;
    }

    /* Command 3: wait for device to be ready again */
    if (NAND_Wait_Until_Ready() != Ret_Success) {
        return Ret_EraseFailed;
    }

    /* Command 4: WRITE DISABLE */
    __write_disable();

    // TODO: check erase fail bit in status register and return that

    return Ret_Success;

}


/******************************************************************************
 *                              Move Operations
 *****************************************************************************/

// NAND_ReturnType NAND_Copy_Back(NAND_Addr src_addr, NAND_Addr dest_addr) {

// }





/******************************************************************************
 *                              Lock Operations
 *****************************************************************************/








/******************************************************************************
 *                              Internal Functions
 *****************************************************************************/

/**
 * @brief Enable writing to NAND.
 * 
 * @param[in] None
 * @return NAND_SPI_ReturnType
 */
NAND_SPI_ReturnType __write_enable(void) {
    uint8_t command = SPI_NAND_WRITE_ENABLE;
    SPI_Params transmit = { .buffer = &command, .length = 1 };
    return NAND_SPI_Send(&transmit);
}

/**
 * @brief Disable writing to NAND
 * 
 * @param[in] None
 * @return NAND_SPI_ReturnType 
 */
NAND_SPI_ReturnType __write_disable(void) {
    uint8_t command = SPI_NAND_WRITE_DISABLE;
    SPI_Params transmit = { .buffer = &command, .length = 1 };
    return NAND_SPI_Send(&transmit);
}
