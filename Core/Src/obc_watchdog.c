/*
 * obc_watchdog.c
 *
 *  Created on: Sep 26, 2022
 *      Author: joshd
 */
#include "can.h"
#include "iris_system.h"

#define OBC_WATCHDOG_PERIOD_MS 600000 //10 minutes
extern TIM_HandleTypeDef htim3;

//Also note: beware of leading zeroes on hex-binary conversion?

//frame 1
//0b010000010100011000001001000001000110001000101000001010000011001110000010000011000001000001001001100000100000100111101100011110010101
//0x4146090462282833820C10498209EC795

//Gap between commands = 10us = 10 bits = 0b1111111111

//frame 2
//0b0100000101001110000010000010000010110000111100000100000100111000001000110000111001101010100101101001011100000110110010101
//0x829C104161E08270461CD52D2E0D95

#define OBC_RESET_CMD_1_BITS 132
#define OBC_RESET_CMD_DELAY_BITS 10
#define OBC_RESET_CMD_2_BITS 121
#define UINT32_SIZE_BITS 32
uint32_t obc_reset_cmd_1[OBC_RESET_CMD_1_BITS/UINT32_SIZE_BITS+1] = {0x4, 0x14609046, 0x22828338, 0x20C10498, 0x209EC795};
uint32_t obc_reset_cmd_2[OBC_RESET_CMD_2_BITS/UINT32_SIZE_BITS+1] = {0x829C10, 0x4161E082, 0x70461CD5, 0x2D2E0D95};


typedef enum {
	OK = 0,
	NO_RESPONSE = 1
} ping_response;

ping_response pingOBC();
void send_obc_reset_cmd_to_eps();

void check_obc_watchdog(){
	if(__HAL_TIM_GET_COUNTER(&htim3) > 1000*OBC_WATCHDOG_PERIOD_MS){
		__HAL_TIM_SET_COUNTER(&htim3, 0);
		if(pingOBC() == NO_RESPONSE){
			send_obc_reset_cmd_to_eps();
		}
	}
}


ping_response pingOBC(){

	HAL_GPIO_WritePin(GPIOA, ERR_Pin, GPIO_PIN_SET);

	int polling_start_time_ms = __HAL_TIM_GET_COUNTER(&htim3);
	int wait_time_ms = 50;
	while((__HAL_TIM_GET_COUNTER(&htim3) - polling_start_time_ms) < 1000*wait_time_ms){
		if(HAL_GPIO_ReadPin(GPIOA, WDI_Pin) == GPIO_PIN_SET){
			HAL_GPIO_WritePin(GPIOA, ERR_Pin, GPIO_PIN_RESET);
			return OK;
		}
	}
	HAL_GPIO_WritePin(GPIOA, ERR_Pin, GPIO_PIN_RESET);
	return NO_RESPONSE;
}


void send_obc_reset_cmd_to_eps(){

	//send first CAN frame
	send_can_frame(obc_reset_cmd_1, OBC_RESET_CMD_1_BITS);

    for (int i = 0; i < OBC_RESET_CMD_DELAY_BITS; i++) {
        CAN_TX_GPIO_Port->BRR = CAN_TX_Pin;
        delay_us(CAN_DELAY);
    }

    //send second CAN frame
    send_can_frame(obc_reset_cmd_2, OBC_RESET_CMD_2_BITS);

}
