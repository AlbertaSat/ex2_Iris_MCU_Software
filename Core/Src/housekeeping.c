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
#include "command_handler.h"
#include "ina209.h"

housekeeping_packet_t _get_housekeeping() {
    housekeeping_packet_t hk;
    hk.vis_temp = get_temp(VIS_TEMP_SENSOR);
    hk.nir_temp = get_temp(NIR_TEMP_SENSOR);
	hk.flash_temp = get_temp(FLASH_TEMP_SENSOR);
	hk.gate_temp = get_temp(GATE_TEMP_SENSOR);
    hk.imagenum = get_image_num(1);
    hk.software_version = software_ver;
//	hk.MAX_5V_voltage = get_shunt_voltage_peak_pos(CURRENTSENSE_5V);
//	hk.MAX_5V_power  = get_power_peak(CURRENTSENSE_5V);
//	hk.MAX_3V_voltage = get_shunt_voltage_peak_pos(CURRENTSENSE_3V3);
//	hk.MAX_3V_power = get_power_peak(CURRENTSENSE_3V3);
//	hk.MIN_5V_voltage = get_shunt_voltage_peak_neg(CURRENTSENSE_5V);
//	hk.MIN_3V_voltage = get_shunt_voltage_peak_neg(CURRENTSENSE_3V3);
    return hk;
}

void decode_hk_packet(housekeeping_packet_t hk) {
    char buf[64];
    sprintf(buf, "hk.vis_temp:0x%x, %d.%04d C\r\n", hk.vis_temp, (hk.vis_temp >> 8) - 64,
            ((hk.vis_temp & 0xFF) >> 4) * 625);
    DBG_PUT(buf);
    sprintf(buf, "hk.nir_temp:0x%x, %d.%04d C\r\n", hk.nir_temp, (hk.nir_temp >> 8) - 64,
            ((hk.nir_temp & 0xFF) >> 4) * 625);
    DBG_PUT(buf);
    sprintf(buf, "hk.flash_temp:0x%x, %d.%04d C\r\n", hk.flash_temp, (hk.flash_temp >> 8) - 64,
            ((hk.flash_temp & 0xFF) >> 4) * 625);
    DBG_PUT(buf);
    sprintf(buf, "hk.gate_temp:0x%x, %d.%04d C\r\n", hk.gate_temp, (hk.gate_temp >> 8) - 64,
            ((hk.gate_temp & 0xFF) >> 4) * 625);
    DBG_PUT(buf);
    sprintf(buf, "hk.imgnum: 0x%x\r\n", hk.imagenum);
    DBG_PUT(buf);
    sprintf(buf, "hk.software_version: 0x%x\r\n", hk.software_version);
    DBG_PUT(buf);
    sprintf(buf, "hk.MAX_5V_voltage: 0x%x\r\n", hk.MAX_5V_voltage);
    DBG_PUT(buf);
    sprintf(buf, "hk.MAX_3V_voltage: 0x%x\r\n", hk.MAX_3V_voltage);
    DBG_PUT(buf);
    sprintf(buf, "hk.MIN_5V_voltage: 0x%x\r\n", hk.MIN_5V_voltage);
    DBG_PUT(buf);
    sprintf(buf, "hk.MIN_3V_voltage: 0x%x\r\n", hk.MIN_3V_voltage);
    DBG_PUT(buf);
    sprintf(buf, "hk.MAX_5V_power: 0x%x\r\n", hk.MAX_5V_power);
    DBG_PUT(buf);
    sprintf(buf, "hk.MAX_3V_power: 0x%x\r\n", hk.MAX_3V_power);
    DBG_PUT(buf);
}
