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



housekeeping_packet_t get_housekeeping(){
	housekeeping_packet_t hk;
	hk.vis_temp = get_temp(VIS_SENSOR);
	hk.nir_temp = get_temp(NIR_SENSOR);
	hk.flash_temp = get_temp(TEMP3);
	hk.gate_temp = get_temp(TEMP4);
	hk.imagenum = 0;
	hk.software_version = 0x01;
	return hk;
}



void decode_hk_packet (housekeeping_packet_t hk){
	char buf[64];
	sprintf(buf, "hk.vis_temp: 0x%x", hk.vis_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.nir_temp: 0x%x", hk.nir_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.flash_temp: 0x%x", hk.flash_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.gate_temp: 0x%x", hk.gate_temp);
	DBG_PUT(buf);
	sprintf(buf, "gk.imgnum: 0x%x", hk.vis_temp);
	DBG_PUT(buf);
	sprintf(buf, "hk.software_version: 0x%x", hk.vis_temp);
	DBG_PUT(buf);
}
