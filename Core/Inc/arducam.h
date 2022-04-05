#ifndef ARDUCAM_DEFH
#define ARDUCAM_DEFH

#include "arducam.h"
#include <stdbool.h>
#include <stdio.h>
#define byte uint8_t
#define AC_REG_TEST 0
#define AC_REG_CAPTURE_CTL 1
#define AC_REG_FIFO_CTL 4
#define AC_REG_TEST_MODE 5
#define AC_REG_RESET 7
#define BMP 	    0
#define JPEG	    1
#define RAW 		2
#define OV5642_CHIPID_HIGH 0x300a
#define OV5642_CHIPID_LOW  0x300b


#define OV5642_320x240 		0	//320x240
#define OV5642_640x480		1	//640x480
#define OV5642_1024x768		2	//1024x768
#define OV5642_1280x960 	3	//1280x960
#define OV5642_1600x1200	4	//1600x1200
#define OV5642_2048x1536	5	//2048x1536
#define OV5642_2592x1944	6	//2592x1944

#define ARDUCHIP_MODE      		0x02  //Mode register
#define ARDUCHIP_FIFO      		0x04  //FIFO and I2C control
#define FIFO_CLEAR_MASK    		0x01
#define FIFO_START_MASK    		0x02
#define FIFO_RDPTR_RST_MASK     0x10
#define FIFO_WRPTR_RST_MASK     0x20

#define ARDUCHIP_TRIG      		0x41  //Trigger source
#define VSYNC_MASK         		0x01
#define SHUTTER_MASK       		0x02
#define CAP_DONE_MASK      		0x08

#define FIFO_SIZE1				0x42  //Camera write FIFO size[7:0] for burst to read
#define FIFO_SIZE2				0x43  //Camera write FIFO size[15:8]
#define FIFO_SIZE3				0x44  //Camera write FIFO size[18:16]

#define BURST_FIFO_READ			0x3C  //Burst FIFO read operation
#define SINGLE_FIFO_READ		0x3D  //Single FIFO read operation
#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
#define ARDUCHIP_TIM       		0x03  //Timming control
#define ARDUCHIP_FRAMES			0x01  //FRAME control register, Bit[2:0] = Number of frames to be captured																		//On 5MP_Plus platforms bit[2:0] = 7 means continuous capture until frame buffer is full

struct sensor_reg {
	uint16_t reg;
	uint16_t val;
};

void Arduino_init(int m_fmt, int sensor);
bool arducam_wait_for_ready(uint8_t sensor);
void arducam_raw_init(int width, int depth, uint8_t sensor);
void arducam_get_resolution(int *width, int *depth, uint8_t sensor);
int arducam_set_resolution(int format, int width, uint8_t sensor);

void SingleCapTransfer(int fmt, uint8_t sensor);

bool write_spi_reg(uint8_t reg, uint8_t val, uint8_t sensor);
uint8_t read_spi_reg(uint8_t reg, uint8_t sensor);

void write_reg(uint8_t addr, uint8_t data, uint8_t sensor);
uint8_t read_reg(uint8_t addr, uint8_t sensor);


#endif // ARDUCAM_DEFH
