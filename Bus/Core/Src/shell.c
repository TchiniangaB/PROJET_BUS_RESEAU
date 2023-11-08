/**
 *@file shell.c
 *@brief Shell permettant de controller le moteur
 *@date Oct 1, 2023
 *@author Nicolas
 *@author Tom
 *@author Baptiste
 *@author Antoine
 */


#include "shell.h"
#include "BMP280.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint8_t prompt[]="user@Nucleo-STM32G474RET6>>";
uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on Nucleo-STM32G474 |"
		"\r\n*-----------------------------*"
		"\r\n";
uint8_t newline[]="\r\n";
uint8_t backspace[]="\b \b";
uint8_t cmdNotFound[]="Command not found\r\n";
uint8_t brian[]="Brian is in the kitchen\r\n";
uint8_t uartRxReceived;
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

char	 	cmdBuffer[CMD_BUFFER_SIZE];
int 		idx_cmd;
char* 		argv[MAX_ARGS];
int		 	argc = 0;
char*		token;
int 		newCmdReady = 0;
extern h_BMP280_t h_BMP280;
extern UART_HandleTypeDef huart2;

/**
 * @brief Fonction d'initialisation du Shell
 * @note Affiche un message d'accueil lors du lançement du programme
 */

void Shell_Init(void){
	memset(argv, NULL, MAX_ARGS*sizeof(char*));
	memset(cmdBuffer, NULL, CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

/**
 * @brief Cette fonction tourne en boucle afin de recevoir et d'exécuter les commandes du Shell
 */
void Shell_Loop(void){
	if(uartRxReceived){ //Condition verifiée lors de la réception d'un nouveau caractère UART
		switch(uartRxBuffer[0]){
		case ASCII_CR: //Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token!=NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;
		case ASCII_BACK: //Suppression du dernier caractère
			cmdBuffer[idx_cmd--] = '\0';
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: //Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){ //Condition vérifiant la validitée d'une commande
		if(strcmp(argv[0],"WhereisBrian?")==0){
			HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0],"etalonnage")==0){//Fonction help renvoyant la notice des fonctions spécifiées
			BMP280_etalonnage(&h_BMP280);
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Etalonnage en cours\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			if(strcmp(argv[1],"get_temp_press")==0){
				int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Acquisition de la température\r\n");
				HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
				BMP280_get_temp(&h_BMP280);
				BMP280_get_pressure(&h_BMP280);
				printf("température = %f\r\n", h_BMP280.temp);
				printf("pression = %f\r\n", h_BMP280.press);
			}
		}
		else{
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
		newCmdReady = 0;
	}
}

/**
 * @brief Fonction Callback appelée lors de la réception d'un nouveau caractère
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
