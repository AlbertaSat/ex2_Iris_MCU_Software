/*
 * Housekeeping.c
 *
 *  Created on: Mar. 29, 2022
 *      Author: Liam Droog
 */
#include <stdio.h>

#include "iris_system.h"
#include "command_handler.h"
#include "housekeeping.h"
#include "debug.h"
#include "tmp421.h"
#include "ina209.h"

housekeeping_packet_t _get_housekeeping() {
    housekeeping_packet_t hk;
    uint8_t image_count;
    get_image_count(&image_count);

    hk.vis_temp = get_temp(VIS_TEMP_SENSOR);
    hk.nir_temp = get_temp(NIR_TEMP_SENSOR);
    hk.flash_temp = get_temp(FLASH_TEMP_SENSOR);
    hk.gate_temp = get_temp(GATE_TEMP_SENSOR);
    hk.imagenum = image_count;
    hk.software_version = software_ver;
    hk.errornum = get_error_num();

#if defined IRIS_EM || defined IRIS_FM
    uint16_t pospeak, pwrpeak, negpeak;
    // 5V current sense exists.
    get_shunt_voltage_peak_pos(CURRENTSENSE_5V, &pospeak);
    get_power_peak(CURRENTSENSE_5V, &pwrpeak);
    get_shunt_voltage_peak_neg(CURRENTSENSE_5V, &negpeak);
    hk.MAX_5V_voltage = pospeak;
    hk.MAX_5V_power = pwrpeak;
    hk.MIN_5V_voltage = negpeak;
#else
    // 5V current sense does not exist (proto / old Iris)
    hk.MAX_5V_voltage = 0xDEAD;
    hk.MAX_5V_power = 0xBEEF;
    hk.MIN_5V_voltage = 0xBABE;
#endif // IRIS_EM || IRIS_FM

#ifdef IRIS_FM
    // 3V3 current sense exists
    get_shunt_voltage_peak_pos(CURRENTSENSE_3V3, &pospeak);
    get_power_peak(CURRENTSENSE_3V3, &pwrpeak);
    get_shunt_voltage_peak_neg(CURRENTSENSE_3V3, &negpeak);
    hk.MAX_3V_voltage = pospeak;
    hk.MAX_3V_power = pwrpeak;
    hk.MIN_3V_voltage = negpeak;
#else
    // 3v3 current sense does not exist (proto / old / EM)
    hk.MAX_3V_voltage = 0xDEAD;
    hk.MAX_3V_power = 0xBEEF;
    hk.MIN_3V_voltage = 0xBABE;
#endif // IRIS_FM
    return hk;
}

/**
 * @brief Decodes hk packet for debug purposes. Prints output over UART
 *
 * @param hk housekeeping_packet_t
 */
void decode_hk_packet(housekeeping_packet_t hk) {
    char buf[64];
    sprintf(buf, "hk.vis_temp:0x%x, %d.%04d C\r\n", hk.vis_temp, (hk.vis_temp >> 8) - 64,
            ((hk.vis_temp & 0xFF) >> 4) * 625);
    iris_log(buf);
    sprintf(buf, "hk.nir_temp:0x%x, %d.%04d C\r\n", hk.nir_temp, (hk.nir_temp >> 8) - 64,
            ((hk.nir_temp & 0xFF) >> 4) * 625);
    iris_log(buf);
    sprintf(buf, "hk.flash_temp:0x%x, %d.%04d C\r\n", hk.flash_temp, (hk.flash_temp >> 8) - 64,
            ((hk.flash_temp & 0xFF) >> 4) * 625);
    iris_log(buf);
    sprintf(buf, "hk.gate_temp:0x%x, %d.%04d C\r\n", hk.gate_temp, (hk.gate_temp >> 8) - 64,
            ((hk.gate_temp & 0xFF) >> 4) * 625);
    iris_log(buf);
    sprintf(buf, "hk.imgnum: 0x%x\r\n", hk.imagenum);
    iris_log(buf);
    sprintf(buf, "hk.software_version: 0x%x\r\n", hk.software_version);
    iris_log(buf);
    sprintf(buf, "hk.MAX_5V_voltage: 0x%x\r\n", hk.MAX_5V_voltage);
    iris_log(buf);
    sprintf(buf, "hk.MAX_3V_voltage: 0x%x\r\n", hk.MAX_3V_voltage);
    iris_log(buf);
    sprintf(buf, "hk.MIN_5V_voltage: 0x%x\r\n", hk.MIN_5V_voltage);
    iris_log(buf);
    sprintf(buf, "hk.MIN_3V_voltage: 0x%x\r\n", hk.MIN_3V_voltage);
    iris_log(buf);
    sprintf(buf, "hk.MAX_5V_power: 0x%x\r\n", hk.MAX_5V_power);
    iris_log(buf);
    sprintf(buf, "hk.MAX_3V_power: 0x%x\r\n", hk.MAX_3V_power);
    iris_log(buf);
}
