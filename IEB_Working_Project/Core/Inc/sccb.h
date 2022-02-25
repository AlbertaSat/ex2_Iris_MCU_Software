/*
 * sccb.h
 *
 *  Created on: Feb 7, 2022
 *      Author: Liam
 */

#ifndef INC_SCCB_H_
#define INC_SCCB_H_

int wrSensorReg16_8(uint16_t regID, uint8_t regDat, uint8_t sensor);

int wrSensorRegs16_8(const struct sensor_reg reglist[], uint8_t sensor);

int rdSensorReg16_8(uint16_t regID, uint8_t *regDat, uint8_t sensor);


#endif /* INC_SCCB_H_ */
