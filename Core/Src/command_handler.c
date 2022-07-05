/*
 * command_handler.c
 *
 *  Created on: May 9, 2022
 *      Author: Liam
 */
#include "command_handler.h"
#include "string.h"
#include "debug.h"
#include "arducam.h"
#include "SPI_IT.h"
#include "IEB_TESTS.h"
extern uint8_t VIS_DETECTED;
extern uint8_t NIR_DETECTED;
extern SPI_HandleTypeDef hspi1;
extern int format;
extern int width;
extern const struct sensor_reg OV5642_JPEG_Capture_QSXGA[];
extern const struct sensor_reg OV5642_QVGA_Preview[];

uint8_t VIS_DETECTED = 0;
uint8_t NIR_DETECTED = 0;


/*
 * todo:
 * 		- 	TEST THESE FUNCTIONS EH
 *
 * 		- 	Determine if the  SPI CltCallback in main.c are called from interrupt SPI
 * 			functions in here, or if they're needed in here / in SPI_IT.c [SOLVED]
 * 		- 	Find a way to send sensors into idle mode without erasing regs
 * 		  	otherwise save sensor regs somewhere in a struct.
 *		- 	Write functions to interface with sensor currently, but need to adapt to
 *		  	work with Ron's NAND Flash stuff.
 *
 */
uint8_t ack = 0xAA;
uint8_t total_image_num = 0; // This will cause issues with total num of images once board resets. todo: fix
housekeeping_packet_t hk;
char buf[128];

/**
 * @brief prototype for taking image; for use with SPI ONLY
 *        UNTESTED.
 *
 *        todo: rewrite once Jenish gets image transfer working
 */
void take_image() {
    /*
     * Todo: Determine whether or not we want to have individual sensor control, or just cap both at the same time
     * (ish) Fix Arducam.h so we stop with these warnings
     */
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, VIS_SENSOR); // VSYNC is active HIGH
    write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, NIR_SENSOR);

    flush_fifo(VIS_SENSOR);
    flush_fifo(NIR_SENSOR);

    clear_fifo_flag(VIS_SENSOR);
    clear_fifo_flag(NIR_SENSOR);

    start_capture(VIS_SENSOR);
    start_capture(NIR_SENSOR);

    // todo: determine if cap_done_mask stays high for subsequent reads of arducam_trig register. Otherwise this
    // loop
    //		 will never break
    DBG_PUT("listening for cap done mask\r\n");
    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, VIS_SENSOR)) {
    }
    DBG_PUT("vis sensor complete\r\n");
    while (!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, NIR_SENSOR)) {
    }
    DBG_PUT("nir sensor complete\r\n");
    DBG_PUT("Loop broke!\r\n");
    ;

    // todo:
    //  keep track of how many images we have captured. This could come after transferring
    //  to flash
    //	_iterate_image_number();

    // kick over images to our NAND flash
    //	_transfer_images_to_flash();

    return;
}

/**
 * @brief Get the image length object from NAND FLASH and transmit it over SPI
 *
 * Untested
 *
 */
void get_image_length(uint32_t *pdata) {
    *pdata = 1523;
    return;
}

/**
 * @brief Counts number of images stored in the flash file system and transmits it over SPI
 *
 * Untested
 *
 */
void count_images() {
    // todo: @ron - can we consider implementing a function to count images in flash rather than iterating a local
    // counter
    SPI1_IT_Transmit(&total_image_num);
    return;
}

/**
 * @brief Sends arducam sensors into an idle (re: powered off) state by turning off FET driver pin
 *
 */
void sensor_idle() {
    /*
     * Todo: Look into SPI register 0x06 for idle power mode on the sensors. This shouldn't
     * 		 cut power to it and require reprogramming.
     */
    // pull mosfet driver pin low, cutting power to sensors
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_RESET);
    SPI1_IT_Transmit(&ack);

    return;
}

/**
 * @brief Activates sensors by enabling FET driver pin and programming to basic functionality
 *
 */
void sensor_active() {
    // pull mosfet driver pin high, powering sensors
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_SET);
    //	// initialize sensors
    initalize_sensors();
    return;
}

/**
 * @brief Get the housekeeping object and transmits it over SPI
 *
 */
void get_housekeeping(housekeeping_packet_t *hk) { *(hk) = _get_housekeeping(); }

/**
 * @brief   Updates sensor I2C regs based on input struct. Untested, and unknown if
 *          implementation will occur
 *
 */
void update_sensor_I2C_regs() {
    /*
     * Oh boy, this will be fun
     * 		Recieves:
     * 				- Target sensor (0x01 for VIS, 0x02 for NIR)
     * 				- Size of expected struct (uint32_t)
     * 				- Struct
     */
    // psuedoish code
    // uint8_t size = 0x00;
    // uint8_t target = 0x00;
    // SPI1_IT_Recieve(&target);
    // SPI1_IT_Recieve(&size);
    // housekeeping_packet_t packet[size];
    // struct = SPI1_IT_Recieve(&packet);
    // wrSensorRegs16_8(struct, target);
    return;
}

/**
 * @brief   Updates I2C regs on current sense IC chip from input struct.
 *          Needs to be written
 *
 */
void update_current_limits() { return; }

/**
 * @brief   Floods camera SPI with dummy data to non-addressable regs to init sensors
 *
 */
void flood_cam_spi() {
    for (uint8_t i = 0x60; i < 0x6F; i++) {
        read_spi_reg(i, VIS_SENSOR);
        read_spi_reg(i, NIR_SENSOR);
    }
}

/**
 * @brief Transfers images from arducams to nand flash. This will occur in an idle state
 *
 */
void _transfer_images_to_flash() {
    /*
     * todo: 	@RON, this is called at the end of take_image() after acking over spi.
     * 			This needs to transfer images from the flash buffer on the Arducam chip
     * 			from each of the VIS and NIR sensors to appropriate locations on the NAND
     * 			flash.
     */

    return;
}

/**
 * @brief   iterates internal image number by 2 (2 cameras take a picture, += 2)
 *
 */
void iterate_image_num() { total_image_num += 2; }

/**
 * @brief Get the total image number
 *
 * @param hk 1 for integer return (what's used for hk); 0 for spi return
 * @return uint8_t
 */
uint8_t get_image_num(uint8_t hk) {
    if (hk) {
        return total_image_num;
    }
    SPI1_IT_Transmit(&total_image_num);
    return 1;
}

/**
 * @brief Get the total image number
 *
 * @param hk 1 for integer return (what's used for hk); 0 for spi return
 * @return uint8_t
 */
uint8_t get_image_num_spi(uint8_t *num) {
    *num = total_image_num;
    return 1;
}

/*
 * Initializes sensors to our chosen defaults as defined in main.c
 * 	line 48/49
 */
void initalize_sensors(void) {
    uint8_t res = onboot_sensors(VIS_SENSOR);
	if (res == 1){
		program_sensor(format, VIS_SENSOR);
		DBG_PUT("VIS Camera Mode: JPEG\r\nI2C address: 0x3C\r\n\n");
		VIS_DETECTED = 1;
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
		NIR_DETECTED = 1;
	}
	if (res == -1){
		// need some error handling eh
		DBG_PUT("NIR init failed.\r\n");
		return;
	}
    HAL_Delay(100);

    // change resolution of sensors
    arducam_set_resolution(format, width, VIS_SENSOR);
    arducam_set_resolution(format, width, NIR_SENSOR);
    HAL_Delay(500);

}

/*
 * lower level wrapper function for stuff that needs to happen
 * to the sensors on boot.
 *
 * Param:
 * 		Sensor: Integer sensor identifier
 */
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

/*
 * watchdog timer handler. probably deprecated.
 */
void handle_wdt() { SPI1_IT_Transmit(&ack); }

static inline const char *next_token(const char *ptr) {
    /* move to the next space */
    while (*ptr && *ptr != ' ')
        ptr++;
    /* move past any whitespace */
    while (*ptr && isspace(*ptr))
        ptr++;

    return (*ptr) ? ptr : NULL;
}

/**
 * @brief UART command handler
 *
 * @param pointer to char command from main
 */
void uart_handle_command(char *cmd) {
    uint8_t in[sizeof(housekeeping_packet_t)];
    switch (*cmd) {
    case 'c':
        uart_handle_capture_cmd(cmd);
        //        take_image();
        break;
    case 'f':
        uart_handle_format_cmd(cmd);
        break;
    case 'w':
        uart_handle_width_cmd(cmd);
        break;
    case 's':
        switch (*(cmd + 1)) {
        case 'c':
            uart_scan_i2c();
            break;

        case 'a':;
            const char *c = next_token(cmd);
            switch (*c) {
            case 'v':
                uart_handle_saturation_cmd(c, VIS_SENSOR);
                break;
            case 'n':
                uart_handle_saturation_cmd(c, NIR_SENSOR);
                break;
            default:
                DBG_PUT("Target Error\r\n");
                break;
            }
        }
        break;

    case 'p':; // janky use of semicolon??
        const char *p = next_token(cmd);
        switch (*(p + 1)) {
        case 'n':
            sensor_active();
            break;
        case 'f':
            sensor_idle();
            break;
        default:
            DBG_PUT("Use either on or off\r\n");
            break;
        }
        break;
    case 'i':;
        switch (*(cmd + 1)) {
        case '2':
            handle_i2c16_8_cmd(cmd); // needs to handle 16 / 8 bit stuff
            break;
        default:;
            const char *i = next_token(cmd);
            switch (*i) {
            case 's':
                initalize_sensors();
                break;
            }
        }
        break;

    case 'h':
        switch (*(cmd + 1)) {
        case 'k':
            uart_get_hk_packet(in);
            break;
        default:
            help();
            break;
        }
    }
}
