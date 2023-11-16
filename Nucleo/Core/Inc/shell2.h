/*
 * shell.h
 *
 *  Created on: Nov 12, 2023
 *      Author: Bapt
 */
 

#ifndef INC_UART_PROTOCOL_H_
#define INC_UART_PROTOCOL_H_

#include "main.h"
#include "string.h"
#include "BMP280.h"

void protocol(char RX_Pi_buffer[RX_BUFFER_SIZE], uint8_t Size);



#endif /* INC_UART_PROTOCOL_H_ */
