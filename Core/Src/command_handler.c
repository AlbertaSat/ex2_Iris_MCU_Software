/*
 * command_handler.c
 *
 *  Created on: May 9, 2022
 *      Author: Liam
 */
#include "command_handler.h"
#include "string.h"
#include "debug.h"
#include "uart_command_handler.h"
#include "nand_m79a.h"
#include "arducam.h"
#include "IEB_TESTS.h"

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

void help() {
	// UART DEBUG ONLY
#ifdef UART_DEBUG
	DBG_PUT("Test\t|\tCommand\r\n");
	DBG_PUT("3.2.2\t>\tteset\r\n");
	DBG_PUT("3.2.5\t>\ttemp\r\n");
	DBG_PUT("3.2.6\t>\tnand test\r\n");
#endif
}

void take_image(){
	/*
	 * Todo: Determine whether or not we want to have individual sensor control, or just cap both at the same time (ish)
	 * 		 Fix Arducam.h so we stop with these warnings
	 */
	write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, VIS_SENSOR);   //VSYNC is active HIGH
	write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK, NIR_SENSOR);

	flush_fifo(VIS_SENSOR);
	flush_fifo(NIR_SENSOR);

	clear_fifo_flag(VIS_SENSOR);
	clear_fifo_flag(NIR_SENSOR);

	start_capture(VIS_SENSOR);
	start_capture(NIR_SENSOR);

	// todo: determine if cap_done_mask stays high for subsequent reads of arducam_trig register. Otherwise this loop
	//		 will never break
	DBG_PUT("listening for cap done mask\r\n");
	while(!get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK, VIS_SENSOR)){}
	DBG_PUT("vis sensor complete\r\n");
	while(!get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, NIR_SENSOR)){}
	DBG_PUT("Loop broke!\r\n");;

	// ack over SPI
	SPI1_IT_Transmit(&ack);

	// keep track of how many images we have captured. This could come after transferring
	// to flash
//	_iterate_image_number();

	// kick over images to our NAND flash
//	_transfer_images_to_flash();

	return;
}

void transfer_image(){
	/*
	 * Todo: this will be a bruh
	 * 			- Buffer images into 512 byte chunks
	 * 			- Transmission inludes 1 start byte + 512 image bytes + 1 end byte
	 * 				- total 514 bytes
	 * 			Flow:
	 * 				- Buffer 512 bytes of image in to memory
	 * 				- send header byte (not yet confirmed)
	 * 				- clock out 512 byte chunk
	 * 				- clock FF until next 512 byte chunk is ready
	 * 				- send footer byte (not yet confirmed)
	 * 				rinse and repeat.
	 *
	 */
	return;
}

void get_image_length(){
	// todo:	@RON:   Need a way to get image length from NAND flash
	//  				- Expecting a 32 bit integer for image size
	uint32_t image_length = 0x000000;
	SPI1_IT_Transmit(&image_length);
	return;
}

void count_images(){
	//todo: @ron - can we consider implementing a function to count images in flash rather than iterating a local counter
	SPI1_IT_Transmit(&total_image_num);
	return;
}

/*
 * Todo: Look into SPI register 0x06 for idle power mode on the sensors. This shouldn't
 * 		 cut power to it and require reprogramming.
 */
void sensor_idle(){
	// pull mosfet driver pin low, cutting power to sensors
	HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_RESET);
//	SPI1_IT_Transmit(&ack);

	return;
}

void sensor_active(){
	// pull mosfet driver pin high, powering sensors
	HAL_GPIO_WritePin(CAM_EN_GPIO_Port, CAM_EN_Pin, GPIO_PIN_SET);
	DBG_PUT("Initializing Sensors\r\n");
//	// initialize sensors
	print_progress(1, 5);
	_initialize_sensor(VIS_SENSOR);
	print_progress(3, 5);
	_initialize_sensor(NIR_SENSOR);
	print_progress(5, 5);


#ifdef SPI_DEBUG
//	SPI1_IT_Transmit(&ack);
#endif

	return;
}

void get_housekeeping(){
	hk = _get_housekeeping();
	char buffer[sizeof(hk)];
	memcpy(buffer, &hk, sizeof(hk));
	SPI1_IT_Transmit((uint8_t *) buffer); // not sure this is how it's supposed to work
	return;
}

void update_sensor_I2C_regs(){
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

void update_current_limits(){
	return;
}

void _transfer_images_to_flash(){
	/*
	 * todo: 	@RON, this is called at the end of take_image() after acking over spi.
	 * 			This needs to transfer images from the flash buffer on the Arducam chip
	 * 			from each of the VIS and NIR sensors to appropriate locations on the NAND
	 * 			flash.
	 */

	return;
}

void iterate_image_num(){
	total_image_num += 2;
}

uint8_t get_image_num(uint8_t hk){
	// param hk: 1 for integer return,
	// 			 0 for spi transmit.
	if (hk){
		return total_image_num;
	}
	SPI1_IT_Transmit(&total_image_num);
	return 1;
}

void _initialize_sensor(uint8_t sensor){
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

	    }
	    else{
	    	DETECTED = 1;
	    }
	    if (DETECTED==1){
			format = JPEG;
			Arduino_init(format, sensor);
			DBG_PUT(buf);
	    }

}

void program_sensor(uint8_t sensor, struct sensor_reg newprogram[]){
	wrSensorRegs16_8(newprogram, sensor);
	return;
}
void init_nand_flash(){
	FileHandle_t* file;
	NAND_ReturnType res = NAND_Init();
	if (res == Ret_Success){
		DBG_PUT("NAND Flash Initialized Successfully\r\n");
	}
	else if(res == Ret_ResetFailed){
		DBG_PUT("NAND Reset Failed\r\n");
	}
	else if(res == Ret_WrongID){
		DBG_PUT("NAND ID is wrong\r\n");
	}
	else{
		DBG_PUT("Something else is wrong wit the NAND Flash\r\n");
	}

	res = Ret_Failed;
	// format super block
	res =  NAND_File_Format(0);
	if (res == Ret_Success){
		DBG_PUT("NAND Flash File Format Success\r\n");
	}
	else if(res == Ret_WriteFailed){
		DBG_PUT("NAND Write Failed\r\n");
	}
	else if(res == Ret_Failed){
		DBG_PUT("Reset failed\r\n");
	}
	else{
		DBG_PUT("Something else went wrong\r\n");
	}

	res = Ret_Failed;
	file = NAND_File_Create(0xAAAAAA);
	if (!file){
		DBG_PUT("FS is not formatted!\r\n");
	}

	res = Ret_Failed;
	res = NAND_File_Write_Close(file);
	if (res == Ret_Success){
		DBG_PUT("NAND Flash Write_Close Successful\r\n");
	}
	else if(res == Ret_WriteFailed){
		DBG_PUT("NAND Write Failed\r\n");
	}
	else if(res == Ret_Failed){
		DBG_PUT("Reset failed after writing\r\n");
	}
	else{
		DBG_PUT("Something else went wrong during write_close\r\n");
	}
}

void spi_handle_command(uint8_t cmd) {
    switch(cmd) {
    case GET_HK:
//    	get_housekeeping();
    	break;
    case CAPTURE_IMAGE:
//    	handle_capture_cmd(cmd);
    	iterate_image_num();
    	break;
    case GET_IMAGE_NUM:
//    	get_image_num();
    	break;
    case COUNT_IMAGES:
//    	count_images();
    	break;
	case SENSOR_ACTIVE:
		sensor_active();
		break;
	case SENSOR_IDLE:
		sensor_idle();
		break;
	}
}

void print_progress(uint8_t count, uint8_t max)
{
	uint8_t length = 25;
	uint8_t scaled = count*100 / max * length / 100;

    sprintf(buf, "Progress: [%.*s%.*s]\r", scaled, "==================================================", length - scaled, "                                        ");
	DBG_PUT(buf);
	if (count == max){
		DBG_PUT("\r\n");
	}
}


static inline const char* next_token(const char *ptr) {
    /* move to the next space */
    while(*ptr && *ptr != ' ') ptr++;
    /* move past any whitespace */
    while(*ptr && isspace(*ptr)) ptr++;

    return (*ptr) ? ptr : NULL;
}


void uart_handle_command(char *cmd) {
	uint8_t in[sizeof(housekeeping_packet_t)];
    switch(*cmd) {
    case 'c':
//    	uart_handle_capture_cmd(cmd);
    	take_image();
    	break;
    case 'f':
    	uart_handle_format_cmd(cmd);
        break;

    case 'r':
    	read_nand_flash();
//		handle_reg_cmd(cmd);
		break;

    case 'w':
    	uart_handle_width_cmd(cmd);
        break;
    case 't':
    	switch(*(cmd+2)){
    	case 's':
        	CHECK_LED_I2C_SPI_TS_NAND();
        	break;
    	case 'm':
    		testTempSensor();
    		break;
    	}
    	break;
    case 's':
    	switch(*(cmd+1)){
			case 'c':
				uart_scan_i2c();
				break;

			case 'a':;
				const char *c = next_token(cmd);
				switch(*c){
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

    case 'p':	; //janky use of semicolon??
    	const char *p = next_token(cmd);
    	switch(*(p+1)){
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
		switch(*(cmd+1)){
			case '2':
				handle_i2c16_8_cmd(cmd); // needs to handle 16 / 8 bit stuff
				break;
			default:;
				const char *i = next_token(cmd);
				switch(*i){
					case 'n':
						init_nand_flash();
						break;
					case 's':
						uart_reset_sensors();
						break;
				}
		}
		break;

	case 'n':
		test_nand_flash();
		break;
    case 'h':
    	switch(*(cmd+1)){
    	case 'k':
        	uart_get_hk_packet(&in);
        	break;
        default:
            help();
            break;

    	}
    	break;
    }
}

