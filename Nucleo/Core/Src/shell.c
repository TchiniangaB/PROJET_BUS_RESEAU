/**
 *@file shell.c
 *@brief Shell permettant de controller le BMP280 et le CAN
 *@date Oct 1, 2023
 *@author Tchinianga Bastien
 */


#include "shell.h"
#include "BMP280.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"

uint8_t prompt[]="\r\nuser@Nucleo-STM32G446RE>>";
uint8_t started[]=
		"\r\n*-----------------------------*"
		"\r\n| Welcome on BMP280 drivers shell |"
		"\r\n*-----------------------------*"
		"\r\n";
uint8_t newline[]="\r\n";
uint8_t backspace[]="\b \b";
uint8_t cmdNotFound[]="Command not found\r\n";
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
extern int K;
extern uint8_t aData[2];
uint8_t Temp;
extern UART_HandleTypeDef huart2;


void Shell_Init(void){
	memset(argv, NULL, MAX_ARGS*sizeof(char*));
	memset(cmdBuffer, NULL, CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart4, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart4, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart4, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}


void Shell_Loop(void){
	if(uartRxReceived){ //Condition verifiée lors de la réception d'un nouveau caractère UART
		switch(uartRxBuffer[0]){
		case ASCII_CR: //Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart4, newline, sizeof(newline), HAL_MAX_DELAY);
			//HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
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
			HAL_UART_Transmit(&huart4, backspace, sizeof(backspace), HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: //Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart4, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){ //Condition vérifiant la validitée d'une commande
		if(strcmp(argv[0],"etalonnage")==0){//Fonction help renvoyant la notice des fonctions spécifiées
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Etalonnage en cours\r\n");
			BMP280_etalonnage(&h_BMP280);
			HAL_UART_Transmit(&huart4, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
		}
		else if (strcmp(argv[0],"GET_P")==0){
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Acquisition de la pression\r\n");
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart4, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			BMP280_get_pressure(&h_BMP280);
			printf("P=%dPa\r\n",h_BMP280.press);
		}
		else if (strcmp(argv[0],"GET_T")==0){
			int uartTxStringLength = snprintf((char *)uartTxBuffer, UART_TX_BUFFER_SIZE, "Acquisition de la température\r\n");
			HAL_UART_Transmit(&huart4, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, uartTxBuffer, uartTxStringLength, HAL_MAX_DELAY);
			BMP280_get_temp(&h_BMP280);
			Temp = (int)h_BMP280.temp;
			printf("T=+%d.%d_C\r\n",(int)(h_BMP280.temp/100),h_BMP280.temp%100);
			aData[0] = (Temp + (int)h_BMP280.temp)/10;
		}
		else if (strcmp(argv[0],"GET_K")==0){
			printf("K=%d.%d000\r\n",(int)(K/100),K%100);
		}
		else if (strcmp(argv[0],"GET_A")==0){
			printf("A=%d.%d000\r\n",(int)(aData[0]/1000),aData[0]%1000);
		}
		else if (strcmp(argv[0],"SET_K")==0){
			if(atoi(argv[1])>=0){				//La valeur de K reçue étant une chaine de caractères ASCII, atoi permet de la convertir en entier
				if(atoi(argv[1])<=100){
					K = atoi(argv[1]);
				}
			}
		}
		else{
			HAL_UART_Transmit(&huart4, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart4, prompt, sizeof(prompt), HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
		newCmdReady = 0;
	}
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart4, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
