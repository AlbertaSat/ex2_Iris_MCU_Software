#ifndef CLI_DEFH
#define CLI_DEFH


#define GET_IMAGE_NUM	0x05
#define CAPTURE_IMAGE 	0x10
#define COUNT_IMAGES 	0x20
#define SENSOR_IDLE 	0x30
#define SENSOR_ACTIVE	0x40
#define GET_HK			0x50
#define I2C_COMPLEX_SHIT 0x69

void handle_command(uint8_t cmd);
void reset_sensors(void);
extern int format;
void scan_i2c();
void reset_sensors(void);
void sensor_togglepower(int i);

#endif // CLI_DEFH
