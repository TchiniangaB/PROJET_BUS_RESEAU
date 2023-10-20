/*
 * BMP280.c
 *
 *  Created on: 10 oct. 2023
 *      Author: GILLES Baptiste
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
	h_BMP280->data_available = 0;
	h_BMP280->skipped_data = 0;

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
	uint8_t power_mode = BMP280_SAMPLE_2|BMP280_SAMPLE_16|BMP280_NORMAL_MODE ;
	uint8_t buffer[2];
	buffer[0] = BMP280_CONTROL;
	buffer[1] = power_mode;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, buffer, 2);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 1);
	if (buffer[0] == power_mode){
		printf("Configuration done\r\n");
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
	uint32_t var1,var2,T;
	uint8_t buffer[3];
	uint8_t temp=BMP280_TEMP_MSB;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &temp, 1);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 3);
	if (buffer[0]==0){
		printf("Erreur de lecture\r\n");
		return 1;
	}
	T=buffer[0]|buffer[1]|buffer[2];
	var1 = ((((T>>3) - (((uint32_t)dig_T1<<1))) * ((uint32_t)dig_T2))) >> 11;
	var2 = ((((T>>4) - ((uint32_t)dig_T1)) * ((T>>4) - ((uint32_t)dig_T1))) >> 12)*((uint32_t)dig_T3) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	h_BMP280->temp=T;
	return 0;
}

int BMP280_get_pressure(h_BMP280_t * h_BMP280)
{
	uint64_t var1,var2,P;
	uint8_t buffer[3];
	uint8_t temp=BMP280_PRESSURE_MSB;
	h_BMP280->I2C_drv.transmit(BMP280_I2CADDR, &temp, 1);
	h_BMP280->I2C_drv.receive(BMP280_I2CADDR, buffer, 3);
	if (buffer[0]==0){
		printf("Erreur de lecture\r\n");
		return 1;
	}
	P =buffer[0]|buffer[1]|buffer[2];

	var1 = t_fine - 128000;
	var2 = var1 * var1 * (uint64_t)dig_P6;
	var2 = var2 + ((var1*(uint64_t)dig_P5)<<17);
	var2 = var2 + ((uint64_t)dig_P4<<35);
	var1 = ((var1 * var1 * (uint64_t)dig_P3)>>8) + ((var1 * (uint64_t)dig_P2)<<12);
	var1 = ((((uint64_t)1<<47)+var1)*((uint64_t)dig_P1))>>33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	P = 1048576-P;
	P = (((P<<31)-var2)*3125)/var1;
	var1 = (((uint64_t)dig_P9) * (P>>13) * (P>>13)) >> 25;
	var2 = (((uint64_t)dig_P8) * P) >> 19;
	P = ((P + var1 + var2) >> 8) + ((uint64_t)dig_P7<<4);
	h_BMP280->press=(uint32_t)P;
	return 0;
}

