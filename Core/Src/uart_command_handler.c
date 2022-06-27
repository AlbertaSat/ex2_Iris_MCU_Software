#include <stdio.h>
#include <ctype.h>
#include <command_handler.h>
#include <iris_system.h>
#include "stm32l0xx_hal.h"
#include "arducam.h"
#include "debug.h"
#include "IEB_TESTS.h"
#include "flash_cmds.h"
#include "housekeeping.h"
extern int format;
extern I2C_HandleTypeDef hi2c2;

// prototypes
uint8_t onboot_sensors(uint8_t sensor);


static inline const char *next_token(const char *ptr) {
    /* move to the next space */
    while (*ptr && *ptr != ' ')
        ptr++;
    /* move past any whitespace */
    while (*ptr && isspace(*ptr))
        ptr++;

    return (*ptr) ? ptr : NULL;
}

void help() {
    // UART DEBUG ONLY
#ifdef UART_DEBUG
    DBG_PUT("TO RUN TESTS: test\r\n\n\n");
    DBG_PUT("Commands:\r\n");
    DBG_PUT("\tWorking/Tested:\r\n");
    DBG_PUT("\t\tpower <on/off> | toggles sensor power\r\n");
    DBG_PUT("\t\tinit sensor | Resets arducam modules to default\r\n");
    DBG_PUT("\t\tcapture <vis/nir> | capture image from sensor\r\n");
    DBG_PUT("\t\tformat<vis/nir> [JPEG|BMP|RAW]\r\n");
    DBG_PUT("\t\t hk | Gets housekeeping\r\n");
    DBG_PUT("\t\twidth <vis/nir> [<pixels>]\r\n");
    DBG_PUT("\t\tscan | Scan I2C bus 2\r\n");
    DBG_PUT("\t\ti2c read deviceaddress registeraddress | read from i2c device register. values in hex\r\n");
    DBG_PUT("\t\ti2c write deviceaddress registeraddress value | write to i2c device register. values in hex\r\n");
    DBG_PUT("\tNeeds work\r\n");
    DBG_PUT("\t\txfer sensor media filename | transfer image over media\r\n");
    DBG_PUT("\tNot tested/partially implemented:\r\n");
    DBG_PUT("\t\tlist n | List the n most recent files\r\n");
    DBG_PUT("\t\tread ro media | Transfer relative file offset number\r\n");
    DBG_PUT("\t\treg <vis/nir> read <regnum>\r\n\treg write <regnum> <val>\r\n");
    DBG_PUT("\t\tSaturation [<0..8>]\r\n");
#endif
}

void uart_handle_format_cmd(const char *cmd) {
    // TODO: Needs to handle sensor input
    const char *format_names[3] = {"BMP", "JPEG", "RAW"};
    char buf[64];

    const char *wptr = next_token(cmd);

    int target_sensor;
    switch (*wptr) {
    case 'v':
        if (VIS_DETECTED == 0) {
            DBG_PUT("VIS Unavailable.\r\n");
            return;
        }
        target_sensor = VIS_SENSOR; // vis = 0
        break;

    case 'n':
        if (NIR_DETECTED == 0) {
            DBG_PUT("NIR Unavailable.\r\n");
            return;
        }
        target_sensor = NIR_SENSOR;
        break;
    default:
        DBG_PUT("Target Error.\r\n");
        return;
    }

    const char *fmtarg = next_token(wptr);
    int old_format = format;

    if (fmtarg) {
        switch (*fmtarg) {
        case 'B':
            format = BMP;
            break;
        case 'J':
            format = JPEG;
            break;
        case 'R':
            format = RAW;
            break;
        default:
            sprintf(buf, "unknown format: <%s>\r\n", fmtarg);
            DBG_PUT(buf);
            return;
        }
    }

    if (format != old_format) {
        program_sensor(format, target_sensor);
    }
    DBG_PUT("current format: ");
    DBG_PUT(format_names[format]);
    DBG_PUT("\r\n");
}

void handle_reg_cmd(const char *cmd) {
    char buf[64];
    const char *wptr = next_token(cmd);

    int target_sensor;
    switch (*wptr) {
    case 'v':
        if (VIS_DETECTED == 0) {
            DBG_PUT("VIS Unavailable.\r\n");
            return;
        }
        target_sensor = VIS_SENSOR; // vis = 0
        break;

    case 'n':
        if (NIR_DETECTED == 0) {
            DBG_PUT("NIR Unavailable.\r\n");
            return;
        }
        target_sensor = NIR_SENSOR;
        break;
    default:
        DBG_PUT("Target Error.\r\n");
        return;
    }

    const char *rwarg = next_token(wptr);

    if (!rwarg) {
        help();
        return;
    }

    const char *regptr = next_token(rwarg);
    if (!regptr) {
        help();
        return;
    }

    uint32_t reg;
    if (sscanf(regptr, "%lx", &reg) != 1) {
        help();
        return;
    }

    switch (*rwarg) {
    case 'r': {
        uint8_t val;
        rdSensorReg16_8(reg, &val, target_sensor);
        sprintf(buf, "register 0x%lx = 0x%02x\r\n", reg, val);
    } break;

    case 'w': {
        const char *valptr = next_token(regptr);
        if (!valptr) {
            sprintf(buf, "reg write 0x%lx: missing reg value\r\n", reg);
            break;
        }
        uint32_t val;
        if (sscanf(valptr, "%lx", &val) != 1) {
            sprintf(buf, "reg write 0x%lx: bad val '%s'\r\n", reg, valptr);
            break;
        }
        wrSensorReg16_8(reg, val, target_sensor);
        sprintf(buf, "register 0x%lx wrote 0x%02lx\r\n", reg, val);
    } break;
    default:
        sprintf(buf, "reg op must be read or write, '%s' not supported\r\n", rwarg);
        break;
    }
    DBG_PUT(buf);
}

void uart_handle_width_cmd(const char *cmd) {
    char buf[64];
    const char *wptr = next_token(cmd);
    if (!wptr) {
        int width, depth;
        if (VIS_DETECTED) {
            arducam_get_resolution(&width, &depth, VIS_SENSOR);
            sprintf(buf, "VIS Camera Resolution: %d by %d\r\n", width, depth);
            DBG_PUT(buf);
        }
        if (NIR_DETECTED) {
            arducam_get_resolution(&width, &depth, NIR_SENSOR);
            sprintf(buf, "NIR Camera Resolution: %d by %d\r\n", width, depth);
            DBG_PUT(buf);
        }
        return;
    }
    buf[0] = 0;
    int target_sensor;
    switch (*wptr) {
    case 'v':
        if (VIS_DETECTED == 0) {
            DBG_PUT("VIS Unavailable.\r\n");
            return;
        }
        target_sensor = VIS_SENSOR; // vis = 0
        break;
    case 'n':
        if (NIR_DETECTED == 0) {
            DBG_PUT("NIR Unavailable.\r\n");
            return;
        }
        target_sensor = NIR_SENSOR;
        break;
    default:
        DBG_PUT("Target Error.\r\n");
        return;
    }
    const char *res = next_token(wptr);

    switch (*res) {
    case '6':
        if (arducam_set_resolution(format, 640, target_sensor))
            strcpy(buf, "resolution is now 640x480\r\n");
    case '1':
        switch (*(res + 1)) {
        case '0':
            if (arducam_set_resolution(format, 1024, target_sensor))
                strcpy(buf, "resolution is now 1024x768\r\n");
            break;
        case '2':
            if (arducam_set_resolution(format, 1280, target_sensor))
                strcpy(buf, "resolution is now 1280x960\r\n");
            break;
        case '9':
            if (arducam_set_resolution(format, 1920, target_sensor))
                strcpy(buf, "resolution is now 1920x1080\r\n");
            break;
        default:
            break;
        }
        break;
    case '3':
        if (arducam_set_resolution(format, 320, target_sensor))
            strcpy(buf, "resolution is now 320x240\r\n");
        break;
    default:
        sprintf(buf, "unsupported width: <%s>\r\n", res);
        break;
    }

    if (buf[0])
        DBG_PUT(buf);
}

void uart_handle_capture_cmd(const char *cmd) {
    const char *wptr = next_token(cmd);

    int target_sensor;
    switch (*wptr) {
    case 'v':
#ifndef FAKE_CAM
        if (VIS_DETECTED == 0) {
            DBG_PUT("VIS Unavailable.\r\n");
            return;
        }
#endif
        target_sensor = VIS_SENSOR; // vis = 0
        break;

    case 'n':
        if (NIR_DETECTED == 0) {
            DBG_PUT("NIR Unavailable.\r\n");
            return;
        }
        target_sensor = NIR_SENSOR;
        break;
    default:
        DBG_PUT("Target Error.\r\n");
        return;
    }

    arducam_capture_image(target_sensor);
}

void uart_handle_xfer_cmd(const char *cmd) {
    const char *wptr = next_token(cmd);

    int target_sensor;
    switch (*wptr) {
    case 'v':
#ifndef FAKE_CAM
        if (VIS_DETECTED == 0) {
            DBG_PUT("VIS Unavailable.\r\n");
            return;
        }
#endif
        target_sensor = VIS_SENSOR; // vis = 0
        break;

    case 'n':
        if (NIR_DETECTED == 0) {
            DBG_PUT("NIR Unavailable.\r\n");
            return;
        }
        target_sensor = NIR_SENSOR;
        break;
    default:
        DBG_PUT("Target Error.\r\n");
        return;
    }

    int media = XFER_UART;
    wptr = next_token(wptr);
    if (!wptr) {
        DBG_PUT("Xfer: missing media\n");
        return;
    }
    switch (*wptr) {
    case 'f':
        media = XFER_FLASH;
        break;
    case 'u':
        media = XFER_UART;
        break;
    default:
        media = XFER_SPI;
        break;
    }

    wptr = next_token(wptr);
    if (!wptr) {
        DBG_PUT("Xfer: missing name\n");
        return;
    }

    uint32_t fname;
    if (sscanf(wptr, "%lx", &fname) != 1) {
        DBG_PUT("name must be a number (for now)\r\n");
        memcpy(&fname, wptr, sizeof(fname));
    }

    transfer_image(target_sensor, fname, media);
}

void uart_handle_read_file_cmd(const char *cmd) {
    const char *wptr = next_token(cmd);

    int which;
    if (sscanf(wptr, "%d", &which) != 1) {
        DBG_PUT("Can't parse relative file offset\r\n");
        return;
    }

    int media = XFER_UART;
    wptr = next_token(wptr);
    if (!wptr) {
        DBG_PUT("Xfer: missing media\n");
        return;
    }
    switch (*wptr) {
    case 'f':
        media = XFER_FLASH;
        break;
    case 'u':
        media = XFER_UART;
        break;
    default:
        media = XFER_SPI;
        break;
    }

    transfer_file(which, media);
}

void uart_handle_list_files_cmd(const char *cmd) {
    const char *wptr = next_token(cmd);


    list_files();
}


uint8_t onboot_sensors(uint8_t sensor){
	// Reset the CPLD

	// Make sure camera is listening over SPI
	arducam_wait_for_ready(sensor);
	//reset I2C regs
	write_reg(AC_REG_RESET, 1, sensor);
	write_reg(AC_REG_RESET, 1, sensor);
	HAL_Delay(100);
	write_reg(AC_REG_RESET, 0, sensor);
	HAL_Delay(100);

	if (!arducam_wait_for_ready(sensor)) {
		DBG_PUT("Camera %x: SPI Unavailable\r\n", sensor);
	} else {
		DBG_PUT("Camera %x: SPI Initialized\r\n", sensor);
	}

	// Change MCU mode
	write_reg(ARDUCHIP_MODE, 0x0, sensor);
	wrSensorReg16_8(0xff, 0x01, sensor);

	uint8_t vid = 0, pid = 0;
	// checks sensor id to ensure proper i2c performance
	rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid, sensor);
	rdSensorReg16_8(OV5642_CHIPID_LOW, &pid, sensor);

	if (vid != 0x56 || pid != 0x42) {
		DBG_PUT("Camera %x I2C Address: Unknown\r\nVIS not available\r\n\n", sensor);
		return -1;

	} else {
		return 1;
	}
}

void uart_init_sensors(void) {
	format = JPEG;
    uint8_t res = onboot_sensors(VIS_SENSOR);
	if (res == 1){
		program_sensor(format, VIS_SENSOR);
	DBG_PUT("VIS Camera Mode: JPEG\r\nI2C address: 0x3C\r\n\n");
	}
	if (res == -1){
		// need some error handling eh
		DBG_PUT("VIS init failed./r/n");
		return;
	}

	res = 0;
    res = onboot_sensors(NIR_SENSOR);
	if (res == 1){
		program_sensor(format, NIR_SENSOR);
	DBG_PUT("NIR Camera Mode: JPEG\r\nI2C address: 0x3D\r\n\n");
	}
	if (res == -1){
		// need some error handling eh
		DBG_PUT("NIR init failed.\r\n");
		return;
	}
    HAL_Delay(1000);
}

void sensor_togglepower(int i) {
    if (i == 1) {
        HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_SET);
        DBG_PUT("Sensor Power Enabled.\r\n");
        return;
    }
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_RESET);
    DBG_PUT("Sensor Power Disabled.\r\n");
}

// todo implement sensor selection
void uart_handle_saturation_cmd(const char *cmd, uint8_t sensor) {
    char buf[64];
    const char *satarg = next_token(cmd);
    int saturation;

    if (satarg) {
        if (*satarg >= '0' && *satarg <= '8') {
            saturation = *satarg - '0';
            arducam_set_saturation(saturation, sensor);
        } else
            DBG_PUT("legal saturation values are 0-8\r\n");
    }

    saturation = arducam_get_saturation(sensor);
    sprintf(buf, "current saturation: %x\r\n", saturation);
    DBG_PUT(buf);
}

void handle_i2c16_8_cmd(const char *cmd) {
    char buf[64];
    const char *rwarg = next_token(cmd);

    if (!rwarg) {
        DBG_PUT("rwarg broke\r\n");
        return;
    }
    const char *rwaddr = next_token(rwarg);

    if (!rwaddr) {
        DBG_PUT("rwaddr broke\r\n");
        return;
    }

    const char *regptr = next_token(rwaddr);
    if (!regptr) {
        DBG_PUT("regptr broke\r\n");
        return;
    }

    uint32_t reg;
    if (sscanf(regptr, "%lx", &reg) != 1) {
        DBG_PUT("reg broke\r\n");
        return;
    }

    uint32_t addr;
    if (sscanf(rwaddr, "%lx", &addr) != 1) {
        DBG_PUT("addr broke\r\n");
        return;
    }

    switch (*rwarg) {
    case 'r': {
        uint8_t val = 0x00;
        i2c2_read16_8(addr, reg, &val); // switch back to 16-8
        sprintf(buf, "Device 0x%lx register 0x%lx = 0x%x\r\n", addr, reg, val);
    } break;

    case 'w': {
        const char *valptr = next_token(regptr);
        if (!valptr) {
            sprintf(buf, "reg write 0x%lx: missing reg value\r\n", reg);
            break;
        }
        uint32_t val;
        if (sscanf(valptr, "%lx", &val) != 1) {
            sprintf(buf, "reg write 0x%lx: bad val '%s'\r\n", reg, valptr);
            break;
        }
        i2c2_write16_8(addr, reg, val);

        sprintf(buf, "Device 0x%lx register 0x%lx wrote 0x%02lx\r\n", addr, reg, val);
    } break;
    default:
        sprintf(buf, "reg op must be read or write, '%s' not supported\r\n", rwarg);
        break;
    }
    DBG_PUT(buf);
}

void uart_get_hk_packet(uint8_t *out) {
    // uint8_t *out as arg
    housekeeping_packet_t hk;
    hk = _get_housekeeping();
    //    memcpy(out, (uint8_t *)&hk, sizeof(housekeeping_packet_t));
    decode_hk_packet(hk);
    return;
}

void print_progress(uint8_t count, uint8_t max) {
    uint8_t length = 25;
    uint8_t scaled = count * 100 / max * length / 100;
    char buf[128];
    sprintf(buf, "Progress: [%.*s%.*s]\r", scaled,
            "==================================================", length - scaled,
            "                                        ");
    DBG_PUT(buf);
    if (count == max) {
        DBG_PUT("\r\n");
    }
}
