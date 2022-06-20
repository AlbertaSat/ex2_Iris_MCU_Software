#include "arducam.h"
#include "flash_cmds.h"
#include "debug.h"
#include "nandfs.h"

#define DUMP_ASCII 1

#ifdef DUMP_ASCII
static void uart_dump_buf(uint8_t *data, uint16_t len) {
    char digit[4];
    digit[2] = ' ';
    digit[3] = 0;
    for (int i = 0; i < len; i++) {
        digit[0] = hex_2_ascii(data[i] >> 4);
        digit[1] = hex_2_ascii(data[i] & 0x0f);
        DBG_PUT(digit);
    }
}
#endif

static int uart_open(io_funcs_t *iofuncs, uint32_t name) {
    iofuncs->handle = &huart1;
    DBG_PUT("JPG");
    return 0;
}

static int uart_write(io_funcs_t *iofuncs, uint8_t *data, uint16_t len) {
    // Note: we're assuming the ARM is BE
#ifndef DUMP_ASCII
    return HAL_UART_Transmit((UART_HandleTypeDef *)iofuncs->handle, data, len, 100);
#else
    uart_dump_buf(data, len);
#endif
}

static int uart_write_len(io_funcs_t *iofuncs, uint32_t len) {
    return HAL_UART_Transmit((UART_HandleTypeDef *)iofuncs->handle, (uint8_t *)&len, 4, 100);
    return 0;
}

static int uart_close(io_funcs_t *iofuncs) {
    DBG_PUT("\04");
    return 0;
}

static io_funcs_t uart_driver = {.handle = &huart1,
                                 .blksz = PAGE_DATA_SIZE,
                                 .open = uart_open,
                                 .write = uart_write,
                                 .write_len = uart_write_len,
                                 .close = uart_close};

static int flash_open(struct io_funcs *iofuncs, uint32_t name) {
    FileHandle_t *fp = NAND_File_Create(name);
    iofuncs->handle = fp;
    if (!fp) {
        DBG_PUT("NAND Flash file open failed\r\n");
        return -1;
    }
    return 0;
}

static int flash_write(io_funcs_t *iofuncs, uint8_t *data, uint16_t len) {
    return NAND_File_Write(iofuncs->handle, len, data);
}

static int flash_close(io_funcs_t *iofuncs) {
    NAND_ReturnType rc = NAND_File_Write_Close(iofuncs->handle);

    if (rc != Ret_Success) {
        DBG_PUT("NAND Flash file close failed");
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
    DBG_PUT(msg);

    if (io_funcs->open(io_funcs, name))
        return -2;

    if ((rc = arducam_dump_image(sensor, io_funcs))) {
        sprintf(msg, "image dump failed, rc: %d\r\n", rc);
        DBG_PUT(msg);
    }

    rc = io_funcs->close(io_funcs);
    return rc;
}

uint8_t fdata[PAGE_DATA_SIZE];

static int nand_dump_file(int which, io_funcs_t *io_driver) {
    char str[64];
    int rc;
    uint32_t len = NAND_File_Length(which);
    if (len == 0) {
        DBG_PUT("no files\r\n");
        return -1;
    }

    sprintf(str, "rfo %d, length %ld\n", which, len);
    DBG_PUT(str);

    FileHandle_t *fh = NAND_File_Open(which);
    if (!fh) {
        DBG_PUT("open failed\r\n");
        return -2;
    }

    if (io_driver->write_len) {
        if ((rc = io_driver->write_len(io_driver, len))) {
            return rc;
        }
    }

    size_t i = len;
    uint16_t cnt;
    while (i >= PAGE_DATA_SIZE) {
        uint16_t cnt = PAGE_DATA_SIZE;
        if ((rc = NAND_File_Read(fh, &cnt, fdata)) != Ret_Success) {
            sprintf(str, "read failed: %d\r\n", rc);
            DBG_PUT(str);
            break;
        }
        if (cnt == 0) {
            DBG_PUT("read returned 0?");
            break;
        }
        if ((rc = io_driver->write(io_driver, fdata, len))) {
            return rc;
        }
        i -= cnt;
    }

    if (i > 0) {
        if ((rc = NAND_File_Read(fh, &cnt, fdata)) != Ret_Success) {
            sprintf(str, "residue read failed: %d\r\n", rc);
            DBG_PUT(str);
        }

        if (cnt != i) {
            DBG_PUT("residue read returned 0?");
        }

        if ((rc = io_driver->write(io_driver, fdata, len))) {
            return rc;
        }
    }

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
        DBG_PUT("unsupported transfer media\r\n");
        return -1;
    }

    sprintf(msg, "Starting xfer to media %d\r\n", media);
    DBG_PUT(msg);

    if (io_funcs->open(io_funcs, which))
        return -2;

    if ((rc = nand_dump_file(which, io_funcs))) {
        sprintf(msg, "image dump failed, rc: %d\r\n", rc);
        DBG_PUT(msg);
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
	// TODO: This
    char msg[64];

	NAND_DIR *dir = NANDfs_opendir();
	if (dir == (NAND_DIR *)-1) {
		sprintf(msg, "Error opening dir, nand_errno: %d\n", nand_errno);
		DBG_PUT(msg);
		return -1;
	}
	DIRENT entry;
	do {
		entry = NANDfs_readdir(dir);
		sprintf(msg, "%d\n", entry.id);
		DBG_PUT(msg);
	} while (entry.id != 0);
    return 0;
}
