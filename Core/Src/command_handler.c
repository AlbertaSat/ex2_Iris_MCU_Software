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

static void help() {
	// UART DEBUG ONLY
#ifdef UART_DEBUG
    DBG_PUT("TO RUN TESTS: test\r\n\n\n");
    DBG_PUT("Commands:\r\n");
    DBG_PUT("\tWorking/Tested:\r\n");
    DBG_PUT("\t\tcapture <vis/nir>\r\n");
    DBG_PUT("\t\tformat<vis/nir> [JPEG|BMP|RAW]\r\n");
    DBG_PUT("\t\treg <vis/nir> read <regnum>\r\n\treg write <regnum> <val>\r\n");
    DBG_PUT("\t\twidth  <vis/nir> [<pixels>]\r\n");
    DBG_PUT("\t\tpower on/off\r\n");
    DBG_PUT("\tscan Scan I2C bus 2\r\n");
    DBG_PUT("\tNeeds work\r\n");
    DBG_PUT("\t\tinit sensor Resets arducam modules to default\r\n");
    DBG_PUT("\t\tinit nand Initialize NAND Flash\r\n");
    DBG_PUT("\tNot tested/partially implemented:\r\n");
    DBG_PUT("\t\tSaturation [<0..8>]\r\n");
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
	while(!get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK, VIS_SENSOR) && !get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK, NIR_SENSOR)){}

	// ack over SPI
	SPI1_IT_Transmit(&ack);

	// keep track of how many images we have captured. This could come after transferring
	// to flash
	_iterate_image_number();

	// kick over images to our NAND flash
	_transfer_images_to_flash();

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

	// initialize sensors
	_initalize_sensor(VIS_SENSOR);
	_initalize_sensor(NIR_SENSOR);

	// program sensors
	_program_sensor(JPEG, VIS_SENSOR);
	_program_sensor(JPEG, NIR_SENSOR);

#ifdef UART_DEBUG
	DBG_PUT("Sensors active!");
#endif

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

void get_image_num(){
	SPI1_IT_Transmit(&total_image_num);
	return;
}

void _initalize_sensor(uint8_t sensor){
	  arducam_wait_for_ready(sensor);
	  write_reg(AC_REG_RESET, 1, sensor);
	  write_reg(AC_REG_RESET, 1, sensor);
	  HAL_Delay(100);
	  write_reg(AC_REG_RESET, 0, sensor);
	  HAL_Delay(100);

	  // todo: add error handling for spi if it fucks up

	  // if (!arducam_wait_for_ready(sensor))

	  write_reg(ARDUCHIP_MODE, 0x0, sensor);
	  wrSensorReg16_8(0xff, 0x01, sensor);

	  uint8_t vid = 0, pid = 0;
	  rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid, sensor);
	  rdSensorReg16_8(OV5642_CHIPID_LOW, &pid, sensor);

	  if (vid != 0x56 || pid != 0x42) {
		  // todo: error handler!
	  }
	  _program_sensor(JPEG, sensor);
}

void _program_sensor(uint8_t m_fmt, uint8_t sensor){
	if (m_fmt == RAW){
	        arducam_raw_init(1280, 960, sensor);
	    }
	else {
		wrSensorReg16_8(REG_SYS_CTL0, 0x82, sensor); // software reset
		wrSensorRegs16_8(OV5642_QVGA_Preview, sensor);
		HAL_Delay(100);

		if (m_fmt == JPEG) {
			HAL_Delay(100);

			wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA, sensor);
			wrSensorRegs16_8(OV5642_1600x1200, sensor); // changed from 320x240
			HAL_Delay(100);
			wrSensorReg16_8(0x3818, 0xa8, sensor);
			wrSensorReg16_8(0x3621, 0x10, sensor);
			wrSensorReg16_8(0x3801, 0xb0, sensor);
#if (defined(OV5642_MINI_5MP_PLUS) || (defined ARDUCAM_SHIELD_V2))
			wrSensorReg16_8(0x4407, 0x08, sensor);
#else
			wrSensorReg16_8(0x4407, 0x0C, sensor);
#endif
			wrSensorReg16_8(0x5888, 0x00, sensor);
			wrSensorReg16_8(0x5000, 0xFF, sensor);
	        }
		else  {
			byte reg_val;
			wrSensorReg16_8(0x4740, 0x21, sensor);
			wrSensorReg16_8(0x501e, 0x2a, sensor); // RGB Dither Ctl = RGB565/555
			wrSensorReg16_8(0x5002, 0xf8, sensor); // ISP Ctl 2 = Dither enable
			wrSensorReg16_8(0x501f, 0x01, sensor); // Format MUX Ctl = ISP RGB
			wrSensorReg16_8(0x4300, 0x61, sensor); // Format Ctl = RGB565
			rdSensorReg16_8(0x3818, &reg_val, sensor); // Timing Ctl = Mirror/Vertical flip
			wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff, sensor);
			rdSensorReg16_8(0x3621, &reg_val, sensor); // Array Ctl 01 = Horizontal bin
			wrSensorReg16_8(0x3621, reg_val & 0xdf, sensor);
	        }
	    }
}


void init_nand_flash(){
	FileHandle_t* file;
	NAND_ReturnType res = NAND_Init();
#ifdef UART_DEBUG
		if (res != Ret_Success){
			DBG_PUT("NAND Initialization failed\r\n");
		}
#endif
#ifdef SPI_DEBUG
		if(res != Ret_Success){
			// SPI_NACK
		}
#endif
		res = Ret_Failed;
		// format super block
		res =  NAND_File_Format(0);
#ifdef UART_DEBUG
		if (res != Ret_Success){
			DBG_PUT("NAND super block format failed\r\n");
		}
#endif
#ifdef SPI_DEBUG
		if(res != Ret_Success){
			// SPI_NACK
		}
#endif

		res = Ret_Failed;
		file = NAND_File_Create(0xAAAAAA);
		if (!file){
#ifdef UART_DEBUG
		DBG_PUT("NAND file creation failed\r\n");

#endif
#ifdef SPI_DEBUG
		// SPI_NACK

#endif
		}

		res = NAND_File_Write_Close(file);
#ifdef UART_DEBUG
		if (res != Ret_Success){
			DBG_PUT("NAND file write close failed\r\n");
		}
#endif
#ifdef SPI_DEBUG
		if(res != Ret_Success){
			// SPI_NACK
		}
#endif

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
    case 'g':
    	uart_get_hk_packet(&in);
    	break;
    case 'c':
    	uart_handle_capture_cmd(cmd);
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
    	for (int i=0; i<150; i++){
		testTempSensor();
    	HAL_Delay(1000);
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


    case 'h':
    default:
        help();
        break;
    }
}

