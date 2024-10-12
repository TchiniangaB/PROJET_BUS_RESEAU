/*
 * BMP280.c
 *
 *  Created on: 10 oct. 2023
 *      Author: Tchinianga Bastien
 */

#include "BMP280.h"
#include <stdio.h>

short dig_T1 = 0;
signed short dig_T2 = 0;
signed short dig_T3 = 0;
short dig_P1 = 0;
signed short dig_P2 = 0;
signed short dig_P3 = 0;
signed short dig_P4 = 0;
signed short dig_P5 = 0;
signed short dig_P6 = 0;
signed short dig_P7 = 0;
signed short dig_P8 = 0;
signed short dig_P9 = 0;
uint32_t t_fine;


int BMP280_init(h_BMP280_t * h_BMP280)
{

	//Récupération de l'ID du composant
	uint8_t chip_id = BMP280_CHIP_ID_t;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &chip_id, 2);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, &chip_id, 2);
	if(chip_id != 0x58)
	{
		printf("Incorrect ID\r\n");
		return 1;
	}
	h_BMP280->chip = chip_id;

	//Reset du composant
	uint8_t reset = BMP280_RESET;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &reset, 1);


	//Configuration du composant
	//uint8_t power_mode = BMP280_SAMPLE_2|BMP280_SAMPLE_16|BMP280_NORMAL_MODE ;
	uint8_t power_mode = (0b010 <<5)|(0b101 <<2)|(0b11);
	uint8_t buffer[2];
	buffer[0] = BMP280_CONTROL;
	buffer[1] = power_mode;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, buffer, 2);
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, buffer, 1);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 1);
	if (buffer[0] == power_mode){
		printf("\r\nConfiguration done\r\n");
		h_BMP280->power = BMP280_NORMAL_MODE ;
		h_BMP280->sample = BMP280_SAMPLE_2|BMP280_SAMPLE_16 ;
	}
	return 0;
}


void BMP280_etalonnage(h_BMP280_t * h_BMP280)
{
	uint8_t buffer[24];
	uint8_t etalonage = BMP250_CALIB;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &etalonage, 1);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 24);
	printf("Valeurs d'étalonage : \r\n");
	for(int i=0;i<24;i++){
		printf("calib %2d = 0x%x\r\n",i,buffer[i]);
	}
	dig_T1 = buffer[0]|(buffer[1]<<8);
	dig_T2 = buffer[2]|(buffer[3]<<8);
	dig_T3 = buffer[4]|(buffer[5]<<8);
	dig_P1 = buffer[6]|(buffer[7]<<8);
	dig_P2 = buffer[8]|(buffer[9]<<8);
	dig_P3 = buffer[10]|(buffer[11]<<8);
	dig_P4 = buffer[12]|(buffer[13]<<8);
	dig_P5 = buffer[14]|(buffer[15]<<8);
	dig_P6 = buffer[16]|(buffer[17]<<8);
	dig_P7 = buffer[18]|(buffer[19]<<8);
	dig_P8 = buffer[20]|(buffer[21]<<8);
	dig_P9 = buffer[22]|(buffer[23]<<8);
}

int BMP280_get_temp(h_BMP280_t * h_BMP280)
{
	uint32_t T;
	BMP280_S32_t Temperature_value_compensated;
	uint8_t buffer[3];
	uint8_t temp=BMP280_TEMP_MSB;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &temp, 1);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 3);
	if (buffer[0]==0){
		printf("Erreur de lecture\r\n");
		return 1;
	}
	T=((BMP280_S32_t)buffer[0]<<12)+((BMP280_S32_t)buffer[1]<<4)+((BMP280_S32_t)buffer[2]>>4);
	printf("T_N = 0x%05lX\r\n",T);
	Temperature_value_compensated=bmp280_compensate_T_int32(T);
	h_BMP280->temp = Temperature_value_compensated;
	return 0;
}

int BMP280_get_pressure(h_BMP280_t * h_BMP280)
{
	uint32_t P;
	BMP280_U32_t Pressure_value_compensated;
	uint8_t buffer[3];
	uint8_t temp=BMP280_PRESSURE_MSB;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &temp, 1);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 3);
	if (buffer[0]==0){
		printf("Erreur de lecture\r\n");
		return 1;
	}
	P =((BMP280_U32_t)buffer[0]<<12)+((BMP280_U32_t)buffer[1]<<4)+((BMP280_U32_t)buffer[2]>>4);
	printf("P_N = 0x%05lX\r\n",P);
	Pressure_value_compensated=bmp280_compensate_P_int32(P);
	h_BMP280->press = Pressure_value_compensated;

	return 0;
}

BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
	BMP280_S32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) *
			((BMP280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
{
	BMP280_S32_t var1, var2;
	BMP280_U32_t p;
	var1 = (((BMP280_S32_t)t_fine)>>1) - (BMP280_S32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BMP280_S32_t)dig_P6);
	var2 = var2 + ((var1*((BMP280_S32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((BMP280_S32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BMP280_S32_t)dig_P2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((BMP280_S32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((BMP280_U32_t)(((BMP280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((BMP280_U32_t)var1);
	}
	else
	{
		p = (p / (BMP280_U32_t)var1) * 2;
	}
	var1 = (((BMP280_S32_t)dig_P9) * ((BMP280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((BMP280_S32_t)(p>>2)) * ((BMP280_S32_t)dig_P8))>>13;
	p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}

