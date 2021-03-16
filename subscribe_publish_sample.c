/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved2.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitatio ns under the License.
 */

/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the aws_iot_config.h file and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "sdkTest/sub"
 *
 * If all the certs are correct, you should see the messages received by the application in a loop.
 *
 * The application takes in the certificate path, host name , port and the number of times the publish should happen.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "Parameter.h"
#include "def.h"
#include "Ram.h"
#include "jsmn.h"

//pp
u32 Etichetta;
//      Tipo di centrale FM oppure SD
////u8      TipoCentrale;
u8		SupinvFlags[8];
//	CVPS della centrale
u16		CVPSSupInv;
//	Codice di impianto della centrale (1..92)
u8		CodiceImpiantoSupInv;
//      Address sul bus Rs485 della centrale Supinv
u8		AddressBusCentraleSupInv[MAX_NUM_UART];
//      Mappa a bit dei log disabilitati
////u8		MapBitLogDisabled[32];
//	Codice di identificazione della centrale (etichetta)
u32		EtichettaSupInv;

tTabFilesExSpiFlash TabFilesExSpiFlash[] = {
    {SPI_FLASH_PARAM_START_ADDR,                SPI_FLASH_PARAM_END_ADDR,               "/home/root/FM/Param.bin"},
    {SPI_FLASH_LOG_BOOK_START_ADDR,             SPI_FLASH_LOG_BOOK_END_ADDR,            "LogBook.bin"},
    {SPI_FLASH_SNAPSHOT_START_ADDR,             SPI_FLASH_SNAPSHOT_END_ADDR,            "Snapshot.bin"},
    {SPI_FLASH_BMP_LOGO_BEGHELLI_START_ADDR,    SPI_FLASH_BMP_LOGO_BEGHELLI_END_ADDR,   "LogoBeghelli.bin"},
    {SPI_FLASH_BMP_PC_START_ADDR,               SPI_FLASH_BMP_PC_END_ADDR,              "Pc.bin"},
    {SPI_FLASH_BMP_ANTENNA_GSM_START_ADDR,      SPI_FLASH_BMP_ANTENNA_GSM_END_ADDR,     "AntennaGsm.bin"},
    {SPI_FLASH_BMP_WIFI_START_ADDR,             SPI_FLASH_BMP_WIFI_END_ADDR,            "Wifi25_25.bin"},
    {SPI_FLASH_PROGRAMMA_UPGRADE_START_ADDR,    SPI_FLASH_PROGRAMMA_UPGRADE_END_ADDR,   "ProgrammaUpgrade.bin"},
    {SPI_FLASH_BMP_BANDIERE_START_ADDR,         SPI_FLASH_BMP_BANDIERE_END_ADDR,        "Bandiere.bin"},
    {SPI_FLASH_BMP_RS485_START_ADDR,            SPI_FLASH_BMP_RS485_END_ADDR,           "Rs485.bin"},
    {SPI_FLASH_BMP_MONDO_INTERNET_START_ADDR,   SPI_FLASH_BMP_MONDO_INTERNET_END_ADDR,  "MondoInternet.bin"},
    {SPI_FLASH_PARAM_EXT_START_ADDR,            SPI_FLASH_PARAM_EXT_END_ADDR,           "ParamExt.bin"},
    {SPI_FLASH_DATI_CONSUMO_NODI_START_ADDR,    SPI_FLASH_DATI_CONSUMO_NODI_END_ADDR,   "DatiConsumoNodi.bin"},
    {0xffffffff,                                0xffffffff,                             ""}
};
u8              Param[SPI_FLASH_LEN];
u16 RadioIDSupInv;
u16 RadioIDSupInvInFindNodes;
u32 NSecErrCom12H;
u32 NSecErrCom24H;
u16 gTotNodi;
int num_shadow;


typedef struct cenlinStatus {
	u32 StateGsm;
	u8 StateGsmReg;
	u8 StateSim;
	u8 StateGsmSignalQuality;
	u8 StateSupinv[8];
	u32 StateEth0;
	u8 supinvFlags[8];
	u16 swVer;
	u8 testInProgress;
	u32 Timestamp;

} cenlinStatusStruct;

struct shmem_cenlin {

	u8   status[992];
	u8   errors[992];
	u32  tOn[992];
	u8   kWh[992*6];
	char status_changed;
	char errors_changed;
	char power_changed;
	u16 gTotNodi;
	u8 power[992];
	u8 new_message;
	u8 message[200];
	cenlinStatusStruct statoCenlin;

};

struct shmem_cenlin* p_shmem_cenlin;
AWS_IoT_Client client;

// --------------------------------------------

#define NUM_BLOCK 32
#define LEN_BLOCK 64

#define SHADOW_CONFIG_PUBLISHED             0x01
#define SHADOW_STATUS_BASE_ERROR_PUBLISHED  0x02
#define SHADOW_STATUS_BASE_STATUS_PUBLISHED 0x04
#define SHADOW_ENERGY_TIME_PUBLISHED        0x08

//	---------------------------------------------------------------------------
//	N.ro max di nodi del sistema
//	---------------------------------------------------------------------------
#define	MAX_NODE                                    992

#define TRUE    1
#define FALSE   0

//	---------------------------------------------------------------------------
//	N.ro di gruppi di nodi programmabili e gestibili
//	---------------------------------------------------------------------------
////#define	TOT_GROUPS                          16
#define NODE_TYPE_RELAIS_DOMOTICO           29

//	---------------------------------------------------------------------------
//	Node type
//	---------------------------------------------------------------------------
#define NODE_TYPE_FM                        0
#define NODE_TYPE_CR                        1
#define NODE_TYPE_US                        2
#define NODE_TYPE_SOLAR                     3
#define NODE_TYPE_FREE_0                    4
#define NODE_TYPE_INSANO                    5
#define NODE_TYPE_LAMPIONI_STRADALI         6
#define NODE_TYPE_FARETTI_SCARICA           7
#define NODE_TYPE_UNICREDIT                 8
#define NODE_TYPE_CONTAGUADAGNO             9
#define NODE_TYPE_LAMPIONI_LED              10
#define NODE_TYPE_CENTRALE_COMPACT_INSANO   11
#define NODE_TYPE_ITL                       12
#define NODE_TYPE_ZIGMOD2                   13
#define NODE_TYPE_COVECO                    14
#define NODE_TYPE_LAMPADE_LED               15
#define NODE_TYPE_SOS_626_RIV_LUCE          16
#define NODE_TYPE_MISRAD		    		17
#define NODE_TYPE_SMOTER                    18
#define NODE_TYPE_STM                       19
#define NODE_TYPE_BALERA                    20
#define NODE_TYPE_MISPOT                    21
#define NODE_TYPE_SENSORI_AUTOMAZIONE       22
#define NODE_TYPE_ZIGMOD3                   23
#define NODE_TYPE_ZIGMOD4                   24
#define NODE_TYPE_ZIGBAL                    25
#define NODE_TYPE_CONCENTRATORE             26
#define NODE_TYPE_SENSORE_FOTONICO          27
#define NODE_TYPE_TRASMETTITORE_DOMOTICO    28
#define NODE_TYPE_RELAIS_DOMOTICO           29
#define NODE_TYPE_GP_ATMEL                  30
#define NODE_TYPE_MISURATORE                31
#define NODE_TYPE_AMADORI                   32

#define TOT_NODE_TYPE                       33

#define	MAX(x,y) (x>y ? x : y)
#define	MIN(x,y) (x<y ? x : y)

// LISTA OPCODE micro 
#define 	OPCODE_GET_SW_INFO_CU 					0x01
#define 	OPCODE_GET_STATUS_ALARM_CU           	0x02
#define 	OPCODE_SET_CONFIG_REG_CU       			0x03
#define 	OPCODE_GET_MEASURES           			0x04
#define 	OPCODE_SET_STATUS_MOD_WIFI           	0x07
#define 	OPCODE_ENROLLMENT_CMD          			0x09
#define 	OPCODE_ENROLLMENT_GET_PRESSED  			0x0A
#define 	OPCODE_GET_ENROLLMENT_REG      			0x0B
#define 	OPCODE_PRENOTA_SWITCHOFF 				0x0D
#define 	OPCODE_SET_DATA_ORA                    	0x0E
#define 	OPCODE_GET_DATA_ORA            			0x0F
#define 	OPCODE_GET_DATI_IMPIANTO       			0x10
#define 	OPCODE_GET_TIM_TEST_E_PERIOD          	0x11
#define 	OPCODE_SET_TIM_TEST_FUNZIONALE  		0x12
#define 	OPCODE_SET_TIM_TEST_AUTONOMIA   		0x13
#define 	OPCODE_SET_PERIOD_TEST_FUNZIONALE 		0x14
#define 	OPCODE_SET_PERIOD_TEST_AUTONOMIA        0x15
#define 	OPCODE_GET_CONT_LAST_EVENTI            	0x16
#define 	OPCODE_RESET_CONT_LAST_EVENTI         	0x17
#define 	OPCODE_GET_TOT_NUM_LOGS                	0x18
#define 	OPCODE_GET_DATES_LOG_TEST_FUNZ       	0x19
#define 	OPCODE_GET_DATES_LOG_TEST_AUTO          0x1A
#define 	OPCODE_GET_DATES_LOG_EVENTI_RETE        0x1B
#define 	OPCODE_GET_LOGBOOK                    	0x1C
#define 	OPCODE_GET_LG_SERIAL_ID                	0x20
#define 	OPCODE_GET_LG_TIPO_APPARECCHIO       	0x21
#define 	OPCODE_GET_LG_COD_PROD_FINITO       	0x22   
#define 	OPCODE_GET_LG_STATUS_BYTE1  	        0x23   
#define 	OPCODE_GET_LG_STATUS_BYTE2          	0x24   
#define 	OPCODE_GET_LG_ERROR_REGISTER        	0x25   
#define 	OPCODE_GET_LG_SA_STATUS_BYTE        	0x26  
#define 	OPCODE_GET_LG_TEMPO_RESIDUO         	0x27   
#define 	OPCODE_GET_LG_IDSW_REL              	0x28   
#define 	OPCODE_GET_LG_BUILD                 	0x29   
#define 	OPCODE_GET_LG_DATA_PRODUZIONE       	0x2A   
#define 	OPCODE_GET_LG_PLANT_PRODUTTIVO      	0x2B
#define 	OPCODE_SET_LG_SERIAL_ID             	0x30
#define 	OPCODE_LG_CMD_PASS_THROUGH          	0x40
#define 	OPCODE_STOP_TEST_COMUNICAZIONE      	0x41   
#define 	OPCODE_GET_RESULT_TEST_COM_LG       	0x42   
#define 	OPCODE_GET_STATO_LAMPADE            	0x43
#define 	OPCODE_GET_DATA_ORA_LAST_TEST       	0x44   
#define 	OPCODE_GET_DATA_ORA_LAST_EVENT_RETE 	0x45   
#define 	OPCODE_WRITE_EEPROM                 	0x50   
#define 	OPCODE_READ_EEPROM                 		0x51   
#define 	OPCODE_UPG_FLASH_START             		0x60   
#define 	OPCODE_UPG_FLASH_WRITE              	0x61   
#define 	OPCODE_UPG_FLASH_VERIFY_CHKSUM      	0x62   
#define 	OPCODE_UPG_FLASH_END                	0x63   
#define 	OPCODE_RESET_SWAP_FW                	0x64   
#define 	OPCODE_SET_SERIAL_ID                	0x6A   
#define 	OPCODE_GET_SERIAL_ID                	0x6B   												   
#define 	OPCODE_SET_DATE_PRODUCTION          	0x6C   
#define 	OPCODE_GET_DATE_PRODUCTION          	0x6D   											   
#define 	OPCODE_SET_DATA_TABLE               	0x6E   								   
#define 	OPCODE_GET_DATA_TABLE               	0x6F
#define 	OPCODE_SET_NOME_IMPIANTO            	0x70   
#define 	OPCODE_GET_NOME_IMPIANTO            	0x71   
#define 	OPCODE_SET_PLANT_PRODUTTIVO         	0x72   
#define 	OPCODE_GET_PLANT_PRODUTTIVO         	0x73   
#define 	OPCODE_SET_TIPO_PRODOTTO_FINITO     	0x74   
#define 	OPCODE_GET_TIPO_PRODOTTO_FINITO     	0x75   
#define 	OPCODE_PROTOCOL_FD				     	0xFD  

//Lista opcodes micro WiFi (quello che si occupa della connessione al cloud):
#define 	OPCODE_GET_SW_INFO                  	0x01
#define 	OPCODE_GET_STATUS_ALARM             	0x02   
#define 	OPCODE_SET_AP_PASSWORD              	0x04
#define 	OPCODE_SET_STA_SSID                 	0x05
#define 	OPCODE_SET_STA_PASSWORD             	0x06
#define 	OPCODE_SET_TZ                       	0x07
#define 	OPCODE_SET_LOCATION                 	0x08
#define 	OPCODE_OTA_FW_UPGRADE               	0x09
#define 	OPCODE_RESET                        	0x0A    
#define 	OPCODE_SET_CA_CERT_KEY_START        	0x20    
#define 	OPCODE_SET_CA_CERT_KEY_WRITE        	0x21   
#define 	OPCODE_SET_CA_CERT_KEY_VER_CHKSUM   	0x22    
#define 	OPCODE_SET_CA_CERT_KEY_END          	0x23    
#define 	OPCODE_FORMAT_CERT_PARTITON         	0x24    
#define 	OPCODE_SWAP_CERT                    	0x25    
#define 	OPCODE_SELECT_AWS_ENDPOINT          	0x26    
#define 	OPCODE_REQUEST_ENROLLMENT           	0x64    
#define 	OPCODE_SET_LOG_LEVEL                	0x6A    
#define 	OPCODE_START_OTA_BY_CLOUD           	0x70    
#define 	OPCODE_START_OTA_CUSTOM_BY_CLOUD    	0x71

//Lista subcode di OPCODE_LG_CMD_PASS_THROUGH  
#define 	SUBCODE_RICERCA_LAMPADE					0x01 
#define 	SUBCODE_CICLO_POLL_COMPLETO				0x02 
#define 	SUBCODE_TEST_FUNZIONALE			    	0x03
#define 	SUBCODE_TEST_AUTONOMIA_1H			   	0x04
#define 	SUBCODE_TEST_AUTONOMIA_DURATA_IMPOSTATA	0x05
#define 	SUBCODE_ACCENSIONE_INCONDIZIONATA		0x06
#define 	SUBCODE_STOP_TEST					   	0x07
#define 	SUBCODE_COMANDO_SA					   	0x08
#define 	SUBCODE_DISABILITA_EMERGENZA			0x09
#define 	SUBCODE_ABILITA_EMERGENZA				0x0A
#define 	SUBCODE_INIBIZIONE_IMPIANTO				0x0B
#define		SUBCODE_CANCELLA_ERRORI					0x0C
#define		SUBCODE_BLINK_LAMPADA_ON_OFF			0x0D
#define		SUBCODE_SCRIVI_TIPO_PRODOTTO			0x0E
#define		SUBCODE_TEST_COMUNICAZIONE				0x0F
#define		SUBCODE_SET_CONFIG_LAMP					0x11
#define		SUBCODE_SET_CONFIG_LAMP_MULTI			0x12

// --------------------------------------------
#define HOST_ADDRESS_SIZE 255
/**
 * @brief Default cert location
 */
static char certDirectory[PATH_MAX + 1] = "../../../certs";

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
static char HostAddress[HOST_ADDRESS_SIZE] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
static uint32_t port = AWS_IOT_MQTT_PORT;

////unsigned char blocks_changed[NUM_BLOCK*100];//attenzione !!!!
unsigned char changed[256];
/**
 * @brief This parameter will avoid infinite loop of publish and exit the program after certain number of publishes
 */
static uint32_t publishCount = 0;
static unsigned char  bufferTx[500];

#define MAX_LEN_SHADOW 5*1024

//static char Payload[MAX_LEN_SHADOW+1];
//static char str1_topic_shadow[100];
static char json_string[MAX_LEN_SHADOW+1] = {}; 

void convertStringToByte(char* st, int byte[]);
bool checkCRC(int byte[]);
unsigned char calcCRC(unsigned char byte[]);
void gestOpcodeMain(int byte[]);
void gestOpcodeWIFI(int byte[]);
void gestCmdPassThrough(int byte[]);
void print_json_command_lo(unsigned char msg[]);
void print_json_command_fd(unsigned char msg[]);
void gestProtocolFD(int byte[], int n);

static bool flag_tx_json_command_fd = false;
static bool flag_tx_json_command_lo = false;


static void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
									IoT_Publish_Message_Params *params, void *pData) {
	char value[1024];    
	char name[50];    
	IOT_UNUSED(pData);
	IOT_UNUSED(pClient);
	IOT_INFO("Subscribe callback");
	IOT_INFO("RICEVUTO : %.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
	char keyString[1024];
	int resultCode;
	//IoT_Error_t rc1 = FAILURE;
	//AWS_IoT_Client client1;
	//IoT_Publish_Message_Params paramsQOS0;
	jsmn_parser p;
	jsmntok_t tokens[128]; // a number >= total number of tokens
	jsmntok_t key;
	jsmn_init(&p);
	resultCode = jsmn_parse(&p, (char *) params->payload, params->payloadLen, tokens, 128);
	// restituisce n token, ognuno di quali contiene una stringa campo oppure valore 
	// mi aspetto che siano sempre a 2 a 2 
	key = tokens[1];
	unsigned int length = key.end - key.start;
	printf("1 - length is %d  key.end %d - key.start %d \n", length, key.end, key.start);
	memcpy(name, params->payload+key.start, length);
	name[length] = '\0';
	printf("Campo 1 is : %s\n", name);
	key = tokens[2];
	length = key.end - key.start;
	printf("2 - length is %d  key.end %d - key.start %d \n", length, key.end, key.start);
	//keyString[length + 1];    
	memcpy(value, params->payload+key.start, length);
	value[length] = '\0';
	printf("Valore 1 is : %s\n", value);
	//traduzione da stringa a serie di caratteri esadecimali 
    ///int num_value = (int)strtol(value, NULL, 16);
	///printf("converto il primo carattere in un intero : %d \n", num_value );

    unsigned char numBytes, ctrlCode, opcode, char_1, char_2; 
	int byteF[300];
	convertStringToByte(value, byteF);

	char_1 = byteF[0];
	char_2 = byteF[1];
	printf("char_1 = %02x - %c  char_2 = %02x - %c\n",byteF[0],byteF[0],byteF[1],byteF[1]);
	
	if((char_1 == 'L')&&(char_2 = 'O' )){
		numBytes = byteF[2];
		ctrlCode = byteF[3];
		opcode = byteF[4];

		printf("Gestione protocollo LD len = %d\n",numBytes);

		for (int i=0; i<numBytes; i++){
			printf("byte[%d] = %d\n",i,byteF[i]);
			byteF[i]=byteF[i+2];
		}

		//controllo crc e gestione codice operativo
		if(checkCRC(byteF)){
			printf("CRC OK\n");
			if(ctrlCode&0x01)
				gestOpcodeWIFI(byteF);
			else gestOpcodeMain(byteF);
			
			//mando la risposta
			flag_tx_json_command_lo = TRUE;

		} else printf("CRC ERROR\n");
	} else if((char_1 == 'F')&&(char_2 = 'D' )) {
		printf("Gestione protocollo FD\n");	
		//devo rigirare il messaggio senza lo stuffing che è previsto nel protocollo.
		if(byteF[2]==0xFD){
			numBytes = byteF[6]*255+byteF[7];

			int j=0;
			for (int i=0; i<(numBytes+9); i++){
/*				if(byteF[i+2]==0xFA){
					byteF[j]=byteF[i+2]+byteF[i+3];
					//printf("byte[%d] = %d\n",i,byteF[i]);
					i++;
				}else{
					byteF[j]=byteF[i];
				}
*/				
			    byteF[j]=byteF[i+2];
				printf("byte[%d] = %d\n",j,byteF[j]);
				j++;
			}
			printf("vado a fare la copia in shared \n");
			gestProtocolFD(byteF,j);
			//mando la risposta
			flag_tx_json_command_fd = TRUE;
		} else printf("ERROR RX FD PROTOCOL\n");
	} else {
		printf("Tipo protocollo command non riconosciuto !!!! ");
	}
	
}


void print_json_command_lo(unsigned char msg[]) {
    int i = 0;
	int j = 0;
	char appo[MAX_LEN_SHADOW+1];
	
	sprintf(json_string, "{\"frame\":\"" );
	
	for (j=0; j < msg[0]; j++) {
		sprintf(appo, "%02X", msg[j]);
		strcat(json_string, appo);	
	}
	sprintf(appo, "\",");
	strcat(json_string, appo);	

    //aggiungo "cu_id":"99998","cu_type":"logicafm"
	sprintf(appo, "\"cu_id\":\"%5d\",\"cu_type\":\"logicafm\"", Etichetta);
	strcat(json_string, appo);	


	sprintf(appo, "}");
	strcat(json_string, appo);
	printf("json RESPONSE is : %s \n", json_string);
}

void print_json_command_fd(unsigned char msg[]) {
    int i = 0;
	int j = 0;
	char appo[MAX_LEN_SHADOW+1];
	
	sprintf(json_string, "{\"frame\":\"" );
	
	for (j=0; j < ((msg[4]<<8|msg[5])+7); j++) {
		sprintf(appo, "%02X", msg[j]);
		strcat(json_string, appo);	
	}
	sprintf(appo, "\",");
	strcat(json_string, appo);	

    //aggiungo "cu_id":"99998","cu_type":"logicafm"
	sprintf(appo, "\"cu_id\":\"%5d\",\"cu_type\":\"logicafm\"", Etichetta);
	strcat(json_string, appo);	


	sprintf(appo, "}");
	strcat(json_string, appo);
	printf("json RESPONSE is : %s \n", json_string);
}

void convertStringToByte(char* st, int byte[]) {
    
    int i;
    char stTemp[2];
 
    //Sul primo byte ho la lunghezza
    strncpy(stTemp,st,2);
    byte[0]=strtol(stTemp, NULL, 16);
    
    //ciclo ora sulla lunghezza  
    for (i=1; i<byte[0]; i++){
        strncpy(stTemp,st+(i*2),2);
        byte[i]=strtol(stTemp, NULL, 16);
    }
}

//protocollo:  Numbyte CtrlCode Opcode Data CheckSum
//CRC somma di tutti i byte 
bool checkCRC(int byte[]){
     
	 unsigned char sum = 0;
	 int len = byte[0];
	 for (int i=1; i<(len-1); i++){
		 sum^=byte[i];
	 }
	 if (sum == byte[len-1])
	 	return TRUE;
	 else return FALSE;	 
}

unsigned char calcCRC(unsigned char byte[]){
     
	 unsigned char sum = 0;
	 int len = byte[0];
	 for (int i=1; i<(len-1); i++){
		 sum^=byte[i];
	 }
	 return sum;	
}

void gestProtocolFD(int byte[], int n) {

			int i = 0;
			int timeout = 0;
			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			printf("TX MSG FD TO CENLIN: ");
			p_shmem_cenlin->message[0] = OPCODE_PROTOCOL_FD;
			printf("%02X",p_shmem_cenlin->message[0]);
			for (i=1; i<n; i++) {
				p_shmem_cenlin->message[i+1] = byte[i];
				printf("%02X",p_shmem_cenlin->message[i+1]);
			}

			//Dovro gestire la risposta che mi arriverà dalla cenlin ????
			while((p_shmem_cenlin->new_message != 2)&&(timeout<10)){//mi aspetto 2 dalla cenlin metto 5 secondi di timeout
				sleep(1);
				timeout++;
			}
			if (p_shmem_cenlin->new_message == 2) {
			
				//invio la risposta corretta che ho ricevuto dalla cenlin
				int lenRx = p_shmem_cenlin->message[4]*255 + p_shmem_cenlin->message[5];
				memcpy(&bufferTx[0], &p_shmem_cenlin->message[0], lenRx+7);
				printf("La risposta a nube è di %d caratteri : 0x%02X 0x%02X \n", lenRx+7, bufferTx[0], bufferTx[1]);

			
			} else {
			
				bufferTx[0]=0x04;//numBytes 
				bufferTx[1]=0x01;//ctrlCode ERRORE OPCODE NON GESTITO = 0x01
 				bufferTx[2]=OPCODE_PROTOCOL_FD; //ripeto l'opcode nella risposta
				bufferTx[3]=calcCRC(bufferTx);//CRC 
				//invio un messaggio di errore
			
			}

}



void gestOpcodeMain(int byte[]){
    
    //int byteTx[20];
	switch(byte[2]) {

		case  OPCODE_GET_SW_INFO_CU: //0x01
			printf("RX GET SW INFO CU\n");
			//preparare la risposta 
			bufferTx[0] = 13;//numBytes 
			bufferTx[1] = 0;//ctrlCode
			bufferTx[2] = OPCODE_GET_SW_INFO_CU; //ripeto l'opcode nella risposta
			bufferTx[3] = 0;  //SW_VERSION_BYTE_HI            
			bufferTx[4] = 0;  //SW_VERSION_BYTE_LOW           
			bufferTx[5] = 0;  //SW_BUILD_BYTE_HI              
			bufferTx[6] = 0;  //SW_BUILD_BYTE_LOW             
			bufferTx[7] = 0;  //TIPO_DISPOSITIVO              
			bufferTx[8] = 0;  //BOOTLOADER_VER                
			bufferTx[9] = 0;  //FW_ATTUALE                    
			bufferTx[10] = 0; //SW_NOTACTIVE_BUILD_BYTE_HI    
			bufferTx[11] = 0; //SW_NOTACTIVE_BUILD_BYTE_LOW 
            bufferTx[12] = calcCRC(bufferTx); //CRC 
		break;

		case  OPCODE_GET_STATUS_ALARM_CU: //0x02
			printf("RX GET_STATUS_ALARM_CU\n");
			
			//preparare la risposta 
			bufferTx[0] = 13;//numBytes 
			bufferTx[1] = 0;//ctrlCode
			bufferTx[2] = OPCODE_GET_STATUS_ALARM_CU; //ripeto l'opcode nella risposta
			bufferTx[3] = 0;  // byte0: TIPO_DISPOSITIVO
			bufferTx[4] = 0;  // byte1: uStatusDispositivo.lByte.Low;
			bufferTx[5] = 0;  // byte2: uStatusDispositivo.lByte.MediumL;
			bufferTx[6] = 0;  // byte3: uSettingsReg.cReg;
			bufferTx[7] = 0;  // byte4: uErrori.iByte.Low;
			bufferTx[8] = 0;  // byte5: uErrori.iByte.High;
			bufferTx[9] = 0;  // byte6: uStatusModuloWiFi.cReg;
			bufferTx[10] = 0; // byte7: uStatusDispositivo.lByte.MediumH;
			bufferTx[11] = 0; // byte8: uStatusDispositivo.lByte.High;
            bufferTx[12] = calcCRC(bufferTx); //CRC 
		break;

		case  OPCODE_SET_CONFIG_REG_CU:
			printf("RX SET_CONFIG_REG_CU\n");
		break;

		case  OPCODE_GET_MEASURES: // 0x04 Risposta: 2 byte Tensione batteria CU (H-L)
			printf("RX GET MEASURE\n");
			//devo leggere V e I sulla cenlin ???? 
			//preparare la risposta 
			bufferTx[0]=7;//numBytes 
			bufferTx[1]=0;//ctrlCode
			bufferTx[2]=OPCODE_GET_MEASURES; //ripeto l'opcode nella risposta
			bufferTx[3]=0; //0=OK 1=KO e poi mando i dati in una shadow???? oppure devo mettere qui i dati????
			bufferTx[4]=0; //tensione batteria H
			bufferTx[5]=0; //tensione batteria L
            bufferTx[6]=calcCRC(bufferTx); //CRC 
			//shadow ????
		break;

		case OPCODE_SET_DATA_ORA: //0x0E
			printf("RX SET DATA ORA\n");
			//TO DO devo passare i dati alla cenlin
			//p_shmem_cenlin->new_message = 1;
			//p_shmem_cenlin->message[0] = OPCODE_SET_DATA_ORA;
			//p_shmem_cenlin->message[1] = byte[3]; //Anno
			//p_shmem_cenlin->message[2] = byte[4]; //Mese
			//p_shmem_cenlin->message[3] = byte[5]; //Giorno
			//p_shmem_cenlin->message[4] = byte[6]; //Giorno della settimana [1=lun .. 7=dom]
			//p_shmem_cenlin->message[5] = byte[7]; //Ore
			//p_shmem_cenlin->message[6] = byte[8]; //Minuti
			//p_shmem_cenlin->message[7] = byte[9]; //Secondi

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 

		break;

		case OPCODE_GET_DATA_ORA: //0x0F
			printf("RX GET DATA ORA\n");
		break;

		case OPCODE_GET_DATI_IMPIANTO: //0x10
			printf("RX GET DATI IMPIANTO\n");
		break;

		
		case OPCODE_GET_TIM_TEST_E_PERIOD:
			printf("RX GET TIMER TEST E PERIOD\n"); //0x11

			//preparo la risposta 
			bufferTx[0]=0x14;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_funzionale[3];  
			bufferTx[4] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_funzionale[2];  
			bufferTx[5] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_funzionale[1];  
			bufferTx[6] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_funzionale[0];  
			bufferTx[7] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_autonomia[3];   
			bufferTx[8] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_autonomia[2];  
			bufferTx[9] = 0;  //((unsigned char*)&p_shmem_cenlin->timer_test_autonomia[1];  
			bufferTx[10] = 0; //((unsigned char*)&p_shmem_cenlin->timer_test_autonomia[0];  
			bufferTx[11] = 0;  //((unsigned char*)&p_shmem_cenlin->period_test_funzionale[3];  
			bufferTx[12] = 0;//((unsigned char*)&p_shmem_cenlin->period_test_funzionale[2];  
			bufferTx[13] = 0;  //((unsigned char*)&p_shmem_cenlin->period_test_funzionale[1];  
			bufferTx[14] = 0;  //((unsigned char*)&p_shmem_cenlin->period_test_funzionale[0];  
			bufferTx[15] = 0;//((unsigned char*)&p_shmem_cenlin->period_test_autonomia[3];  
			bufferTx[16] = 0;  //((unsigned char*)&p_shmem_cenlin->period_test_autonomia[2];  
			bufferTx[17] = 0;  //((unsigned char*)&p_shmem_cenlin->period_test_autonomia[1];  
			bufferTx[18] = 0;//((unsigned char*)&p_shmem_cenlin->period_test_autonomia[0];  
			bufferTx[19]=calcCRC(bufferTx);//CRC 
		break;

		case OPCODE_SET_TIM_TEST_FUNZIONALE:
			printf("RX IMPOSTA TIMER TEST FUNZIONALE\n"); //0x12
			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = OPCODE_SET_TIM_TEST_FUNZIONALE;
			p_shmem_cenlin->message[1] = byte[3];
			p_shmem_cenlin->message[2] = byte[4];
			p_shmem_cenlin->message[3] = byte[5];
			p_shmem_cenlin->message[4] = byte[6];
			p_shmem_cenlin->message[5] = byte[7];
			p_shmem_cenlin->message[6] = byte[8];

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;

		case OPCODE_SET_TIM_TEST_AUTONOMIA:
			printf("RX IMPOSTA TIMER TEST AUTONOMIA\n"); //0x13
			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = OPCODE_SET_TIM_TEST_AUTONOMIA; 
			//cambiato il protocollo rispetto alle impostazioni di modestino: non passo i secondi in 4 byte ma
			//ora minuto secondi giorno mese anno N.B. l'ordine potrebbe non essere quello indicato. 
			p_shmem_cenlin->message[1] = byte[3]; 
			p_shmem_cenlin->message[2] = byte[4];
			p_shmem_cenlin->message[3] = byte[5];
			p_shmem_cenlin->message[4] = byte[6];
			p_shmem_cenlin->message[5] = byte[7];
			p_shmem_cenlin->message[6] = byte[8];

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;

		case OPCODE_SET_PERIOD_TEST_FUNZIONALE:
			printf("RX IMPOSTA PERIODO TEST FUNZIONALE\n"); //0x14
			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = OPCODE_SET_PERIOD_TEST_FUNZIONALE;
			p_shmem_cenlin->message[1] = byte[3];
			p_shmem_cenlin->message[2] = byte[4];
			p_shmem_cenlin->message[3] = byte[5];
			p_shmem_cenlin->message[4] = byte[6];

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;

		case OPCODE_SET_PERIOD_TEST_AUTONOMIA: //0x15
			printf("RX IMPOSTA PERIODO TEST AUTONOMIA\n"); //0x15
			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = OPCODE_SET_PERIOD_TEST_AUTONOMIA;
			p_shmem_cenlin->message[1] = byte[3];
			p_shmem_cenlin->message[2] = byte[4];
			p_shmem_cenlin->message[3] = byte[5];
			p_shmem_cenlin->message[4] = byte[6];

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;


		case  OPCODE_LG_CMD_PASS_THROUGH: //0x40  
		    //devo gestire i sotto opcode 
			printf("RX OPCODE_LG_CMD_PASS_THROUGH\n");
			gestCmdPassThrough(byte);
		break;

		case OPCODE_STOP_TEST_COMUNICAZIONE: //0x41
			printf("RX OPCODE_STOP_TEST_COMUNICAZIONE\n");
		break;

		case OPCODE_SET_STATUS_MOD_WIFI:
			printf("RX OPCODE_SET_STATUS_MOD_WIFI\n");
		break;

		case OPCODE_SET_NOME_IMPIANTO: //0x70
			printf("RX OPCODE_SET NOME IMPIANTO\n");

			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = OPCODE_SET_NOME_IMPIANTO;
			for (int i=3;i<byte[0]-1;i++)
				p_shmem_cenlin->message[i-2] = byte[i];

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;

		/*case OPCODE_GET_NOME_IMPIANTO: //0x71
			printf("RX OPCODE_GET NOME IMPIANTO\n");

			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = OPCODE_SET_NOME_IMPIANTO;
			for (i=3;i<byte[0]-1;i++)
				p_shmem_cenlin->message[i-2] = byte[i];

			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x00;//ctrlCode 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;*/

		default: //subcode non previsto 
			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x01;//ctrlCode ERRORE OPCODE NON GESTITO = 0x01
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;
	}
}

void gestOpcodeWIFI(int byte[]){

	switch(byte[2]){
		case  OPCODE_GET_SW_INFO:
			printf("RX GET SW INFO\n");
		break;

		default: //opcode non previsto 
			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x01;//ctrlCode ERRORE OPCODE NON GESTITO 
 			bufferTx[2]=byte[2]; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;
	}
}

void gestCmdPassThrough(int byte[]){
	
	switch(byte[3]){
		case  SUBCODE_TEST_FUNZIONALE: //0x03 test funzionale + 2 byte address destinatario H-L (0xFFFF = broadcast)
			printf("TEST FUNZIONALE\n");
			
			//devo chiedere l'esecuzione del test funzionale...
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = 0x01;
			p_shmem_cenlin->message[1] = 41;

			//preparo la risposta 
			bufferTx[0]=0x07;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_TEST_FUNZIONALE; //subcode 0x03
            bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		case  SUBCODE_TEST_AUTONOMIA_1H: //0x04 test autonomia + 2 byte address destinatario H-L (0xFFFF = broadcast)
			printf("TEST Autonomia 1H\n");
			
			//devo chiedere l'esecuzione del test autonomia...
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = 0x01;
			p_shmem_cenlin->message[1] = 42;

			//preparo la risposta 
			bufferTx[0]=0x07;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_TEST_AUTONOMIA_1H; //subcode 0x04
            bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		case  SUBCODE_STOP_TEST: //0x07 stop test + 2 byte address destinatario H-L (0xFFFF = broadcast)
			printf("TEST Stop test\n");
			
			//devo chiedere l'esecuzione dello stop test ...
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = 0x01;
			p_shmem_cenlin->message[1] = 43;

			//preparo la risposta 
			bufferTx[0]=0x07;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_STOP_TEST; //subcode 0x07
            bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		case  SUBCODE_DISABILITA_EMERGENZA: //0x09 disabilita emergenza + 2 byte address destinatario H-L (0xFFFF = broadcast)
			printf("TEST Disabilita emergenza\n");
			
			//TO DO devo chiedere la disabilitazione emergenza...
			//p_shmem_cenlin->new_message = 1;
			//p_shmem_cenlin->message[0] = 0x01;
			//p_shmem_cenlin->message[1] = 41;

			//preparo la risposta 
			bufferTx[0]=0x07;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_DISABILITA_EMERGENZA; //subcode 0x09
            bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		case  SUBCODE_ABILITA_EMERGENZA: //0x0A abilita emergenza + 2 byte address destinatario H-L (0xFFFF = broadcast)
			printf("TEST Disabilita emergenza\n");
			
			//TO DO devo chiedere abilitazione emergenza...
			//p_shmem_cenlin->new_message = 1;
			//p_shmem_cenlin->message[0] = 0x01;
			//p_shmem_cenlin->message[1] = 41;

			//preparo la risposta 
			bufferTx[0]=0x07;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_ABILITA_EMERGENZA; //subcode 0x0A
            bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		case  SUBCODE_INIBIZIONE_IMPIANTO: //0x0B inibizione impianto + 2 byte address destinatario H-L (0xFFFF = broadcast)
			printf("TEST Disabilita emergenza\n");
			
			//TO DO devo chiedere l'inibizione dell'impianto...
			//p_shmem_cenlin->new_message = 1;
			//p_shmem_cenlin->message[0] = 0x01;
			//p_shmem_cenlin->message[1] = 41;

			//preparo la risposta 
			bufferTx[0]=0x07;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_INIBIZIONE_IMPIANTO; //subcode 0x0B
            bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		

		case SUBCODE_SET_CONFIG_LAMP_MULTI: //0x12
		
			//devo passare i dati alla cenlin
			p_shmem_cenlin->new_message = 1;
			p_shmem_cenlin->message[0] = 0x40;
			p_shmem_cenlin->message[1] = 0x12;

			for (int i=4;i<byte[0]-1;i++)
				p_shmem_cenlin->message[i-2] = byte[i];

		    //preparo la risposta 
			bufferTx[0]=0x05;//numBytes 
			bufferTx[1]=0x00;//ctrlCode
			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=SUBCODE_SET_CONFIG_LAMP_MULTI; //subcode 0x12
			bufferTx[4]=0xFF; 
			bufferTx[5]=0xFF;
			bufferTx[6]=calcCRC(bufferTx);//CRC 
		break;

		default: //subcode non previsto 
			//preparo la risposta 
			bufferTx[0]=0x04;//numBytes 
			bufferTx[1]=0x01;//ctrlCode ERRORE OPCODE NON GESTITO 
 			bufferTx[2]=OPCODE_LG_CMD_PASS_THROUGH; //ripeto l'opcode nella risposta
			bufferTx[3]=calcCRC(bufferTx);//CRC 
		break;
		
	}	
}


static void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
	IOT_WARN("MQTT Disconnect");
	IoT_Error_t rc = FAILURE;

	if(NULL == pClient) {
		return;
	}

	IOT_UNUSED(data);

	if(aws_iot_is_autoreconnect_enabled(pClient)) {
		IOT_INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	} else {
		IOT_WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect(pClient);
		if(NETWORK_RECONNECTED == rc) {
			IOT_WARN("Manual Reconnect Successful");
		} else {
			IOT_WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}

static void parseInputArgsForConnectParams(int argc, char **argv) {
	int opt;

	while(-1 != (opt = getopt(argc, argv, "h:p:c:x:"))) {
		switch(opt) {
			case 'h':
				strncpy(HostAddress, optarg, HOST_ADDRESS_SIZE);
				IOT_DEBUG("Host %s", optarg);
				break;
			case 'p':
				port = atoi(optarg);
				IOT_DEBUG("arg %s", optarg);
				break;
			case 'c':
				strncpy(certDirectory, optarg, PATH_MAX + 1);
				IOT_DEBUG("cert root directory %s", optarg);
				break;
			case 'x':
				publishCount = atoi(optarg);
				IOT_DEBUG("publish %s times\n", optarg);
				break;
			case '?':
				if(optopt == 'c') {
					IOT_ERROR("Option -%c requires an argument.", optopt);
				} else if(isprint(optopt)) {
					IOT_WARN("Unknown option `-%c'.", optopt);
				} else {
					IOT_WARN("Unknown option character `\\x%x'.", optopt);
				}
				break;
			default:
				IOT_ERROR("Error in command line argument parsing");
				break;
		}
	}

}

//	Input: 
//		Addr		<== Indirizzo della flash da leggere
//		p 		<== Indirizzo di destinazione della lettura
//		NroBytes	<== Numero dei bytes da leggere
//	Output: -1      ==> Errore in lettura nel file 
//		1.. x   ==> Numero dei bytes letti

u32 SpiFlashRead (u32 Addr, u8 *p, u32 NroBytes)
{
    FILE*           fSpiFlash;
    int             Res;
    int             i;
    int             LenParamBin;
    /*
    pthread_mutex_lock(&SpiFlashMutex);    
	*/
    Res = 0;
    //  Verifico quale file devo andare a leggere
    for (i=0 ; TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff ; i++) {
        if (Addr >= TabFilesExSpiFlash[i].AddrStartSpiFlash && Addr <= TabFilesExSpiFlash[i].AddrEndSpiFlash) {
            //  Verifica se file esiste
            if (access(TabFilesExSpiFlash[i].pNomeFileBin, F_OK) != -1) {
                // Se l'apertura del file fallisce allora esco
                fSpiFlash = fopen(TabFilesExSpiFlash[i].pNomeFileBin, "rb");
                if (fSpiFlash == NULL) {
                    printf("ReadAllFilesSpiFlash(): Errore apertura in lettura file %s\n", TabFilesExSpiFlash[i].pNomeFileBin);
                    Res = -1;
                }
                else {
                    // Posiziono il cursore ad inizio file
                    fseek(fSpiFlash, Addr-TabFilesExSpiFlash[i].AddrStartSpiFlash, SEEK_SET);
                    LenParamBin = fread (&Param[Addr], 1, NroBytes, fSpiFlash);
                    memcpy (p, &Param[Addr], LenParamBin);
                    fclose (fSpiFlash);
                    Res = LenParamBin;
                }
            }
            else {
             printf("Error access file %s\n", TabFilesExSpiFlash[i].pNomeFileBin);
			    Res = -1;
			}
            //  Lettura eseguita.. esci dal loop
            break;
        }
    }
	/*
    pthread_mutex_unlock(&SpiFlashMutex);    
	*/
    return (Res);

}

u32 SpiFlashReadParam (u32 AddrParam, u8 *p, u32 NroBytes)
{
    u32		Result;

    Result = SpiFlashRead (AddrParam, p, NroBytes);
	
    return (Result);
}

//  Leggi il numero dell'etichetta
//  Output : 0              ==> Numero dell'etichetta non valido
//           70000..99999   ==> Numero dell'etichetta
u32 ReadEtichettaCentrale (void) {
    int     fp;
    char    AuxBuf[150];
    char    AuxChar;
    u32     Etich;
    ssize_t nRead;
    int     i;

	//per il momento devo ritornare 99998
	return 99998;

    fp = open("/etc/hostname", O_RDONLY);
    if (fp > 0) {
        memset (AuxBuf, 0, sizeof(AuxBuf));
        for (i=0 ; i<(sizeof(AuxBuf)-1) ; i++) {
            nRead = read(fp, &AuxChar, 1);
            if  (0 == nRead || AuxChar == '\r' || AuxChar == '\n')
                break;
            else
                AuxBuf[i] = AuxChar;
        }
        Etich = atoi(AuxBuf);
        close(fp);
        return (Etich);
    }
    else
        return(0);
}

/*void create_json_shadow_0() {
	char str_appo[100] = {};
	int i=0;

	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"update\":\"completed\",\"num_lamp_found\":%d,\"etichetta\":%d,\"codice_impianto\":%d,\"radio_id\":%d,\"flags\":\"%02x%02x%02x%02x%02x%02x%02x%02x\",\"err_12h\":\"%08X\",\"err_24h\":\"%08X\",\"lamp\":[", Etichetta, p_shmem_cenlin->gTotNodi, EtichettaSupInv, CodiceImpiantoSupInv, RadioIDSupInv, SupinvFlags[0], SupinvFlags[1], SupinvFlags[2], SupinvFlags[3], SupinvFlags[4], SupinvFlags[5], SupinvFlags[6], SupinvFlags[7], NSecErrCom12H, NSecErrCom24H  );

	for (i=0 ; i<50 ; i++) {
		if (i == (50-1))
			sprintf(str_appo, "{\"idx\":\"%d\",\"desc\":\"%s\",\"stat\":%d,\"err\":%d,\"addr\":\"%X\",\"type\":\"%X\",\"ver\":%d}", i, "pippo",0,0,Node[i].Config.Addr,Node[i].Config.NodeType,0);
			//sprintf(str_appo, "{\"desc\":\"%s\",\"addr\":\"%X\",\"type\":\"%X\"}", "pippo",Node[i].Config.Addr,Node[i].Config.NodeType);
		else 
			sprintf(str_appo, "{\"idx\":\"%d\",\"desc\":\"%s\",\"stat\":%d,\"err\":%d,\"addr\":\"%X\",\"type\":\"%X\",\"ver\":%d},", i, "pippo",0,0,Node[i].Config.Addr,Node[i].Config.NodeType,0);
		strcat(json_string, str_appo);
	}
	sprintf(str_appo, "]}}}");
	strcat(json_string, str_appo);
	//printf("json string (%d nodi) : \n%s\n", gTotNodi,json_string );
}*/


/*void create_json_shadow_lum(int j) {
	char str_appo[100] = {};
	int i=0;

	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"update\":\"completed\",\"lamp\":[", Etichetta, p_shmem_cenlin->gTotNodi, EtichettaSupInv, CodiceImpiantoSupInv, RadioIDSupInv, SupinvFlags[0], SupinvFlags[1], SupinvFlags[2], SupinvFlags[3], SupinvFlags[4], SupinvFlags[5], SupinvFlags[6], SupinvFlags[7], NSecErrCom12H, NSecErrCom24H  );

	for (i=50*j ; i<((50+50*j)-1) ; i++) {
		if (i == ((50+50*j)-1))
			sprintf(str_appo, "{\"idx\":\"%d\",\"desc\":\"%s\",\"stat\":%d,\"err\":%d,\"addr\":\"%X\",\"type\":\"%X\",\"ver\":%d}", i, "pippo",0,0,Node[i].Config.Addr,Node[i].Config.NodeType,0);
			//sprintf(str_appo, "{\"desc\":\"%s\",\"addr\":\"%X\",\"type\":\"%X\"}", "pippo",Node[i].Config.Addr,Node[i].Config.NodeType);
		else 
			sprintf(str_appo, "{\"idx\":\"%d\",\"desc\":\"%s\",\"stat\":%d,\"err\":%d,\"addr\":\"%X\",\"type\":\"%X\",\"ver\":%d},", i, "pippo",0,0,Node[i].Config.Addr,Node[i].Config.NodeType,0);
		strcat(json_string, str_appo);
	}
	sprintf(str_appo, "]}}}");
	strcat(json_string, str_appo);
	//printf("json string (%d nodi) : \n%s\n", gTotNodi,json_string );
}*/



char *readCnfgToRam(char path[], unsigned int *fileSize) {

    FILE *fileSpiFlash;
	unsigned long int i;
	if (access(path, F_OK) != -1) {
		// Se l'apertura del file fallisce allora esco
		fileSpiFlash = fopen(path, "rb");
		if (fileSpiFlash == NULL) {
			printf("readCnfgToRam(): Errore apertura in lettura file %s\n", path);
			return 0;
		} else {
			printf("readCnfgToRam(): File  %s aperto \n", path);
			//Get file size
			if(fseek(fileSpiFlash, 0, SEEK_END) == -1){ return 0; }
			*fileSize = ftell(fileSpiFlash);
			fseek(fileSpiFlash, 0, SEEK_SET);
			printf("The file size is %d \n", *fileSize);
			//Allocate the buffer and put file into buffer 
			char *buffer = malloc(*fileSize + 1);
			fread(buffer, *fileSize, 1, fileSpiFlash);
			return buffer;
		}
	} else {
		printf("Error access file %s\n", path);
		return 0;
	}

}

void print_json(char *buffer, unsigned int which_shadow) {
    int i = 0;
	int j = 0;
	char appo[MAX_LEN_SHADOW+1];
	bool primoblocco_changed=true;
	//In una shadow da 5K ci stanno 40 msg da 128 bytes facciamo 30 per sicurezza 
	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"update\":\"completed\",", Etichetta );
    for (j=0; j < NUM_BLOCK; j++) {
		//cerco il bit which_shadow*NUM_BLOCK+j
		//if((changed[((which_shadow*NUM_BLOCK)+j)/8]^(1<<(j%8))) == 0) {
			if((changed[((which_shadow*NUM_BLOCK)+j)/8]&(1<<(j%8))) == (1<<(j%8))) {

			printf("block %d CAMBIATO !!!!!\n", j);
	
			if(primoblocco_changed) {
				sprintf(appo, "\"block%02d\":\"", j);
				primoblocco_changed=false;
			} else {
				sprintf(appo, ",\"block%02d\":\"", j);
			}

			strcat(json_string, appo);	
			for(i=0; i < LEN_BLOCK; i+=4) {
				sprintf(appo, "%02X%02X%02X%02X", ((char *)buffer)[i+j*LEN_BLOCK+which_shadow*LEN_BLOCK*NUM_BLOCK], ((char *)buffer)[i+1+j*LEN_BLOCK+which_shadow*LEN_BLOCK*NUM_BLOCK], ((char *)buffer)[i+2+j*LEN_BLOCK+which_shadow*LEN_BLOCK*NUM_BLOCK], ((char *)buffer)[i+3+j*LEN_BLOCK+which_shadow*LEN_BLOCK*NUM_BLOCK], ((char *)buffer)[i+4+j*LEN_BLOCK+which_shadow*LEN_BLOCK*NUM_BLOCK]);
				strcat(json_string, appo);
			}
			sprintf(appo, "\"");
			strcat(json_string, appo);
		}
	}
	sprintf(appo, "}}}");
	strcat(json_string, appo);
	//printf("json is : %s \n", json_string);

}

//facciamo 4 shadow da 248 lampade ciascuna 
// all'interno 31 blocchi di 8 lampade 
void print_json_energy_and_time(u8 quale_shadow) {
    int i = 0;
	int j = 0;
	int k = 0;
	char appo[MAX_LEN_SHADOW+1];
	bool primoblocco_changed = true;
	//In una shadow da 5K ci stanno 40 msg da 128 bytes facciamo 30 per sicurezza 
	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"update\":\"completed\",", Etichetta);
    strcat(json_string, appo);	
	//printf("appo is %s json is %s", appo, json_string);
		//il primo blocco non ha la virgola davanti. 
	i=0;
	sprintf(appo, "\"block%02d\":\"", i);
	strcat(json_string, appo);
	//printf("appo is %s json is %s", appo, json_string);
	for(j=0; j<10; j++) {
		k = quale_shadow*31+i*10+j;
		printf("%d 0x%08X \n", p_shmem_cenlin->tOn[k], p_shmem_cenlin->tOn[k]);
		sprintf(appo, "%08X%02X%02X%02X%02X%02X%02X", p_shmem_cenlin->tOn[k], p_shmem_cenlin->kWh[k*6], p_shmem_cenlin->kWh[k*6+1], p_shmem_cenlin->kWh[k*6+2], p_shmem_cenlin->kWh[k*6+3], p_shmem_cenlin->kWh[k*6+4], p_shmem_cenlin->kWh[k*6+5]);
		strcat(json_string, appo);
	}	
	sprintf(appo, "\"");
	strcat(json_string, appo);

	for(i=1; i < 31; i++) {
		if(p_shmem_cenlin->gTotNodi > (i*10)) {
			sprintf(appo, ",\"block%02d\":\"", i);
			strcat(json_string, appo);
			for(j=0; j<10; j++) {
				k = quale_shadow*31+i*10+j;
				sprintf(appo, "%08X%02X%02X%02X%02X%02X%02X", p_shmem_cenlin->tOn[k], p_shmem_cenlin->kWh[k*6], p_shmem_cenlin->kWh[k*6+1], p_shmem_cenlin->kWh[k*6+2], p_shmem_cenlin->kWh[k*6+3], p_shmem_cenlin->kWh[k*6+4], p_shmem_cenlin->kWh[k*6+5]);
				strcat(json_string, appo);
			}
			sprintf(appo, "\"");
			strcat(json_string, appo);
		}
	}

	sprintf(appo, ",\"numLamp\":");
	strcat(json_string, appo);	
	sprintf(appo, "%d", p_shmem_cenlin->gTotNodi);
	strcat(json_string, appo);	

	sprintf(appo, ",\"firstLamp\":");
	strcat(json_string, appo);	
	sprintf(appo, "%d", quale_shadow*31);
	strcat(json_string, appo);	


	sprintf(appo, "}}}");
	strcat(json_string, appo);
	//printf("json is : %s \n", json_string);

}


/*
{
  "reported": {
    "cu_type": "logicafm",
    "cu_id": "99998",
    "update": "completed",
    "blocks_modified": "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0000"
  }
}
*/

void print_json_cuconfig(void) {
    int i = 0;
	int j = 0;
	char appo[MAX_LEN_SHADOW+1];
	//In una shadow da 5K ci stanno 40 msg da 128 bytes facciamo 30 per sicurezza 
	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"update\":\"completed\",\"blocks_modified\":\"", Etichetta );
	
	for (j=0; j < (NUM_BLOCK*LEN_BLOCK/8); j++){
		sprintf(appo, "%02X", changed[j]);
		strcat(json_string, appo);	
	}
	sprintf(appo, "\"");
	strcat(json_string, appo);	
	
	sprintf(appo, "}}}");
	strcat(json_string, appo);
	printf("json CUCONFIG is : %s \n", json_string);

}


void print_json_status(void) {
    int i = 0;
	int j = 0;
	char appo[MAX_LEN_SHADOW+1];
	bool primoblocco_changed=true;
	//In una shadow da 5K ci stanno 40 msg da 128 bytes facciamo 30 per sicurezza 
	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"cu_desc\":\"Centrale piano terra\",\"update\":\"completed\",", Etichetta );
    //if (p_shmem_cenlin->status_changed) {
	sprintf(appo, "\"status\":\"");
	strcat(json_string, appo);	
	for(i=0; i < 992; i+=4) {
		sprintf(appo, "%02X%02X%02X%02X", p_shmem_cenlin->status[i], p_shmem_cenlin->status[i+1], p_shmem_cenlin->status[i+2], p_shmem_cenlin->status[i+3]);
		strcat(json_string, appo);
	}
	sprintf(appo, "\"");
	strcat(json_string, appo);
	//}

	sprintf(appo, ",\"numLamp\":");
	strcat(json_string, appo);	
	sprintf(appo, "%d", p_shmem_cenlin->gTotNodi);
	strcat(json_string, appo);	

	sprintf(appo, ",\"timestamp\":");
	strcat(json_string, appo);	
	sprintf(appo, "%d", p_shmem_cenlin->statoCenlin.Timestamp);
	strcat(json_string, appo);	

	sprintf(appo, ",\"testRunning\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%04X\"", p_shmem_cenlin->statoCenlin.testInProgress);
	strcat(json_string, appo);	

	sprintf(appo, ",\"qualitaGSM\":");
	strcat(json_string, appo);	
	sprintf(appo, "%d", p_shmem_cenlin->statoCenlin.StateGsmSignalQuality);
	strcat(json_string, appo);	

	sprintf(appo, ",\"statoGSM\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%04X\"", p_shmem_cenlin->statoCenlin.StateGsm);
	strcat(json_string, appo);	

	sprintf(appo, ",\"statoGSMreg\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%04X\"", p_shmem_cenlin->statoCenlin.StateGsmReg);
	strcat(json_string, appo);	

	sprintf(appo, ",\"statoSim\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%04X\"", p_shmem_cenlin->statoCenlin.StateSim);
	strcat(json_string, appo);	

	sprintf(appo, ",\"statoEth0\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%04X\"", p_shmem_cenlin->statoCenlin.StateEth0);
	strcat(json_string, appo);	

	sprintf(appo, ",\"swVerCentrale\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%04X\"", p_shmem_cenlin->statoCenlin.swVer);
	strcat(json_string, appo);	


	sprintf(appo, ",\"statoSupinv\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%02X%02X%02X%02X%02X%02X%02X%02X\"", p_shmem_cenlin->statoCenlin.StateSupinv[0], p_shmem_cenlin->statoCenlin.StateSupinv[1], p_shmem_cenlin->statoCenlin.StateSupinv[2], 
                                                     	p_shmem_cenlin->statoCenlin.StateSupinv[3], p_shmem_cenlin->statoCenlin.StateSupinv[4], p_shmem_cenlin->statoCenlin.StateSupinv[5],
													    p_shmem_cenlin->statoCenlin.StateSupinv[6], p_shmem_cenlin->statoCenlin.StateSupinv[7]);
	strcat(json_string, appo);	


	sprintf(appo, ",\"supinvFlags\":\"");
	strcat(json_string, appo);	
	sprintf(appo, "%02X%02X%02X%02X%02X%02X%02X%02X\"", p_shmem_cenlin->statoCenlin.supinvFlags[0], p_shmem_cenlin->statoCenlin.supinvFlags[1], p_shmem_cenlin->statoCenlin.supinvFlags[2], 
                                                     	p_shmem_cenlin->statoCenlin.supinvFlags[3], p_shmem_cenlin->statoCenlin.supinvFlags[4], p_shmem_cenlin->statoCenlin.supinvFlags[5],
													    p_shmem_cenlin->statoCenlin.supinvFlags[6], p_shmem_cenlin->statoCenlin.supinvFlags[7]);
	strcat(json_string, appo);	

	sprintf(appo, "}}}");
	strcat(json_string, appo);

	printf("json status is : %s \n", json_string);

}


void print_json_errors(void) {
    int i = 0;
	int j = 0;
	char appo[MAX_LEN_SHADOW+1];
	bool primoblocco_changed=true;
	//In una shadow da 5K ci stanno 40 msg da 128 bytes facciamo 30 per sicurezza 
	sprintf(json_string, "{\"state\":{\"reported\":{\"cu_type\":\"logicafm\",\"cu_id\":\"%5d\",\"update\":\"completed\",", Etichetta );
    
    //if (p_shmem_cenlin->errors_changed) {
	sprintf(appo, "\"errors\":\"");
	strcat(json_string, appo);	
	for(i=0; i < 992; i+=4) {
		sprintf(appo, "%02X%02X%02X%02X", p_shmem_cenlin->errors[i], p_shmem_cenlin->errors[i+1], p_shmem_cenlin->errors[i+2], p_shmem_cenlin->errors[i+3]);
		strcat(json_string, appo);
	}
	sprintf(appo, "\"");
	strcat(json_string, appo);
	//}
	sprintf(appo, ",\"numLamp\":");
	strcat(json_string, appo);	
	sprintf(appo, "%d", p_shmem_cenlin->gTotNodi);
	strcat(json_string, appo);	

	sprintf(appo, "}}}");
	strcat(json_string, appo);

	printf("json errors is : %s \n", json_string);

}


unsigned char compare_buffers(char *buffer, char *buffer1, unsigned int filesize) {
	int i, n, k;
	int diff=0;
	for(i=0;i<256;i++){
		changed[i]=0;
	}
	for(i=0; i<(filesize/LEN_BLOCK); i++ ) {
		//printf("Comparazione blocco %d \n", i);
		if(0 != memcmp(buffer+LEN_BLOCK*i, buffer1+LEN_BLOCK*i, LEN_BLOCK)) {
			printf("Blocco %d cambiato\n", i);
			n = (i/8);
			k = (i%8);
			changed[n] = (changed[n]|(1>>k));
			diff=1;
		} else {
			//printf("Blocco %d invariato\n", i);
		}
	}
	return diff;
}



////static const size_t size = 4*1024;

void* thread_shmem(void* p) {

	//crea una shared memory 
	int shmid_cenlin;
	key_t key=0x1000;

	shmid_cenlin = shmget (key, sizeof(struct shmem_cenlin), IPC_CREAT|0666);
	if(shmid_cenlin<0) {
		printf("Unable to create shared memory\n");
	}
    printf("shmid : %d\n", shmid_cenlin);
	p_shmem_cenlin = (struct shmem_cenlin *) shmat(shmid_cenlin, 0, SHM_RND);

	while (1) {
		printf("Controllo se ci sono stati cambiamenti nei %d nodi presenti  \n\n", p_shmem_cenlin->gTotNodi);
		//Max time the yield function will wait for read messages
		aws_iot_mqtt_yield(&client, 100);
		sleep(1);
	}

}
      

int is_cnfg_file_modified(void) {
	static time_t oldMTime ;
	struct stat file_stat;

	int err = stat(TabFilesExSpiFlash[0].pNomeFileBin, &file_stat);

	if(err != 0) {
		perror("file is modified ");
	}
	if (file_stat.st_mtime > oldMTime) {
		oldMTime = file_stat.st_mtime;
		printf("file cnfg modificato\n");
		return 1;
	} else {
		printf("file cnfg NON modificato\n");
		return 0;
	}

}

int main(int argc, char **argv) {
	bool infinitePublishFlag = true;
	unsigned int len_file = 0;
	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];
	char cPayload[MAX_LEN_SHADOW+1];
    char str_topic_shadow[100];
	int j=0;
	int32_t i = 0;
	bool primavolta = true;
	char *buffer_saved ;
	int res;
	pthread_t id_thread0;
	int published;
	time_t t;
	struct tm time_now; 
	u32 cnt_cicle=0;
	char appo_str[30];
    //	Load etichetta CenLin
    Etichetta = ReadEtichettaCentrale ();
    printf("Etichetta is %d \n", Etichetta);
	setvbuf(stdout, NULL, _IONBF, 0); // turn off buffering for stdout
	printf("create thread \n");
	res = pthread_create(&id_thread0, NULL, thread_shmem, NULL);
	if (res != 0) {
		printf("Failed creation thread - thread_shmem\n");
		return 0;
	}
	sleep(10);
	IoT_Error_t rc = FAILURE;

	//AWS_IoT_Client client;
	IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
	IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

	IoT_Publish_Message_Params paramsQOS0;
	IoT_Publish_Message_Params paramsQOS1;

	strncpy(HostAddress, "a1b5pwfg95b14c-ats.iot.eu-west-1.amazonaws.com\0", HOST_ADDRESS_SIZE);
	
	parseInputArgsForConnectParams(argc, argv);

	IOT_INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

	getcwd(CurrentWD, sizeof(CurrentWD));
	snprintf(rootCA, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_ROOT_CA_FILENAME);
	snprintf(clientCRT, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_CERTIFICATE_FILENAME);
	snprintf(clientKey, PATH_MAX + 1, "%s/%s/%s", CurrentWD, certDirectory, AWS_IOT_PRIVATE_KEY_FILENAME);

	IOT_DEBUG("rootCA %s", rootCA);
	IOT_DEBUG("clientCRT %s", clientCRT);
	IOT_DEBUG("clientKey %s", clientKey);
	mqttInitParams.enableAutoReconnect = false; // We enable this later below
	mqttInitParams.pHostURL = HostAddress;
	mqttInitParams.port = port;
	mqttInitParams.pRootCALocation = rootCA;
	mqttInitParams.pDeviceCertLocation = clientCRT;
	mqttInitParams.pDevicePrivateKeyLocation = clientKey;
	mqttInitParams.mqttCommandTimeout_ms = 20000;
	mqttInitParams.tlsHandshakeTimeout_ms = 5000;
	mqttInitParams.isSSLHostnameVerify = true;
	mqttInitParams.disconnectHandler = disconnectCallbackHandler;
	mqttInitParams.disconnectHandlerData = NULL;

	rc = aws_iot_mqtt_init(&client, &mqttInitParams);
	if(SUCCESS != rc) {
		IOT_ERROR("aws_iot_mqtt_init returned error : %d ", rc);
		return rc;
	}

	connectParams.keepAliveIntervalInSec = 30;
	connectParams.isCleanSession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = AWS_IOT_MQTT_CLIENT_ID;
	connectParams.clientIDLen = (uint16_t) strlen(AWS_IOT_MQTT_CLIENT_ID);
	connectParams.isWillMsgPresent = false;


	IOT_DEBUG("Finalmente tento la connessione :  %s %d ", connectParams.pClientID, connectParams.isCleanSession);

	IOT_INFO("Connecting...");
	rc = aws_iot_mqtt_connect(&client, &connectParams);
	if(SUCCESS != rc) {
		IOT_ERROR("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
		return rc;
	}
	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
	if(SUCCESS != rc) {
		IOT_ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	IOT_INFO("Subscribing...");
	sprintf(appo_str, "logicafm_%5d/command_fm", Etichetta);
	rc = aws_iot_mqtt_subscribe(&client, appo_str, 25, QOS0, iot_subscribe_callback_handler, NULL);
	if(SUCCESS != rc) {
		IOT_ERROR("Error subscribing : %d ", rc);
		return rc;
	}


	rc = aws_iot_mqtt_subscribe(&client, "sdkTest/sub", 11, QOS0, iot_subscribe_callback_handler, NULL);
	if(SUCCESS != rc) {
		IOT_ERROR("Error subscribing : %d ", rc);
		return rc;
	}

	sprintf(cPayload, "%s : %d ", "hello from SDK", i);

	paramsQOS0.qos = QOS0;
	paramsQOS0.payload = (void *) cPayload;
	paramsQOS0.isRetained = 0;

	paramsQOS1.qos = QOS1;
	paramsQOS1.payload = (void *) cPayload;
	paramsQOS1.isRetained = 0;

	if (publishCount != 0) {
		infinitePublishFlag = false;
	}
	
	//print_json(buffer_start, );

	//create_json_shadow_0(TabFilesExSpiFlash[0].pNomeFileBin, );
	//if(gTotNodi>50)
    //	create_json_shadow_1();
	//la prima volta voglio mandare su tutto 
	for(i=0;i<256;i++){
		changed[i]=0xFF;
	}
	p_shmem_cenlin->status_changed = 1;
	p_shmem_cenlin->errors_changed = 1;
	while((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc)
		  && (publishCount > 0 || infinitePublishFlag)) {

/*
		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(&client, 100);
		if(NETWORK_ATTEMPTING_RECONNECT == rc) {
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}
*/

		cnt_cicle++;
		printf("publishCount is %d(%d) infinitePublishFlag is %d \n", publishCount, cnt_cicle, infinitePublishFlag);	  


		if (is_cnfg_file_modified()) {
		
			char *buffer_start = readCnfgToRam(TabFilesExSpiFlash[0].pNomeFileBin, &len_file);
			if(primavolta) {
				primavolta = false;
				buffer_saved = malloc(len_file + 1);
				printf("len file is %d \n", len_file);
				memcpy(buffer_saved, buffer_start, len_file);
			} else {
				printf("len_file is %d\n", len_file);
				if (0!=compare_buffers(buffer_saved, buffer_start, len_file)) {
					memcpy(buffer_saved, buffer_start, len_file);
				}
			}

			num_shadow = (int) (len_file/(NUM_BLOCK*LEN_BLOCK))+1;
			i=0;
			while (i < num_shadow) {
				//IOT_INFO("Publish Shadow%02d of %02d(len is %d)\n", i, num_shadow, len_file);
				//create_json_shadow_0();
				//printf("changed %d is %d \n", i, changed[i]);
				if((changed[i*4] != 0) || (changed[i*4+1] != 0 ) || (changed[i*4+2] != 0) || (changed[i*4+3] != 0)) {
					print_json(buffer_start, i);
					//sleep(1);
					sprintf(cPayload, "%s", json_string);
					printf("Send shadow %d (len %d )  %s \n", i, strlen(json_string), json_string);
					//printf("Send shadow %d (len %d ) : %s  \n", i, strlen(cPayload), cPayload);
					paramsQOS0.payloadLen = strlen(cPayload);
					sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/param%02d/update", Etichetta, i);
					//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
					rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
					printf("Publish returned : %d \n", rc);
					published |= SHADOW_CONFIG_PUBLISHED;
				}	
				i++;
			}
		}

		if(p_shmem_cenlin->status_changed) {
			print_json_status();
			sprintf(cPayload, "%s", json_string);
			//printf("Send shadow status (len %d )  \n", strlen(json_string));//, json_string);
			printf("Send shadow status : %s  \n", cPayload);
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/status/update", Etichetta);
			//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);
			if(p_shmem_cenlin->status_changed)
				published |= SHADOW_STATUS_BASE_STATUS_PUBLISHED;
			p_shmem_cenlin->status_changed = 0;

		}

		if(p_shmem_cenlin->errors_changed) {
			print_json_errors();
			sprintf(cPayload, "%s", json_string);
			//printf("Send shadow status (len %d )  \n", strlen(json_string));//, json_string);
			printf("Send shadow status : %s  \n", cPayload);
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/errors/update", Etichetta);
			//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);
			if(p_shmem_cenlin->errors_changed)
				published |= SHADOW_STATUS_BASE_ERROR_PUBLISHED;
			p_shmem_cenlin->errors_changed = 0;
		}
		

		if (flag_tx_json_command_lo == TRUE) {
			flag_tx_json_command_lo = FALSE;
			print_json_command_lo(bufferTx);
			sprintf(cPayload, "%s", json_string);
			//printf("Send shadow cuconfig %d (len %d )  \n", i, strlen(json_string));//, json_string);
			printf("Send shadow command lo (len %d )  \n", strlen(cPayload));
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "logicafm_%5d/response", Etichetta);
			//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);
		}
		

		if (flag_tx_json_command_fd == TRUE) {
			flag_tx_json_command_fd = FALSE;
			print_json_command_fd(bufferTx);
			sprintf(cPayload, "%s", json_string);
			//printf("Send shadow cuconfig %d (len %d )  \n", i, strlen(json_string));//, json_string);
			printf("Send shadow command fd (len %d )  \n", strlen(cPayload));
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "logicafm_%5d/response", Etichetta);
			//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);
		}
		


		t = time(NULL);
		time_now= *localtime (&t);
		printf("Sono le %02d:%02d:%02d  \n",time_now.tm_hour, time_now.tm_min, time_now.tm_sec);
		//if((time_now.tm_hour == 10)&&(time_now.tm_min == 10)) {
			if((time_now.tm_min == 00)&&(time_now.tm_sec < 30)) {
			//per il momento mando su i dati una volta ogni ora 
			//ogni giorno alle 24 pubblico i dati di energia consumata e tempo di accensione 
			print_json_energy_and_time(0);
			sprintf(cPayload, "%s", json_string);
			printf("Send shadow %d (len %d )  -%s- \n", i, strlen(json_string), json_string);
			//printf("Send shadow %d (len %d ) : %s  \n", i, strlen(cPayload), cPayload);
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/energy_time_00/update", Etichetta, i);
			//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);

			if(p_shmem_cenlin->gTotNodi>248) {
				print_json_energy_and_time(1);
				sprintf(cPayload, "%s", json_string);
				printf("Send shadow %d (len %d )  %s \n", i, strlen(json_string), json_string);
				//printf("Send shadow %d (len %d ) : %s  \n", i, strlen(cPayload), cPayload);
				paramsQOS0.payloadLen = strlen(cPayload);
				sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/energy_time_01/update", Etichetta);
				//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
				rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
				printf("Publish returned : %d \n", rc);
			}

			if(p_shmem_cenlin->gTotNodi>(248*2)) {
				print_json_energy_and_time(2);
				sprintf(cPayload, "%s", json_string);
				printf("Send shadow %d (len %d )  %s \n", i, strlen(json_string), json_string);
				//printf("Send shadow %d (len %d ) : %s  \n", i, strlen(cPayload), cPayload);
				paramsQOS0.payloadLen = strlen(cPayload);
				sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/energy_time_02/update", Etichetta);
				//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
				rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
				printf("Publish returned : %d \n", rc);
			}

			if(p_shmem_cenlin->gTotNodi>(248*3)) {
				print_json_energy_and_time(3);
				sprintf(cPayload, "%s", json_string);
				printf("Send shadow %d (len %d )  %s \n", i, strlen(json_string), json_string);
				//printf("Send shadow %d (len %d ) : %s  \n", i, strlen(cPayload), cPayload);
				paramsQOS0.payloadLen = strlen(cPayload);
				sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/energy_time_03/update", Etichetta);
				//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );					
				rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);																																																																	
				printf("Publish returned : %d \n", rc);
			}																																																																																																																										

			published |= SHADOW_ENERGY_TIME_PUBLISHED;
		}																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																												

		if((published & SHADOW_ENERGY_TIME_PUBLISHED) != 0 ) {
			print_json_status();
			sprintf(cPayload, "%s", json_string);	
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/status/update", Etichetta);
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);
			published &=~SHADOW_ENERGY_TIME_PUBLISHED;
		}

		if(0 != published) {
			printf("Trigghero la lamda perche' e' cambiato %d \n", published);
			print_json_cuconfig();
			sprintf(cPayload, "%s", json_string);
			//printf("Send shadow cuconfig %d (len %d )  \n", i, strlen(json_string));//, json_string);
			printf("Send shadow cuconfig %d (len %d )  \n", i, strlen(cPayload));//, cPayload);
			paramsQOS0.payloadLen = strlen(cPayload);
			sprintf(str_topic_shadow, "$aws/things/logicafm_%5d/shadow/name/cuconfig/update", Etichetta);
			//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
			rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
			printf("Publish returned : %d \n", rc);
			published = 0;
		}
		
/*
		for(j=1;j<20;j++) {
			if(gTotNodi > (j*50)) {
				IOT_INFO("Publish Shadow%d : ", j);
				create_json_shadow_lum(j);
				//sleep(1);
				sprintf(cPayload, "%s", json_string);
				paramsQOS0.payloadLen = strlen(cPayload);
				sprintf(str_topic_shadow, "%s%02d%s", "$aws/things/logicafm_99998/shadow/name/lamps",j,"/update");
				//IOT_INFO("sending : %s on %s\n",cPayload, str_topic_shadow );
				rc = aws_iot_mqtt_publish(&client, str_topic_shadow, strlen(str_topic_shadow), &paramsQOS0);
				printf("publish returned : %d \n", rc);
			}
		}
*/
		if(publishCount > 0) {
			publishCount--;
		}


		if(publishCount == 0 && !infinitePublishFlag) {
			break;
		}
/*
		sprintf(cPayload, "%s : %d ", "hello from SDK QOS1", i++);
		paramsQOS1.payloadLen = strlen(cPayload);
		rc = aws_iot_mqtt_publish(&client, "sdkTest/sub", 11, &paramsQOS1);
		printf("publish return : %d \n", rc);
		if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
			IOT_WARN("QOS1 publish ack not received.\n");
			rc = SUCCESS;
		}
		if(publishCount > 0) {
			publishCount--;
		}
		*/
		sleep(10);
	}

	// Wait for all the messages to be received vittorio
	aws_iot_mqtt_yield(&client, 100);

	if(SUCCESS != rc) {
		IOT_ERROR("An error occurred in the loop.\n");
	} else {
		IOT_INFO("Publish done\n");
	}

	return rc;
}
