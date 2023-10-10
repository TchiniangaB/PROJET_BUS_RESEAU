/*
 * BMP280.c
 *
 *  Created on: 10 oct. 2023
 *      Author: GILLES Baptiste
 */

#include "BMP280.h"

int BMP280_init(h_BMP280_t * h_BMP280)
{
    h_BMP280->data_available = 0;
	h_BMP280->skipped_data = 0;
    //Récupération de l'ID du composant
    uint8_t chip_id = 0;
    h_BMP280->I2C_drv.receive(BMP280_CHIP_ID, &chip_id, 1);
    if(chip_id != 0x58)
    {
        return -1;
    }
    
    //Reset du composant
    uint8_t reset = 0xB6;
    h_BMP280->I2C_drv.transmit(BMP280_RESET, &reset, 1);

    //Configuration du composant
    uint8_t config = 0;
    config |= (h_BMP280->sample << 5);
    h_BMP280->I2C_drv.transmit(BMP280_CONFIG, &config, 1);

    return 0;
}