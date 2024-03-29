#include <stdio.h>

#include "arducam.h"
#include "flash_cmds.h"
#include "debug.h"
#include "nandfs.h"

#define DUMP_ASCII 1

extern UART_HandleTypeDef huart1;

#ifdef DUMP_ASCII
static void uart_dump_buf(uint8_t *data, uint16_t len) {
    char digit[4];
    digit[2] = ' ';
    digit[3] = 0;
    for (int i = 0; i < len; i++) {
        digit[0] = hex_2_ascii(data[i] >> 4);
        digit[1] = hex_2_ascii(data[i] & 0x0f);
        iris_log(digit);
    }
}
#endif

static int uart_open(io_funcs_t *iofuncs, uint32_t name) {
    iofuncs->handle = &huart1;
    iris_log("JPG");
    return 0;
}

static int uart_write(io_funcs_t *iofuncs, uint8_t *data, uint16_t len) {
    // Note: we're assuming the ARM is BE
#ifndef DUMP_ASCII
    return HAL_UART_Transmit((UART_HandleTypeDef *)iofuncs->handle, data, len, 100);
#else
    uart_dump_buf(data, len);
    return 0;
#endif
}

static int uart_write_len(io_funcs_t *iofuncs, uint32_t len) {
    return HAL_UART_Transmit((UART_HandleTypeDef *)iofuncs->handle, (uint8_t *)&len, 4, 100);
    return 0;
}

static int uart_close(io_funcs_t *iofuncs) {
    iris_log("\04");
    return 0;
}

static io_funcs_t uart_driver = {.handle = &huart1,
                                 .blksz = PAGE_DATA_SIZE,
                                 .open = uart_open,
                                 .write = uart_write,
                                 .write_len = uart_write_len,
                                 .close = uart_close};

static int flash_open(struct io_funcs *iofuncs, uint32_t name) {
    NAND_FILE *fp = NANDfs_create(name);
    iofuncs->handle = fp;
    if (!fp) {
        iris_log("NAND Flash file open failed\r\n");
        return -1;
    }
    return 0;
}

static int flash_write(io_funcs_t *iofuncs, uint8_t *data, uint16_t len) {
    return NANDfs_write(iofuncs->handle, len, data);
}

static int flash_close(io_funcs_t *iofuncs) {
    NAND_ReturnType rc = NANDfs_close(iofuncs->handle);

    if (rc != Ret_Success) {
        iris_log("NAND Flash file close failed");
    }

    return rc;
}

static io_funcs_t flash_driver = {
    .blksz = PAGE_DATA_SIZE, .open = flash_open, .write = flash_write, .close = flash_close};

int transfer_image(uint8_t sensor, int32_t name, int media) {
    io_funcs_t *io_funcs;
    char msg[64];
    int rc;

    switch (media) {
    case XFER_UART:
        io_funcs = &uart_driver;
        break;
    case XFER_FLASH:
        io_funcs = &flash_driver;
        break;
    case XFER_SPI:
    default:
        return -1;
    }

    sprintf(msg, "Starting xfer to media %d\r\n", media);
    iris_log(msg);

    if (io_funcs->open(io_funcs, name))
        return -2;

    if ((rc = arducam_dump_image(sensor, io_funcs))) {
        sprintf(msg, "image dump failed, rc: %d\r\n", rc);
        iris_log(msg);
    }

    rc = io_funcs->close(io_funcs);
    return rc;
}

uint8_t fdata[PAGE_DATA_SIZE];

static int nand_dump_file(int which, io_funcs_t *io_driver) {
    int rc;
    NAND_FILE *fh = NANDfs_open(which);
    if (!fh) {
        iris_log("open failed\r\n");
        return -1;
    }

    uint32_t len = fh->node.file_size;
    if (len == 0) {
        iris_log("no files\r\n");
        NANDfs_close(fh);
        return -2;
    }

    iris_log("file id %d, length %ld\n", which, len);

    if (io_driver->write_len) {
        if ((rc = io_driver->write_len(io_driver, len))) {
            NANDfs_close(fh);
            return rc;
        }
    }

    size_t remaining = len;
    uint16_t count = PAGE_DATA_SIZE;
    while (remaining >= count) {
        if (NANDfs_read(fh, count, fdata) == -1) {
            iris_log("read failed: %d\r\n", nand_errno);
            break;
        }
        if ((rc = io_driver->write(io_driver, fdata, count))) {
            NANDfs_close(fh);
            return rc;
        }
        remaining -= count;
    }

    if (remaining > 0) {
        if (NANDfs_read(fh, remaining, fdata) == -1) {
            iris_log("residue read failed: %d\r\n", nand_errno);
        }

        if ((rc = io_driver->write(io_driver, fdata, remaining))) {
            NANDfs_close(fh);
            return rc;
        }
    }

    NANDfs_close(fh);
    return 0;
}

int transfer_file(int which, int media) {
    char msg[64];
    int rc;
    io_funcs_t *io_funcs;

    switch (media) {
    case XFER_UART:
        io_funcs = &uart_driver;
        break;
    case XFER_FLASH:
        io_funcs = &flash_driver;
        break;
    case XFER_SPI:
    default:
        iris_log("unsupported transfer media\r\n");
        return -1;
    }

    sprintf(msg, "Starting xfer to media %d\r\n", media);
    iris_log(msg);

    if (io_funcs->open(io_funcs, which))
        return -2;

    if ((rc = nand_dump_file(which, io_funcs))) {
        sprintf(msg, "image dump failed, rc: %d\r\n", rc);
        iris_log(msg);
    }

    rc = io_funcs->close(io_funcs);
    return rc;
}

static inline void char_name(uint32_t name, char *str) {
    for (int i = 0; i < 4; i++) {
        uint8_t bite = (name >> (24 - i * 8)) & 0x0ff;
        *str++ = hex_2_ascii(bite >> 4);
        *str++ = hex_2_ascii(bite & 0x0f);
    }
    *str = 0;
}

int list_files() {
    NAND_DIR *dir = NANDfs_opendir();
    if (!dir) {
        iris_log("Error opening dir, nand_errno: %d\n", nand_errno);
        return -1;
    }

    do {
        inode_t *entry = NANDfs_getdir(dir);

        iris_log("id %ld, size %ld, start %d\r\n", entry->id, entry->file_size, entry->start_block);
    } while (NANDfs_nextdir(dir) > 0);

    NANDfs_closedir(dir);
    return 0;
}

static uint8_t data[PAGE_DATA_SIZE];

int dump_page(int block, int page) {
    PhysicalAddrs paddr = {.block = block, .page = page};
    NAND_ReturnType rc;

    if ((rc = NAND_Page_Read(&paddr, PAGE_DATA_SIZE, data)) != Ret_Success) {
        iris_log("Page read <%d,%d> failed: %d\r\n", block, page, rc);
        return -1;
    }

    for (int i = 0; i < 64; i++) {
        iris_log("%02x ", data[i]);
    }
    iris_log("\r\n");

    return 0;
}

int erase_block(int block) {
    PhysicalAddrs paddr = {.block = block};
    NAND_ReturnType rc;

    if ((rc = NAND_Block_Erase(&paddr)) != Ret_Success) {
        iris_log("block erase %d failed: %d\r\n", block, rc);
        return -1;
    }

    return 0;
}
