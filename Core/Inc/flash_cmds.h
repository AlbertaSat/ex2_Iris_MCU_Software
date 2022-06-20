#ifndef INC_TRANSFER_H_
#define INC_TRANSFER_H_

#include <stdint.h>

typedef enum {
    XFER_UART,
    XFER_FLASH,
    XFER_SPI,
} xfer_media_t;

typedef struct io_funcs {
    void *handle;
    uint32_t blksz;
    int (*open)(struct io_funcs *iof, uint32_t name);
    int (*write)(struct io_funcs *iof, uint8_t *data, uint16_t len);
    int (*write_len)(struct io_funcs *iof, uint32_t len);
    int (*close)(struct io_funcs *iof);
} io_funcs_t;

void init_nand_flash(void);

int transfer_image(uint8_t sensor, int32_t name, int media);

int transfer_file(int which, int media);

int list_files();

#endif // INC_TRANSFER_H_
