/**
******************************************************************************
@brief 		Trafficlight functions, functions for Trafficlight-program
@file 		Trafficlight_functions.c
@author 		Anton Herdin Ringstedt
@version 1.0
@date 		13-December-2022
@brief 		Functions and structures for program Trafficlight
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stdbool.h"
#include "Test.h"
#include "Trafficlight_functions.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/**
@brief 		ShiftLED, sending buffer to Shift-registers using SPI
@param1 	ledbuffer, buffer containing data to transmit
@param2 	bytesize, size of data to transmit
@return 	void, no return value
*/
void ShiftLED(uint32_t ledbuffer[], uint8_t bytesize)
{
	HAL_SPI_Transmit(&hspi3, &ledbuffer[0], bytesize, 100);

	HAL_Delay(1);

	HAL_GPIO_WritePin(CLK_595_STCP_GPIO_Port, CLK_595_STCP_Pin, GPIO_PIN_SET);
	//transfer from buffer
	HAL_GPIO_WritePin(CLK_595_STCP_GPIO_Port, CLK_595_STCP_Pin, GPIO_PIN_RESET);
}

/**
@brief 		setLights, set traffic light bits (lane1 only)
@param1 	statebit, bits to shift into place
@return 	shift, the finished bit shifted value
*/
uint32_t setLights(uint8_t statebit)
{
	uint32_t shift = 0x000000;
	shift |= statebit << 8;
	shift |= statebit << 3;

	return shift;
}

/**
@brief 		setPedestrian, set pedestrian light bits
@param1 	ped1, bits to shift into place
@param2 	ped2, bits to shift into place
@return 	shift, the finished bit shifted value
*/
uint32_t setPedestrian(uint8_t ped1, uint8_t ped2)
{
	uint32_t shift = 0x000000;

	shift |= ped1 << 11;
	shift |= ped2 << 19;

	return shift;
}

/**
@brief 		readPed1, reads bits connected to ped1
@param1 	ped1, bits to read for ped1
@param2 	shift, buffer containing bits to read
@return1 	true, the bits were located
@return2 	false, the bits were not located
*/
uint32_t readPed1(uint8_t ped1, uint32_t shift)
{
	if(ped1 == ((shift >> 11)& 0x00000007))
		return true;
	return false;
}

/**
@brief 		readPed2, reads bits connected to ped2
@param1 	ped2, bits to read for ped2
@param2 	shift, buffer containing bits to read
@return1 	true, the bits were located
@return2 	false, the bits were not located
*/
uint32_t readPed2(uint8_t ped2, uint32_t shift)
{
	if(ped2 == ((shift >> 19)& 0x00000007))
		return true;
	return false;
}



