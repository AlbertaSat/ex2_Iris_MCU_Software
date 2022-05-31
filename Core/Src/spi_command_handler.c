#include <command_handler.h>

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
