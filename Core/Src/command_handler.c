/*
 * command_handler.c
 *
 *  Created on: May 9, 2022
 *      Author: Liam
 */
#include "command_handler.h"
#include "string.h"
#include "debug.h"
#include "nand_m79a.h"
#include "arducam.h"
#include "SPI_IT.h"
extern SPI_HandleTypeDef hspi1;
extern const struct sensor_reg OV5642_JPEG_Capture_QSXGA[];
extern const struct sensor_reg OV5642_QVGA_Preview[];
/*
 * todo:
 * 		- 	TEST THESE FUNCTIONS EH
 *
 * 		- 	Determine if the  SPI CltCallback in main.c are called from interrupt SPI
 * 			functions in here, or if they're needed in here / in SPI_IT.c
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

int format = JPEG;


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

    // ack over SPI
    SPI1_IT_Transmit(&ack);

	//todo:
    // keep track of how many images we have captured. This could come after transferring
    // to flash
    //	_iterate_image_number();

    // kick over images to our NAND flash
    //	_transfer_images_to_flash();

    return;
}

void get_image_length() {
    // todo:	@RON:   Need a way to get image length from NAND flash
    //  				- Expecting a 32 bit integer for image size
    uint32_t image_length = 0x000000;
    SPI1_IT_Transmit(&image_length);
    return;
}

void count_images() {
    // todo: @ron - can we consider implementing a function to count images in flash rather than iterating a local
    // counter
    SPI1_IT_Transmit(&total_image_num);
    return;
}

/*
 * Todo: Look into SPI register 0x06 for idle power mode on the sensors. This shouldn't
 * 		 cut power to it and require reprogramming.
 */
void sensor_idle() {
    // pull mosfet driver pin low, cutting power to sensors
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_RESET);
	SPI1_IT_Transmit(&ack);

    return;
}

void sensor_active() {
    // pull mosfet driver pin high, powering sensors
    HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_SET);
    DBG_PUT("Initializing Sensors\r\n");
    //	// initialize sensors
    print_progress(1, 5);
    _initalize_sensor(VIS_SENSOR);
    print_progress(3, 5);
    _initalize_sensor(NIR_SENSOR);
    print_progress(5, 5);

#ifdef SPI_DEBUG
//	SPI1_IT_Transmit(&ack);
#endif

    return;
}

void get_housekeeping() {
    hk = _get_housekeeping();
    char buffer[sizeof(hk)];
    memcpy(buffer, &hk, sizeof(hk));
    SPI1_IT_Transmit((uint8_t *)buffer); // not sure this is how it's supposed to work
    return;
}

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

void update_current_limits() { return; }

void _transfer_images_to_flash() {
    /*
     * todo: 	@RON, this is called at the end of take_image() after acking over spi.
     * 			This needs to transfer images from the flash buffer on the Arducam chip
     * 			from each of the VIS and NIR sensors to appropriate locations on the NAND
     * 			flash.
     */

    return;
}

void iterate_image_num() { total_image_num += 2; }

uint8_t get_image_num(uint8_t hk) {
    // param hk: 1 for integer return,
    // 			 0 for spi transmit.
    if (hk) {
        return total_image_num;
    }
    SPI1_IT_Transmit(&total_image_num);
    return 1;
}

void _initalize_sensor(uint8_t sensor) {

    char buf[64];
    uint8_t DETECTED = 0;
    arducam_wait_for_ready(sensor);
    write_reg(AC_REG_RESET, 1, sensor);
    write_reg(AC_REG_RESET, 1, sensor);
    HAL_Delay(100);
    write_reg(AC_REG_RESET, 0, sensor);
    HAL_Delay(100);

    if (!arducam_wait_for_ready(sensor)) {
        DBG_PUT("Sensor: SPI Unavailable\r\n");
    }

    // Change MCU mode
    write_reg(ARDUCHIP_MODE, 0x0, sensor);
    wrSensorReg16_8(0xff, 0x01, sensor);

    uint8_t vid = 0, pid = 0;
    rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid, sensor);
    rdSensorReg16_8(OV5642_CHIPID_LOW, &pid, sensor);

    if (vid != 0x56 || pid != 0x42) {
        sprintf(buf, "Sensor not available\r\n\n");
        DBG_PUT(buf);

    } else {
        DETECTED = 1;
    }
    if (DETECTED == 1) {
        format = JPEG;
        Arduino_init(format, sensor);
        DBG_PUT(buf);
    }
}
