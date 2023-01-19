/**
******************************************************************************
@brief 		Crossing functions, functions for Crossing-program
@file 		Crossing_functions.c
@author 		Anton Herdin Ringstedt
@version 1.0
@date 		13-December-2022
@brief 		Functions and structures for program Crossing
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stdbool.h"
#include "Test.h"
#include "Crossing_functions.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/**
@brief 		setTraffic, shifting bits into place
@param1 	lane1, bits to shift in for lane1
@param2 	lane2, bits to shift in for lane2
@return 	shift, the finished bit shifted value
*/
uint32_t setTraffic(uint8_t lane1, uint8_t lane2)
{
	uint32_t shift = 0x000000;

	//lane 1
	shift |= lane1 << 8;		//light car 2
	shift |= lane1 << 3;		//light car 4

	//lane 2
	shift |= lane2;				//light car 3
	shift |= lane2 << 16;		//light car 1

	return shift;
}

/**
@brief 		readLane1, reads bits connected to lane1
@param1 	lane1, bits to read for lane1
@param2 	shift, buffer containing bits to read
@return1 	true, the bits were located
@return2 	false, the bits were not located
*/
bool readLane1(uint8_t lane1, uint32_t shift)
{
	if( lane1 == ((shift >> 8)& 0x00000007) && lane1 == ((shift >> 3)& 0x00000007))
		return true;
	return false;
}

/**
@brief 		readLane2, reads bits connected to lane2
@param1 	lane2, bits to read for lane2
@param2 	shift, buffer containing bits to read
@return1 	true, the bits were located
@return2 	false, the bits were not located
*/
bool readLane2(uint8_t lane2, uint32_t shift)
{
	if( lane2 == (shift & 0x00000007) && lane2 == ((shift >> 16)& 0x00000007))		//light car 3
		return true;									//light car 1
	return false;
}




