/*
 * shell2.c
 *
 *  Created on: Nov 12, 2023
 *      Author: Bapt
 */


#include "shell2.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


BMP280_U32_t Pressure_value;
BMP280_S32_t Temperature_value;
extern h_BMP280_t h_BMP280;
extern int K;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern int K;
extern uint8_t aData[2];
//extern BMP280_S32_t last_temp;


void protocol(char RX_Pi_buffer[RX_BUFFER_SIZE], uint8_t Size)
{
	//HAL_UART_Transmit(&huart2, RX_Pi_buffer, Size, 0xFFFF);
	if (strncmp(RX_Pi_buffer,"GET_T",strlen("GET_T"))==0)
	{
		Temperature_value=BMP280_get_temp(&h_BMP280);

		if (Temperature_value>=0)
		{
			printf("T=+%d.%d_C\r\n",(int)(Temperature_value/100),Temperature_value%100);
		}
		else
		{
			printf("T=%d.%d_C\r\n",(int)(Temperature_value/100),-Temperature_value%100);
		}
	}
	else if (strncmp(RX_Pi_buffer,"GET_P",strlen("GET_P"))==0)
	{
		Pressure_value=BMP280_get_pressure(&h_BMP280);
		printf("P=%dPa\r\n",Pressure_value);
	}
	else if (strncmp(RX_Pi_buffer,"SET_K",strlen("SET_K"))==0)
	{
		//printf("SET_K Received\r\n");
		if (strncmp(RX_Pi_buffer+strlen("SET_K"),"=",strlen("="))==0)
		{
			K=atoi(RX_Pi_buffer+strlen("SET_K")+strlen("="));
		}
		else
		{
			printf("Command not found\r\n"
					"Please try this syntax\r\n"
					"SET_K=xxxx: to set the K coefficient on xxxx value (in 1/100e)\r\n");
		}
	}
	else if (strncmp(RX_Pi_buffer,"GET_K",strlen("GET_K"))==0)
	{
		//printf("GET_K Received\r\n");
		printf("K=%d.%d000\r\n",(int)(K/100),K%100);
	}
	else if (strncmp(RX_Pi_buffer,"GET_A",strlen("GET_A"))==0)
	{
		//printf("GET_A Received\r\n");
		printf("A=%d.%d000\r\n",(int)(aData[0]/1000),aData[0]%1000);
	}
	else
	{
		printf("Command not found\r\n"
				"Try one of these commands:\r\n"
				"GET_T: to get the temperature value compensated\r\n"
				"GET_P: to get the pressure value compensated\r\n"
				"SET_K=xxxx: to set the K coefficient on xxxx value (in 1/100e)\r\n"
				"GET_K: to get the K coefficient\r\n"
				"GET_A: to get the angle\r\n");
	}
	memset(RX_Pi_buffer,NULL,Size*sizeof(char));
}
