/*
 * Housekeeping.c
 *
 *  Created on: Mar. 29, 2022
 *      Author: Liam Droog
 */
#include "main.h"
#include "tmp421.h"
#include "debug.h"
#include "housekeeping.h"



housekeeping_packet_t _get_housekeeping(){
	housekeeping_packet_t hk;
	hk.vis_temp = get_temp(VIS_TEMP_SENSOR);
	hk.nir_temp = get_temp(NIR_TEMP_SENSOR);
	hk.flash_temp = get_temp(TEMP3);
	hk.gate_temp = get_temp(TEMP4);
	hk.imagenum = get_image_num();
	hk.software_version = software_ver;
	return hk;
}



void decode_hk_packet (housekeeping_packet_t hk){
	char buf[64];
	sprintf(buf, "hk.vis_temp:0x%x\r\n", hk.vis_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.nir_temp: 0x%x\r\n", hk.nir_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.flash_temp: 0x%x\r\n", hk.flash_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.gate_temp: 0x%x\r\n", hk.gate_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.imgnum: 0x%x\r\n", hk.imagenum);
	DBG_PUT(buf);
	sprintf(buf, "hk.software_version: 0x%x\r\n", hk.software_version);
	DBG_PUT(buf);
}


