#include "stm32l0xx_hal.h"
#include "arducam.h"
#include <iris_system.h>
#include "ov5642_regs.h"
#include "flash_cmds.h"
#include "I2C.h"
#include "debug.h"
#include "spi_bitbang.h"
extern UART_HandleTypeDef huart1;

// probs needs to be re-evaluated
uint8_t image_number = 0;

/*
 * Delay wrapper function
 */
void arducam_delay_ms(int ms) { HAL_Delay(ms); }

/*
 * Programs sensor based on input m_fmt and target sensor
 * Called from init_sensors in uart_command_handler.c
 * Params: 	m_fmt: Integer
 * 				#define BMP 0
 * 				#define JPEG 1
 * 				#define RAW 2
 * 			sensor: Integer
 * 				VIS: 0
 * 				NIR: 1
 *
 */
void program_sensor(int m_fmt, int sensor) {
    if (m_fmt == RAW) {
        arducam_raw_init(1280, 960, sensor);
    } else {
        wrSensorReg16_8(REG_SYS_CTL0, 0x82, sensor); // software reset
        wrSensorRegs16_8(OV5642_QVGA_Preview, sensor);
        arducam_delay_ms(100);

        if (m_fmt == JPEG) {
            arducam_delay_ms(100);

            wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA, sensor);
            wrSensorRegs16_8(ov5642_320x240, sensor);
            arducam_delay_ms(100);

            wrSensorReg16_8(0x3818, 0xa8, sensor);
            wrSensorReg16_8(0x3621, 0x10, sensor);
            wrSensorReg16_8(0x3801, 0xb0, sensor);
            wrSensorReg16_8(0x4407, 0x08, sensor);
            wrSensorReg16_8(0x5888, 0x00, sensor);
            wrSensorReg16_8(0x5000, 0xFF, sensor);
        } else {
            byte reg_val;
            wrSensorReg16_8(0x4740, 0x21, sensor);
            wrSensorReg16_8(0x501e, 0x2a, sensor);     // RGB Dither Ctl = RGB565/555
            wrSensorReg16_8(0x5002, 0xf8, sensor);     // ISP Ctl 2 = Dither enable
            wrSensorReg16_8(0x501f, 0x01, sensor);     // Format MUX Ctl = ISP RGB
            wrSensorReg16_8(0x4300, 0x61, sensor);     // Format Ctl = RGB565
            rdSensorReg16_8(0x3818, &reg_val, sensor); // Timing Ctl = Mirror/Vertical flip
            wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff, sensor);
            rdSensorReg16_8(0x3621, &reg_val, sensor); // Array Ctl 01 = Horizontal bin
            wrSensorReg16_8(0x3621, reg_val & 0xdf, sensor);
        }
    }
}

/*
 * Initalizes the arducam in RAW mode. DO NOT USE.
 */
void arducam_raw_init(int width, int depth, uint8_t sensor) {
    /* I don't know if you have to completely reprogram everything every time
     * you change the resolution, but that's what all the examples do.
     *
     * I also don't know if the order of programming the registers matters, but
     * all the examples set the resolution in exactly the same place.
     */
    wrSensorRegs16_8(OV5642_RAW_Init_start, sensor);

    wrSensorReg16_8(REG_DVPHO_HI, (uint8_t)(width >> 8), sensor);
    wrSensorReg16_8(REG_DVPHO_LO, (uint8_t)(width & 0x0ff), sensor);
    wrSensorReg16_8(REG_DVPVO_HI, (uint8_t)(depth >> 8), sensor);
    wrSensorReg16_8(REG_DVPVO_LO, (uint8_t)(depth & 0x0ff), sensor);

    wrSensorRegs16_8(OV5642_RAW_Init_finish, sensor);
}

/*
 * Function to get the current resolution of target sensor
 *
 * params:
 * 		width: pointer to variable to store width
 * 		depth: pointer to variable to store height (depth)
 * 		sensor: Integer sensor identifier
 */
void arducam_get_resolution(int *width, int *depth, uint8_t sensor) {
    if (!width || !depth)
        return;
    *width = 0;
    *depth = 0;

    uint8_t reg_val;
    rdSensorReg16_8(REG_DVPHO_HI, &reg_val, sensor);
    *width = reg_val << 8;
    rdSensorReg16_8(REG_DVPHO_LO, &reg_val, sensor);
    *width |= reg_val;
    rdSensorReg16_8(REG_DVPVO_HI, &reg_val, sensor);
    *depth = reg_val << 8;
    rdSensorReg16_8(REG_DVPVO_LO, &reg_val, sensor);
    *depth |= reg_val;
}

/*
 * Function to get the current resolution of target sensor
 *
 * params:
 * 		width: pointer to variable width
 * 		depth: pointer to variable height (depth)
 * 		sensor: Integer sensor identifier
 */
int arducam_set_resolution(int format, int width, uint8_t sensor) {
    int rc = width;
    switch (width) {
    case 320:
        if (format == RAW) {
            DBG_PUT("320x240 not supported for RAW");
            rc = 0;
        } else
            wrSensorRegs16_8(ov5642_320x240, sensor);
        break;
    case 640:
        if (format == RAW)
            arducam_raw_init(640, 480, sensor);
        else
            wrSensorRegs16_8(ov5642_640x480, sensor);
        break;
    case 1024:
        if (format == RAW) {
            DBG_PUT("1024x768 not supported for RAW");
            rc = 0;
        } else
            wrSensorRegs16_8(ov5642_1024x768, sensor);
        break;
    case 1280:
        if (format == RAW)
            arducam_raw_init(1280, 960, sensor);
        else
            wrSensorRegs16_8(ov5642_1280x960, sensor);
        break;
#if 0
    case 1600:
      wrSensorRegs16_8(ov5642_1600x1200, sensor);
      break;
#endif
    case 1920:
        if (format == RAW)
            arducam_raw_init(1920, 1080, sensor);
        else {
            DBG_PUT("1920X1080 not supported");
            rc = 0;
        }
        break;
#if 0
    case 2048:
      wrSensorRegs16_8(ov5642_2048x1536, sensor);
      break;
#endif
    case 2592:
        if (format == RAW)
            arducam_raw_init(2592, 1944, sensor);
        else {
            wrSensorRegs16_8(ov5642_2592x1944, sensor);
        }
        break;
    default:
        DBG_PUT("unsupported width\r\n");
        rc = 0;
        break;
    }
    HAL_Delay(1000);
    return rc;
}

#define READY_MAGIC 0x55

/*
 * Yells at the arducam module to make sure the spi read/write is working
 *
 * param:
 * 		sensor: integer sensor identifier
 */
bool arducam_wait_for_ready(uint8_t sensor) {
    /* Workaround for the Arducam thinking the first write is a read from 0x40 */
    uint8_t wval;
    uint8_t rval;
    for (int i = 0; i < 10; i++) {
        wval = READY_MAGIC + i;
        rval = 0;
        write_reg(AC_REG_TEST, wval, sensor);
        rval = read_reg(AC_REG_TEST, sensor);
        if (rval == wval)
            break;

        HAL_Delay(100);
    }

    return (rval == wval);
}

/*
 * Read sensor SPI reg
 *
 * param:
 * 		addr: spi register address to read from
 * 		sensor: integer sensor identifier
 * return:
 * 		byte value from register
 */
uint8_t read_reg(uint8_t addr, uint8_t sensor) {
    uint8_t data;
    data = read_spi_reg(addr, sensor);
    return data;
}

/*
 * Read sensor SPI reg
 *
 * param:
 * 		addr: spi register address to write to
 * 		data: byte data to write
 * 		sensor: integer sensor identifier
 */
void write_reg(uint8_t addr, uint8_t data, uint8_t sensor) { write_spi_reg(addr, data, sensor); }

/*
 * Reads image byte from fifo buffer
 *
 * param:
 * 		sensor: integer sensor identifier
 */
static uint8_t read_fifo(uint8_t sensor) {
    uint8_t data;
    data = read_reg(SINGLE_FIFO_READ, sensor);
    return data;
}

/*
 * flushes fifo buffer on arducam sensor
 *
 * param:
 * 		sensor: integer sensor identifier
 */
void flush_fifo(uint8_t sensor) { write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK, sensor); }

/*
 * raises capture flag on arducam sensor
 *
 * param:
 * 		sensor: integer sensor identifier
 */
void start_capture(uint8_t sensor) { write_reg(ARDUCHIP_FIFO, FIFO_START_MASK, sensor); }

/*
 * flushes fifo buffer on arducam sensor
 *
 * param:
 * 		sensor: integer sensor identifier
 */
void clear_fifo_flag(uint8_t sensor) { write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK, sensor); }

// void set_test_mode(uint8_t mode, uint8_t sensor) {
//    write_reg(AC_REG_TEST_MODE, mode, sensor);
//    HAL_Delay(1000);
//}

/*
 * reads  fifo buffer length on arducam sensor
 *
 * param:
 * 		sensor: integer sensor identifier
 * return:
 * 		3 byte length of image buffer
 */
uint32_t read_fifo_length(uint8_t sensor) {
    uint32_t len1, len2, len3, len = 0;
    len1 = read_reg(FIFO_SIZE1, sensor);
    len2 = read_reg(FIFO_SIZE2, sensor);
    len3 = read_reg(FIFO_SIZE3, sensor) & 0x7f;
    len = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
    return len;
}

// Set corresponding bit
void set_bit(uint8_t addr, uint8_t bit, uint8_t sensor) {
    uint8_t temp;
    temp = read_reg(addr, sensor);
    write_reg(addr, temp | bit, sensor);
}

// Clear corresponding bit
void clear_bit(uint8_t addr, uint8_t bit, uint8_t sensor) {
    uint8_t temp;
    temp = read_reg(addr, sensor);
    write_reg(addr, temp & (~bit), sensor);
}

// Get corresponding bit status
uint8_t get_bit(uint8_t addr, uint8_t bit, uint8_t sensor) {
    uint8_t temp;
    temp = read_reg(addr, sensor);
    temp = temp & bit;
    return temp;
}

void arducam_set_saturation(int saturation, uint8_t sensor) {
    wrSensorReg16_8(0x5001, 0xff, sensor);            // enable saturation setting
    wrSensorReg16_8(0x5583, saturation << 4, sensor); // Set U saturation
    wrSensorReg16_8(0x5584, saturation << 4, sensor); // Set V saturation
    wrSensorReg16_8(0x5580, 0x02, sensor);            // enable Special Effects
}

int arducam_get_saturation(uint8_t sensor) {
    uint8_t reg_val;
    /* Technically, this is only the U saturation. They can all be set separately */
    rdSensorReg16_8(0x5583, &reg_val, sensor);
    return reg_val >> 4;
}

void arducam_capture_image(uint8_t sensor) {
    char msg[64];
    sprintf(msg, "Single Capture on sensor %d\r\n", sensor);
    DBG_PUT(msg);

    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, sensor); // VSYNC is active HIGH

    flush_fifo(sensor);
    clear_fifo_flag(sensor);
    start_capture(sensor);

    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, sensor)) {
    }

    DBG_PUT("Capture complete\r\n");
}

static inline uint32_t min(uint32_t a, uint32_t b) { return (a <= b) ? a : b; }

//#define BMP_HDR_LEN 14
//#define INFO_HDR_LEN 52
//#define BMPIMAGEOFFSET (BMP_HDR_LEN + INFO_HDR_LEN)
//
//#define pgm_read_byte(x) (*((char *)x))
//
// char bmp_header[BMPIMAGEOFFSET] = {
//    'B',          'M',  0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, BMPIMAGEOFFSET, 0x00, 0x00, 0x00,
//    INFO_HDR_LEN, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00,           0x00, 0x01, 0x00,
//    0x10,         0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4,           0x0E, 0x00, 0x00,
//    0xC4,         0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,           0x00, 0x00, 0xF8,
//    0x00,         0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
//
// static void dump_uart_bmp(uint8_t sensor) {
//    uint32_t width = 320;
//    uint32_t depth = 240;
//    uint32_t length = width * depth;
//    char buf[64];
//    buf[2] = ' ';
//    buf[3] = 0;
//
//    char BMP_hdr[BMP_HDR_LEN] = {0};
//    char info_hdr[INFO_HDR_LEN] = {0};
//    BMP_hdr[0] = 'B';
//    BMP_hdr[1] = 'M';
//    uint32_t filesize = length * 2 + BMP_HDR_LEN + INFO_HDR_LEN;
//    BMP_hdr[2] = filesize;
//    BMP_hdr[3] = filesize >> 8;
//    BMP_hdr[4] = filesize >> 16;
//    BMP_hdr[5] = filesize >> 24;
//    BMP_hdr[10] = BMP_HDR_LEN + INFO_HDR_LEN;
//
//    // write the header
//    int i;
//    for (i = 0; i < BMP_HDR_LEN; i++) {
//        buf[0] = hex_2_ascii(BMP_hdr[i] >> 4);
//        buf[1] = hex_2_ascii(BMP_hdr[i] & 0x0f);
//        DBG_PUT(buf);
//    }
//
//    info_hdr[0] = INFO_HDR_LEN;
//    info_hdr[4] = width;
//    info_hdr[5] = width >> 8;
//    info_hdr[6] = width >> 16;
//    info_hdr[7] = width >> 24;
//    info_hdr[8] = depth;
//    info_hdr[9] = depth >> 8;
//    info_hdr[10] = depth >> 16;
//    info_hdr[11] = depth >> 24;
//    info_hdr[12] = 1;  // # of color planes
//    info_hdr[14] = 16; // # of bits per pixel
//    info_hdr[16] = 3;  // BI_BITFIELDS
//    info_hdr[20] = (length * 2);
//    info_hdr[21] = (length * 2) >> 8;
//    info_hdr[22] = (length * 2) >> 16;
//    info_hdr[23] = (length * 2) >> 24;
//    info_hdr[24] = info_hdr[28] = 0xc4; // print resolution
//    info_hdr[25] = info_hdr[29] = 0x0e;
//    info_hdr[40] = 0; // Red channel bitmask - in big-endian
//    info_hdr[41] = 0xf8;
//    info_hdr[44] = 0xe0; // Green channel bitmask
//    info_hdr[45] = 0x07;
//    info_hdr[48] = 0x1f; // Blue channel bitmask in big-endian
//
//    // write the info
//    for (i = 0; i < INFO_HDR_LEN; i++) {
//        buf[0] = hex_2_ascii(info_hdr[i] >> 4);
//        buf[1] = hex_2_ascii(info_hdr[i] & 0x0f);
//        DBG_PUT(buf);
//    }
//
//    for (int i = 0; i < length; i++) {
//        uint8_t VH = read_fifo(sensor);
//        uint8_t VL = read_fifo(sensor);
//        sprintf(buf, "%02x %02x ", VH, VL);
//        DBG_PUT(buf);
//    }
//}
//
//
//#define BUF_LEN 64
//
static void dump_uart_jpg_burst(uint32_t length, uint8_t sensor) {

    uint32_t BUF_LEN = 512;
    uint8_t prev = 0, curr = 0;
    bool found_header = false;
    uint32_t i, x = 0;
    uint8_t buf[BUF_LEN];

    // Note: we're assuming the ARM is BE
    memcpy(buf, &length, sizeof(length));
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, sizeof(length), 100);

    spi_init_burst(sensor);
    for (i = 0; i < length; i++) {
        prev = curr;
        curr = spi_read_burst(sensor);
        if ((curr == 0xd9) && (prev == 0xff)) {
            // found the footer - break
            buf[x++] = curr;
            HAL_UART_Transmit(&huart1, buf, x, 100);
            x = 0;
            i++;
            found_header = false;
            break;
        }

        if (found_header) {
            buf[x] = curr;
            x++;
            if (x >= BUF_LEN) {
                HAL_UART_Transmit(&huart1, buf, BUF_LEN, 100);
                x = 0;
            }
        } else if ((curr == 0xd8) && (prev = 0xff)) {
            found_header = true;
            buf[0] = prev;
            buf[1] = curr;
            HAL_UART_Transmit(&huart1, (uint8_t *)buf, 2, 100);
            x = 0;
        }
    }
    spi_deinit_burst(sensor);

    if (x) {
        HAL_UART_Transmit(&huart1, buf, x, 100);
    }

    if (found_header) {
        // We found the header but not the footer :-(
        buf[0] = 0xff;
        buf[1] = 0xd9;
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, 2, 100);
    } else {
        memset(buf, 0, BUF_LEN);
        while (i < length) {
            int cnt = min(length - i, BUF_LEN);
            HAL_UART_Transmit(&huart1, (uint8_t *)buf, cnt, 100);
            i += cnt;
        }
    }
}
//
// static void uart_dump_buf(uint8_t *data, uint16_t len) {
//    char digit[4];
//    digit[2] = ' ';
//    digit[3] = 0;
//    for (int i = 0; i < len; i++) {
//        digit[0] = hex_2_ascii(data[i] >> 4);
//        digit[1] = hex_2_ascii(data[i] & 0x0f);
//        DBG_PUT(digit);
//    }
//}
//
//#define MAX_BLK_SZ 2048
// static uint8_t buf[MAX_BLK_SZ];
//
// int arducam_dump_image(uint8_t sensor, io_funcs_t *io_driver) {
//    int rc;
//    uint8_t prev = 0, curr = 0;
//    bool found_header = false;
//    uint32_t i, x = 0;
//    uint32_t length;
//    char msg[64];
//
//    length = arducam_read_fifo_length(sensor);
//
//
//    if (io_driver->write_len) {
//        if ((rc = io_driver->write_len(io_driver, length))) {
//            sprintf(msg, "xfer length failed, rc: %d\r\n", rc);
//            DBG_PUT(msg);
//            return rc;
//        }
//    }
//
//    for (i = 0; i < length; i++) {
//        prev = curr;
//        curr = read_fifo(sensor);
//        if ((curr == 0xd9) && (prev == 0xff)) {
//            // found the footer - break
//            buf[x++] = curr;
//
//            DBG_PUT("writing footer block\r\n");
//            uart_dump_buf(buf, x);
//
//            if (io_driver->write(io_driver, buf, x)) {
//                DBG_PUT("Write early footer failed");
//            }
//            x = 0;
//            i++;
//            found_header = false;
//            break;
//        }
//
//        if (found_header) {
//            buf[x] = curr;
//            x++;
//            if (x >= io_driver->blksz) {
//                DBG_PUT("writing full block\r\n");
//                uart_dump_buf(buf, x);
//                if (io_driver->write(io_driver, buf, x)) {
//                    DBG_PUT("Write body failed");
//                }
//                x = 0;
//            }
//        } else if ((curr == 0xd8) && (prev = 0xff)) {
//            found_header = true;
//            buf[0] = prev;
//            buf[1] = curr;
//            x = 2;
//        }
//    }
//
//    if (x) {
//        if (io_driver->write(io_driver, buf, x)) {
//            DBG_PUT("Write tail failed");
//        }
//    }
//
//#if 0
//    if (found_header) {
//        // We found the header but not the footer :-(
//        buf[0] = 0xff;
//        buf[1] = 0xd9;
//        HAL_UART_Transmit(&huart1, (uint8_t *) buf, 2, 100);
//    }
//    else {
//        memset(buf, 0, BLK_SZ);
//        while (i < length) {
//            int cnt = min(length - i, BLK_SZ);
//            HAL_UART_Transmit(&huart1, (uint8_t *) buf, cnt, 100);
//            i += cnt;
//        }
//    }
//#endif
//    return 0;
//}
//
// static void dump_uart_raw(uint32_t length, uint8_t sensor) {
//    char buf[4];
//    buf[2] = ' ';
//    buf[3] = '\0';
//
//    for (int i = 0; i < length / 2; i++) {
//        uint8_t rgb = read_fifo(sensor);
//        buf[0] = hex_2_ascii(rgb >> 4);
//        buf[1] = hex_2_ascii(rgb & 0x0f);
//        DBG_PUT(buf);
//    }
//}
//

//
//
void SingleCapTransfer(int format, uint8_t sensor) {
    char buf[64];
    uint32_t length;

    sprintf(buf, "Single Capture Transfer type %x\r\n", format);
    DBG_PUT(buf);
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, sensor); // VSYNC is active HIGH
    uint8_t val;
    rdSensorReg16_8(REG_FORMAT_CTL, &val, sensor);
    sprintf(buf, "format reg: 0x%02x\r\n", val);
    DBG_PUT(buf);

    flush_fifo(sensor);
    clear_fifo_flag(sensor);
    start_capture(sensor);

    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, sensor)) {
    }

    length = read_fifo_length(sensor);
    sprintf(buf, "Capture complete! FIFO len 0x%lx\r\n", length);
    DBG_PUT(buf);
    DBG_PUT("JPG");
    dump_uart_jpg_burst(length, sensor);
    DBG_PUT("\04");
}

void set_image_num(uint8_t num) { image_number = num; }
