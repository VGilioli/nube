/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Eeprom.h
 * Author: variscite
 *
 * Created on December 5, 2018, 5:00 PM
 */

#ifndef EEPROM_H
#define EEPROM_H


typedef int boolean;
typedef uint32_t u32;
typedef uint16_t u16;
typedef __int16_t s16;
typedef uint8_t u8;
typedef char s8;


#define		SPI_OPC_READ		0x03
#define		SPI_OPC_4K_ERASE        0x20
#define		SPI_OPC_32K_ERASE       0x52
#define		SPI_OPC_64K_ERASE       0xd8
#define		SPI_OPC_CHIP_ERASE      0x60
#define		SPI_OPC_BYTE_WRITE      0x02
#define		SPI_OPC_AAI_PROG        0xad
#define		SPI_OPC_RDSR            0x05
#define		SPI_OPC_EWSR            0x50
#define		SPI_OPC_WRSR            0x01
#define		SPI_OPC_WREN            0x06
#define		SPI_OPC_WRDI            0x04
#define		SPI_OPC_RDID            0x90
#define		SPI_OPC_JEDEC_ID        0x9f
#define		SPI_OPC_EBSY            0x70
#define		SPI_OPC_DBSY            0x80

#define MAX_NUM_UART        18

typedef struct
{
    u32     AddrStartSpiFlash;
    u32     AddrEndSpiFlash;
    char    *pNomeFileBin;
} tTabFilesExSpiFlash; 




//------------------------------------------------------------------------------
//	Mappatura SPI FLASH SupInv (sst25vf016b) 16 mega bit:
//	000000..01FFFF		128k		Parametri di sistema
//	020000..05FFFF		256k		Log book
//	060000..09FFFF		256k		free..
//	0A0000..0DFFFF		256k		Snapshot nodi radio
//	0E0000..0EFFFF		64k		free
//	0F0000..0F2FFF		12K		Bmp Logo Beghelli	               
//	0F3000..0F3FFF		4k		free
//	0F4000..0F7FFF		16k		Bmp Personal Computer portatile				
//	0F8000..0F8FFF		4k		Bmp antenna gsm 
//	0F9000..	       	8k		free
//	0FB000..      		8k		free
//	0FD000..    	   	8k		free
//	0FF000..        	8k		free
//	101000..      		8k		free
//	103000..		4k              Bmp Freccia avanti
//	104000..		512k            Programma upgrade modulini radio upgrade sw centrale		
//	184000..		8k              Bitmap bandiere
//	186000..		8k              Bitmap Rs485
//	188000..		4k              Bitmap mondo internet
//	189000..1DFFFF   	348k 		Parametri di sistema (extensione)
//      1E0000..1E7FFF		32k  		Zona memorizzazione dei consumi lampade illuminazione smartdriver e/o stato logica FM
//	........1FFFFF					free		
//
//------------------------------------------------------------------------------
#define     SPI_FLASH_START_ADDR			0
#define     SPI_FLASH_END_ADDR                          0x1FFFFF
#define     SPI_FLASH_LEN				((SPI_FLASH_END_ADDR-SPI_FLASH_START_ADDR)+1)

#define     SPI_FLASH_PARAM_START_ADDR                  0
#define     SPI_FLASH_PARAM_END_ADDR                    0x1FFFF
#define     SPI_FLASH_PARAM_LEN				((SPI_FLASH_PARAM_END_ADDR-SPI_FLASH_PARAM_START_ADDR)+1)

#define     SPI_FLASH_LOG_BOOK_START_ADDR         	0x020000
#define     SPI_FLASH_LOG_BOOK_END_ADDR           	0x05ffff
#define     SPI_FLASH_LOG_BOOK_LEN                	0x040000

#define     SPI_FLASH_SNAPSHOT_START_ADDR         	0x0a0000
#define     SPI_FLASH_SNAPSHOT_END_ADDR           	0x0dffff
#define     SPI_FLASH_SNAPSHOT_LEN                	0x040000

#define     SPI_FLASH_BMP_LOGO_BEGHELLI_START_ADDR	0x0f0000
#define     SPI_FLASH_BMP_LOGO_BEGHELLI_END_ADDR	0x0f2e31
#define     SPI_FLASH_BMP_LOGO_BEGHELLI_LEN             0x002e32

#define     SPI_FLASH_BMP_PC_START_ADDR			0x0f4000
#define     SPI_FLASH_BMP_PC_END_ADDR			0x0f7fff
#define     SPI_FLASH_BMP_PC_LEN			0x4000

#define     SPI_FLASH_BMP_ANTENNA_GSM_START_ADDR	0x0f8000
#define     SPI_FLASH_BMP_ANTENNA_GSM_END_ADDR		0x0f8fff
#define     SPI_FLASH_BMP_ANTENNA_GSM_LEN		0x1000

#define     SPI_FLASH_BMP_WIFI_START_ADDR               0x103000
#define     SPI_FLASH_BMP_WIFI_END_ADDR                 0x103fff
#define     SPI_FLASH_BMP_WIFI_LEN                      0x1000

//	Programma di upgrade per modulini radio e proxy (e anche centrale stessa) 512K
#define     SPI_FLASH_PROGRAMMA_UPGRADE_START_ADDR	0x104000
#define     SPI_FLASH_PROGRAMMA_UPGRADE_END_ADDR	0x183fff
#define     SPI_FLASH_PROGRAMMA_UPGRADE_LEN		0x80000

#define     SPI_FLASH_BMP_BANDIERE_START_ADDR		0x184000
#define     SPI_FLASH_BMP_BANDIERE_END_ADDR		0x185fff
#define     SPI_FLASH_BMP_BANDIERE_LEN			0x2000

#define     SPI_FLASH_BMP_RS485_START_ADDR		0x186000
#define     SPI_FLASH_BMP_RS485_END_ADDR		0x187fff
#define     SPI_FLASH_BMP_RS485_LEN			0x2000

#define     SPI_FLASH_BMP_MONDO_INTERNET_START_ADDR	0x188000
#define     SPI_FLASH_BMP_MONDO_INTERNET_END_ADDR	0x188fff
#define     SPI_FLASH_BMP_MONDO_INTERNET_LEN		0x1000

//	Estensione della zona parametri...
#define     SPI_FLASH_PARAM_EXT_START_ADDR		0x189000
#define     SPI_FLASH_PARAM_EXT_END_ADDR		0x1DFFFF
#define     SPI_FLASH_PARAM_EXT_LEN			((SPI_FLASH_PARAM_EXT_END_ADDR-SPI_FLASH_PARAM_EXT_START_ADDR)+1)

//	Zona memorizzazione dei consumi lampade illuminazione smartdriver e/o stato logica FM
#define     SPI_FLASH_DATI_CONSUMO_NODI_START_ADDR	0x1E0000
#define     SPI_FLASH_DATI_CONSUMO_NODI_END_ADDR	0x1E7FFF
#define     SPI_FLASH_DATI_CONSUMO_NODI_LEN		0x8000

#define     SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN		32
#define     SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN_USED	26

//	Record dati consumo lampade UMDL
#define     DATI_SD_CONS_DATA					0
#define     DATI_SD_CONS_WATT					4
#define     DATI_SD_CONS_KWH					8
#define     DATI_SD_CONS_TIME_ON				14
#define     DATI_SD_CONS_POT_MIN				18
#define     DATI_SD_CONS_POT_MAX				20
#define     DATI_SD_CONS_STATO_BALLAST                          22
#define     DATI_SD_CONS_STATO_ON_OFF_ERR_LAMP                  23
#define     DATI_SD_CONS_STATUS					24	// 2 bytes

//	Record dati consumo lampade logica FM
#define     DATI_FM_DATA					0
#define     DATI_FM_STATO					4
#define     DATI_FM_ERROR					5
#define     DATI_FM_STATO_SA					6
#define     DATI_FM_STATO_2					7
#define     DATI_FM_STATO_ON_OFF_ERR_LAMP                       8
#define     DATI_FM_STATUS					9	// 2 bytes

//	Record dati consumo trasmettitori domotici 20104
#define     DATI_20104_DATA					0
#define     DATI_20104_STATUS					4	// 2 bytes


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	Codici operativi di memorizzazione di log book, log disgiuntori, log snapshot ecc.	
//------------------------------------------------------------------------------
#define		SPI_FLASH_OPC_LOG_DUMMY					0x10
#define		SPI_FLASH_OPC_LOG_MIS_DIG				0x17
#define		SPI_FLASH_OPC_LOG_RESET					0x20
#define		SPI_FLASH_OPC_LOG_MANCANZA_RETE				0x24
#define		SPI_FLASH_OPC_LOG_RITORNO_RETE				0x25
#define		SPI_FLASH_OPC_LOG_MODEM_TX_AT				0x26
#define		SPI_FLASH_OPC_LOG_MODEM_RX_AT				0x27
#define		SPI_FLASH_OPC_LOG_GSM_ON				0x28
#define		SPI_FLASH_OPC_LOG_GSM_OFF				0x29
#define		SPI_FLASH_OPC_LOG_TEST_FUNZ				0x2a
#define		SPI_FLASH_OPC_LOG_TEST_AUT				0x2b
#define		SPI_FLASH_OPC_LOG_START_TEST_FUNZ_MAN                   0x2c
#define		SPI_FLASH_OPC_LOG_START_TEST_AUT_MAN                    0x2d
#define		SPI_FLASH_OPC_LOG_STOP_TEST				0x2e
#define		SPI_FLASH_OPC_LOG_ERR_START_TEST_FUNZ_MAN               0x2f
#define		SPI_FLASH_OPC_LOG_ERR_START_TEST_AUT_MAN                0x30
#define		SPI_FLASH_OPC_LOG_ERR_STOP_TEST				0x31
#define		SPI_FLASH_OPC_LOG_TASK_DATE_TIME_RUN                    0x32
#define		SPI_FLASH_OPC_LOG_VAR_DEBUG				0x33
#define		SPI_FLASH_OPC_LOG_SCENARI				0x34
#define		SPI_FLASH_OPC_LOG_MODEM_RX_FD				0x35

#define		SPI_FLASH_OPC_LOG_SNAP_ITL				0x60
#define		SPI_FLASH_OPC_LOG_SNAP_INVERTER                         0x61
#define		SPI_FLASH_OPC_LOG_SNAP_CENTRALE_ACS                     0x62
#define		SPI_FLASH_OPC_LOG_SNAP_LOGICA_FM                        0x63
#define		SPI_FLASH_OPC_LOG_SNAP_STM				0x64
#define		SPI_FLASH_OPC_LOG_SNAP_CONTARISPARMIO                   0x65
#define		SPI_FLASH_OPC_LOG_SNAP_LAMPADE_LED                      0x66
#define		SPI_FLASH_OPC_LOG_SNAP_BALERA                           0x67
#define		SPI_FLASH_OPC_LOG_SNAP_MISRAD                           0x68
#define		SPI_FLASH_OPC_LOG_SNAP_MISPOT                           0x69
#define		SPI_FLASH_OPC_LOG_SNAP_SENS_AUT                         0x6A
#define		SPI_FLASH_OPC_LOG_SNAP_ZIGBAL                           0x6B
#define		SPI_FLASH_OPC_LOG_SNAP_CONCENTRATORE                    0x6C
#define		SPI_FLASH_OPC_LOG_SNAP_SENSORE_FOT                      0x6D
#define		SPI_FLASH_OPC_LOG_SNAP_TRASM_DOMOTICO                   0x6E
#define		SPI_FLASH_OPC_LOG_SNAP_RELAIS_DOMOTICO                  0x6F
#define		SPI_FLASH_OPC_LOG_SNAP_GP_ATMEL                         0x70
#define		SPI_FLASH_OPC_LOG_SNAP_MISURATORE                       0x71
#define		SPI_FLASH_OPC_LOG_SNAP_AMADORI                          0x72


//------------------------------------------------------------------------------
//	Log delle cause di reset della centrale SupInv	
//------------------------------------------------------------------------------
#define		NO_RESET_CAUSE						0
#define		LOG_RESET_CAUSE_REMOTE_CMD				1
#define		LOG_RESET_CAUSE_KEYBOARD				2
#define		LOG_RESET_CAUSE_WATCHDOG				3


//------------------------------------------------------------------------------
//	Costanti che identificano i diversi tipi di log memorizzati
//------------------------------------------------------------------------------
#define		LOG_BOOK						0x00
#define		LOG_DIG  						0x01
#define		LOG_SNAPSHOT						0x02
#define		NRO_TOT_LOG						0x03

//------------------------------------------------------------------------------

#define		SPI_FLASH_MASK_BIT_SECTOR_4K                            0xFFF
#define		SPI_FLASH_MASK_BIT_SECTOR_32K                           0x7FFF
#define		SPI_FLASH_MASK_BIT_SECTOR_64K                           0xFFFF



//	Struttura per inizializzazione dei parametri della centrale SupInv (in spiflash)
//	Addr :		Indirizzo della spi flash
//	NroBytes :	Quanti bytes inizializzare al valore [Val]
//	Val :		Valore di inizializzazione della spi flash
typedef struct 
{
  u32		Addr;
  u16		NroBytes;
  u8		Val;
} tTabSpiParam;

//------------------------------------------------------------------------------
//	Address parametri del sistema (vedi documentazione W:\WORK\PROGETTI\Cenlin\Documenti SW Becar\ParametriCenlin.doc) 				
//------------------------------------------------------------------------------
#define		SPI_FLASH_ADDR_PAR_PALETTO                                      0x0000
#define		SPI_FLASH_ADDR_PAR_VER_SW                                       0x0002
#define		SPI_FLASH_ADDR_PAR_NR_RING_AUTO_ANSWER                          0x0004
#define		SPI_FLASH_ADDR_PAR_FREE_0x0005      				0x0005
#define		SPI_FLASH_ADDR_PAR_RADIO_ID  					0x0006
#define		SPI_FLASH_ADDR_PAR_FREE_EX_FLAGS                                0x0008
#define		SPI_FLASH_ADDR_PAR_NTEL_SMS					0x000C
#define		SPI_FLASH_ADDR_PAR_DIS_CAUSE_ALLARME                            0x010C
#define		SPI_FLASH_ADDR_PAR_TOUT_FILT_CAUSE_ALLARME                      0x0110
#define		SPI_FLASH_ADDR_PAR_PERC_ADD_SP_CALIB                            0x0190
#define		SPI_FLASH_ADDR_PAR_MAX_BYTES_WRITE_FLASH_MOD_RAD                0x0191
#define		SPI_FLASH_ADDR_PAR_LOGO_BEGHELLI				0x0192
#define		SPI_FLASH_ADDR_PAR_NOME_IMPIANTO				0x01A8
#define		SPI_FLASH_ADDR_PAR_TIPO_CENTRALE                                0x01B8
#define		SPI_FLASH_ADDR_PAR_FREE_0x01B9                                  0x01B9
#define		SPI_FLASH_ADDR_PAR_TOUT_AUTOESCLUSIONE                          0x01BA
#define		SPI_FLASH_ADDR_PAR_EN_NODE_CAUSE_ALLARME                        0x023A
#define		SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST                          0x11BA
#define		SPI_FLASH_ADDR_PAR_exDIG_FREE                                   0x11BE
#define		SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST                          0x11C2
#define		SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG                      	0x11C6
#define		SPI_FLASH_ADDR_PAR_TAB_ALBA_TRAMONTO                            0x11CC
#define		SPI_FLASH_ADDR_PAR_LATITUDE					0x1780
#define		SPI_FLASH_ADDR_PAR_ROUTER_SD_SERVER_ADDR                        0x1784
#define		SPI_FLASH_ADDR_PAR_ROUTER_SD_SERVER_PORT                        0x17C4
#define		SPI_FLASH_ADDR_PAR_FLAGS					0x17C6
#define		SPI_FLASH_ADDR_PAR_ETH0_PORT_NR	 				0x17CE
#define		SPI_FLASH_ADDR_PAR_WIFI_PORT_NR	 				0x17D0
#define		SPI_FLASH_ADDR_PAR_FREE_0x17D2                                  0x17D2
#define		SPI_FLASH_ADDR_PAR_NODE_DALI_GROUPS_20104_IN2                   0x1840
#define		SPI_FLASH_ADDR_PAR_CONFIG_NODES					0x2000
#define		SPI_FLASH_ADDR_PAR_NOMI_UTENTE					0x9C00
#define		SPI_FLASH_ADDR_PAR_ALT_FATHER					0xA000
#define		SPI_FLASH_ADDR_PAR_LG_FM_OPTICOM				0xC6C0
#define		SPI_FLASH_ADDR_PAR_WIFI_AP_NOME_RETE                            0xCAA0      //  64 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_AP_PASSPHRASE                           0xCAE0      //  18 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_AP_IPV4_ADDR                            0xCAF2      //  4 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_AP_IPV4_NETMASK                         0xCAF6      //  4 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_AP_IPV4_GATEWAY                         0xCAFA      //  4 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_STA_NOME_RETE                           0xCAFE      //  64 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_STA_PASSPHRASE                          0xCB3E      //  18 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_STA_IPV4_ADDR                           0xCB50      //  4 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_STA_IPV4_NETMASK                        0xCB54      //  4 bytes
#define		SPI_FLASH_ADDR_PAR_WIFI_STA_IPV4_GATEWAY                        0xCB58      //  4 bytes
#define		SPI_FLASH_ADDR_PAR_FREE_3	 				0xCB5C
#define		SPI_FLASH_ADDR_PAR_TOT_NODI  					0xD000
//  20 uart virtuali previste (fino ad ora nella realtà sono 18..)
#define		SPI_FLASH_ADDR_PAR_TIPO_PROT_UART 				0xD002
//  20 uart modbus previste (fino ad ora nella realtà sono 18..)
#define		SPI_FLASH_ADDR_PAR_MODBUS_ADDR_UART				0xD016
//  20 uart modbus previste (fino ad ora nella realtà sono 18..)
#define		SPI_FLASH_ADDR_PAR_ADDR_BUS_SUPINV_UART				0xD02A

#define         SPI_FLASH_ADDR_PAR_FREE_0xD03E                                  0xD03E
#define		SPI_FLASH_ADDR_PAR_DELAY_START_POLL				0xD41A
#define		SPI_FLASH_ADDR_PAR_TOUT_RIBADIRE_EVENTI                         0xD41E
#define		SPI_FLASH_ADDR_PAR_LAST_SCENE_ACTIVATED                         0xD422
#define		SPI_FLASH_ADDR_PAR_FREE_0xD423	 				0xD423
#define		SPI_FLASH_ADDR_PAR_FREE_0xD426                                  0xD426
#define		SPI_FLASH_ADDR_PAR_FREE_0xD42A                                  0xD42A
#define		SPI_FLASH_ADDR_PAR_FREE_0xD42B                                  0xD42B
#define		SPI_FLASH_ADDR_PAR_FREE_0xD42C                                  0xD42C
#define		SPI_FLASH_ADDR_PAR_FREE_0xD42D                                  0xD42D
#define		SPI_FLASH_ADDR_PAR_ORARIO_SNAPSHOT				0xD431
#define		SPI_FLASH_ADDR_PAR_FREQ_MEM_SNAPSHOT                            0xD434
#define		SPI_FLASH_ADDR_PAR_FREE_D438                                    0xD438
#define		SPI_FLASH_ADDR_PAR_ERR_COMM_12H					0xD43C
#define		SPI_FLASH_ADDR_PAR_ERR_COMM_24H					0xD440
#define		SPI_FLASH_ADDR_PAR_FREE_0xD444                                  0xD444
#define		SPI_FLASH_ADDR_PAR_FREE_0xD448      				0xD448
#define		SPI_FLASH_ADDR_PAR_FREE_0xD44C                                  0xD44C
#define		SPI_FLASH_ADDR_PAR_MAP_BIT_LOG_DISABLED                         0xD450
#define		SPI_FLASH_ADDR_PAR_ENERGIA_CONSUMATA                            0xD470
#define		SPI_FLASH_ADDR_PAR_ENERGIA_SOSTITUITA                           0xD474
#define		SPI_FLASH_ADDR_PAR_POT_SOST_NODI_UMDL                           0xD478
#define		SPI_FLASH_ADDR_PAR_FREE_DC38                                    0xDC38
#define		SPI_FLASH_ADDR_PAR_FREE_DE38                                    0xDE38
#define		SPI_FLASH_ADDR_PAR_FREE_E038                                    0xE038
#define		SPI_FLASH_ADDR_PAR_FREE_E238                                    0xE238
#define		SPI_FLASH_ADDR_PAR_FREE_E438                                    0xE438
#define		SPI_FLASH_ADDR_PAR_FREE_E638                                    0xE638
#define		SPI_FLASH_ADDR_PAR_PERC_ADD_POT_MIN_PRECALIB                    0xE838
#define		SPI_FLASH_ADDR_PAR_PERC_SUB_SP_PRECALIB                         0xE839
#define		SPI_FLASH_ADDR_UPG_VALID_APPL_BOOT				0xE83A
#define		SPI_FLASH_ADDR_UPG_ERASE_FROM_BOOT				0xE83E
#define		SPI_FLASH_ADDR_UPG_ERASE_TO_BOOT				0xE842
#define		SPI_FLASH_ADDR_UPG_PROGRAM_FROM_BOOT                            0xE846
#define		SPI_FLASH_ADDR_UPG_PROGRAM_TO_BOOT				0xE84A
#define		SPI_FLASH_ADDR_UPG_CRC_FROM_BOOT				0xE84E
#define		SPI_FLASH_ADDR_UPG_CRC_TO_BOOT					0xE852
#define		SPI_FLASH_ADDR_UPG_CRC_WHERE_BOOT				0xE856
#define		SPI_FLASH_ADDR_UPG_SW_VER_BOOT					0xE85A
#define		SPI_FLASH_ADDR_UPG_NODE_TYPE_BOOT				0xE85E
#define		SPI_FLASH_ADDR_UPG_NRO_PROXY_BOOT				0xE862
#define		SPI_FLASH_ADDR_UPG_VALID_APPL_APPL				0xE866
#define		SPI_FLASH_ADDR_UPG_ERASE_FROM_APPL				0xE86A
#define		SPI_FLASH_ADDR_UPG_ERASE_TO_APPL				0xE86E
#define		SPI_FLASH_ADDR_UPG_PROGRAM_FROM_APPL                            0xE872
#define		SPI_FLASH_ADDR_UPG_PROGRAM_TO_APPL				0xE876
#define		SPI_FLASH_ADDR_UPG_CRC_FROM_APPL				0xE87A
#define		SPI_FLASH_ADDR_UPG_CRC_TO_APPL					0xE87E
#define		SPI_FLASH_ADDR_UPG_CRC_WHERE_APPL				0xE882
#define		SPI_FLASH_ADDR_UPG_SW_VER_APPL					0xE886
#define		SPI_FLASH_ADDR_UPG_NODE_TYPE_APPL				0xE88A
#define		SPI_FLASH_ADDR_UPG_NRO_PROXY_APPL				0xE88E
#define		SPI_FLASH_ADDR_PAR_UMDL_TAR_INC_DEC_SP                          0xE892
#define		SPI_FLASH_ADDR_PAR_UMDL_TAR_PERC_WATT_MOD_SP                    0xE893
#define		SPI_FLASH_ADDR_PAR_UMDL_TAR_PERC_LUM_MOD_SP                     0xE894
#define		SPI_FLASH_ADDR_PAR_CVPS_WORK_WITH				0xE895
#define		SPI_FLASH_ADDR_PAR_NODE_DALI_GROUPS				0xE897
#define		SPI_FLASH_ADDR_PAR_MSG_RADIO_EV_AUTOMAZIONE                     0xF057
#define		SPI_FLASH_ADDR_PAR_DESCR_EV_AUTOMAZIONE_TEMPO                   0xF877
#define		SPI_FLASH_ADDR_PAR_PROGR_IN1_CC					0xFA07
#define		SPI_FLASH_ADDR_PAR_PROGR_IN1_CA					0xFA09
#define		SPI_FLASH_ADDR_PAR_PROGR_IN2_CC					0xFA0B
#define		SPI_FLASH_ADDR_PAR_PROGR_IN2_CA					0xFA0D
#define		SPI_FLASH_ADDR_PAR_PROGR_IN3_CC					0xFA0F
#define		SPI_FLASH_ADDR_PAR_PROGR_IN3_CA					0xFA11
#define		SPI_FLASH_ADDR_STATO_OUTPUT_EXT					0xFA13
#define		SPI_FLASH_ADDR_FREQ_TEST_MODEM_REG				0xFA14
#define		SPI_FLASH_ADDR_PAR_FREE_FA18                                    0xFA18
#define		SPI_FLASH_ADDR_PAR_FREE_FC18                                    0xFC18
#define		SPI_FLASH_ADDR_PAR_STR_LOCALIZATION				0xFE18
#define		SPI_FLASH_ADDR_PAR_CODICE_IMPIANTO				0x11E18
#define		SPI_FLASH_ADDR_PAR_ETICHETTA_CENTRALE                           0x11E19
#define		SPI_FLASH_ADDR_PAR_NODE_DALI_GROUPS_EXT                         0x11E1D
#define		SPI_FLASH_ADDR_PAR_FREE_12D9D                                   0x12D9D
#define		SPI_FLASH_ADDR_PAR_MASK_BIT_SCENE_ENABLED                       0x12E1D
#define		SPI_FLASH_ADDR_PAR_FREE_12E22     				0x12E22
#define		SPI_FLASH_ADDR_PAR_FREE_12E23 					0x12E23
#define		SPI_FLASH_ADDR_PAR_DELAY_POLL_TRA_NODI                          0x12E24
#define		SPI_FLASH_ADDR_PAR_LINGUA_SEL					0x12E28
#define		SPI_FLASH_ADDR_PAR_CSTT_APN					0x12E29
#define		SPI_FLASH_ADDR_PAR_LG_FM_AUTONOMIA_1H_3H                        0x12E69
#define		SPI_FLASH_ADDR_PAR_LG_FM_PARI_DISPARI                           0x12EE5
#define		SPI_FLASH_ADDR_PAR_LG_FM_CENTR_AUTO_DIMMER                      0x12F61
#define		SPI_FLASH_ADDR_PAR_LG_FM_POS_NEG_DIMMER                         0x12FDD
#define		SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ                     0x13059
#define		SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_FUNZ                         0x1305F
#define		SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT                      0x13061
#define		SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_AUT                          0x13067
#define		SPI_FLASH_ADDR_PAR_LG_FM_LAST_DATA_ORA_TEST_FUNZ                0x13069
#define		SPI_FLASH_ADDR_PAR_LG_FM_LAST_DATA_ORA_TEST_AUT                 0x1306F
#define		SPI_FLASH_ADDR_PAR_RADIO_ID_IN_FIND_NODES                       0x13075
#define		SPI_FLASH_ADDR_PAR_WATT_MIN_NODI				0x13077
#define		SPI_FLASH_ADDR_PAR_WATT_MAX_NODI				0x13837
#define		SPI_FLASH_ADDR_PAR_CIPSTART_TCP_UDP_CONN                        0x13FF7
#define		SPI_FLASH_ADDR_PAR_DELAY_OUT_GRUPPI				0x14037
#define		SPI_FLASH_ADDR_PAR_MB_HR_DELAY_ATT_EVENTO                       0x1403B
#define		SPI_FLASH_ADDR_ERASE_WRITE_CVPS_AUT_DISABLED                    0x1403D
#define		SPI_FLASH_ADDR_WRITE_MAPPA_EE_SENS_AUT_DISABLED                 0x140B9
#define		SPI_FLASH_ADDR_PAR_FREE_14135                                   0x14135
#define		SPI_FLASH_ADDR_PAR_MSG_RADIO_EV_AUTOMAZIONE_EXT                 0x1413D
#define		SPI_FLASH_ADDR_PAR_FREE_142CD                                   0x142CD
#define		SPI_FLASH_ADDR_PAR_AUTO_CALIB					0x148CD
#define		SPI_FLASH_ADDR_PAR_CTR_POLL_TOUR				0x14949
#define		SPI_FLASH_ADDR_PAR_POT_MIN_MAX_EMERG_WRITE                      0x1494A
#define		SPI_FLASH_ADDR_PAR_GRUPPI_WRITE					0x149C6
#define		SPI_FLASH_ADDR_PAR_AUTO_CALIB_WRITE				0x14A42
#define		SPI_FLASH_ADDR_PAR_WATT_SENSORE_FOTONICO                        0x14ABE
#define		SPI_FLASH_ADDR_PAR_FREE_1527E                                   0x1527E
#define		SPI_FLASH_ADDR_PAR_NOME_SCENARIO				0x15282
#define		SPI_FLASH_ADDR_PAR_FREE_15502                                   0x15502
#define		SPI_FLASH_ADDR_PAR_FREE_15504                                   0x15504
#define		SPI_FLASH_ADDR_PAR_FREE_15506                                   0x15506
#define		SPI_FLASH_ADDR_PAR_FREE_15508                                   0x15508
#define		SPI_FLASH_ADDR_PAR_FREE_1550A                                   0x1550A
#define		SPI_FLASH_ADDR_PAR_LQ_ALT_FATHER				0x1550B
#define		SPI_FLASH_ADDR_PAR_WATT_IN_EMERGENZA                            0x1550C
#define		SPI_FLASH_ADDR_PAR_PROG_OUT1					0x15CCC
#define		SPI_FLASH_ADDR_PAR_PROG_OUT2					0x15CCD
#define		SPI_FLASH_ADDR_PAR_PROG_OUT3					0x15CCE
#define		SPI_FLASH_ADDR_PAR_CONFIG_CHANGED				0x15CCF
#define		SPI_FLASH_ADDR_PAR_POT_CHANGED					0x15CD1
#define		SPI_FLASH_ADDR_PAR_GRUPPI_CHANGED				0x15CD3
#define		SPI_FLASH_ADDR_PAR_CVPS_CHANGED					0x15CD5
#define		SPI_FLASH_ADDR_PAR_INTERF_CHANGED				0x15CD7
#define		SPI_FLASH_ADDR_PAR_SCENE_CHANGED				0x15CD9
#define		SPI_FLASH_ADDR_PAR_LOGICA_FM_CHANGED                            0x15CDB
#define		SPI_FLASH_ADDR_PAR_CALIB_CHANGED				0x15CDD
#define 	SPI_FLASH_ADDR_PAR_IP_NR_RETRY_SRV_KEEP_ALIVE                   0x15CDF
#define 	SPI_FLASH_ADDR_PAR_IP_TOUT_RX_SRV_KEEP_ALIVE                    0x15CE0
#define 	SPI_FLASH_ADDR_PAR_FREE_15CE4                                   0x15CE4
#define		SPI_FLASH_ADDR_PAR_FREE5					0x15CE8	 	// 32 bytes free...
#define		SPI_FLASH_ADDR_PAR_PASSWORD_ADMIN				0x15D08
#define		SPI_FLASH_ADDR_PAR_PASSWORD_USER				0x15D18
#define		SPI_FLASH_ADDR_PAR_MAX_BYTES_WRITE_FLASH_BALERA3                0x15D28
#define		SPI_FLASH_ADDR_PAR_LOGICA_FM_WRITE				0x15D29

#define		SPI_FLASH_ADDR_FIRST_PAR_FREE					0x15DA5
#define		SPI_FLASH_ADDR_FIRST_BLOCK_FREE					0x16000

//------------------------------------------------------------------------------
//	Address parametri ext del sistema (vedi documentazione W:\WORK\PROGETTI\Supinv\Documenti SW Becar\Parametri SupInv.doc) 				
//------------------------------------------------------------------------------
#define		SPI_FLASH_ADDR_PAR_EXT_CVPS_AUT_SENS                            0x189000
#define		SPI_FLASH_ADDR_PAR_EXT_SENS_PARAMETRI                           0x1A8000
#define		SPI_FLASH_ADDR_PAR_DONT_USE_0					0x1B1B00
#define		SPI_FLASH_ADDR_PAR_EXT_TRASM_DOMOTICO_PAR                       0x1B2000
#define		SPI_FLASH_ADDR_PAR_EXT_DESCR_QUADRO				0x1D1000
#define		SPI_FLASH_ADDR_PAR_EXT_FIRST_BLOCK_FREE                         0x1D3000


//------------------------------------------------------------------------------
//	Formato dei dati nelle celle ==> SPI_FLASH_ADDR_PAR_EXT_TRASM_DOMOTICO_PAR	0x1B2000
//	quando occupato dai dati delle lampade smartdriver che hanno opticom e sensore ir...
//	1st lettura

//	Blocco del pic..
#define	OFFS_SPI_FLASH_SD_SP						0
#define	OFFS_SPI_FLASH_SD_PMAX						2
#define	OFFS_SPI_FLASH_SD_OFFSET_TEMP					4
#define	OFFS_SPI_FLASH_SD_POT_EMERG					6
#define	OFFS_SPI_FLASH_SD_LAMP_BEHAVIOUR				8
#define	OFFS_SPI_FLASH_SD_PERC_AGING					9
#define	OFFS_SPI_FLASH_SD_PMIN						10
#define	OFFS_SPI_FLASH_SD_CODICE_OPT_SENS_SENSIB			12
#define	OFFS_SPI_FLASH_SD_DURATA_ON_SENS				13
#define	OFFS_SPI_FLASH_SD_DUREZZA_DIMMER				14
#define	OFFS_SPI_FLASH_SD_FADING					15
//	Potenza installatore del pic
#define	OFFS_SPI_FLASH_SD_POT_INST					16
//	Primo blocco lettura del modulo radio.. add 0xB8
#define	OFFS_SPI_FLASH_SD_CVPS_SENS					18
#define	OFFS_SPI_FLASH_SD_TEMPO_FILTR_TRASM				20
#define	OFFS_SPI_FLASH_SD_NR_RIP_TRASM					21
#define	OFFS_SPI_FLASH_SD_PERC_INIZ_TRASM				22
#define	OFFS_SPI_FLASH_SD_PRIORITY_POS					24
#define	OFFS_SPI_FLASH_SD_RAMPA_INIZ_TRASM				25
#define	OFFS_SPI_FLASH_SD_BB_CC_INIZ_TRASM				26
#define	OFFS_SPI_FLASH_SD_DURATA_AUT_INIZ_TRASM				27
#define	OFFS_SPI_FLASH_SD_FINE_BB_CC_TRASM				29
#define	OFFS_SPI_FLASH_SD_FINE_PERC_TRASM				30
#define	OFFS_SPI_FLASH_SD_FINE_RAMPA_AUT_TRASM				31
#define	OFFS_SPI_FLASH_SD_STEP2_RAMPA_AUT_TRASM				32
#define	OFFS_SPI_FLASH_SD_STEP2_BB_CC_TRASM				33
#define	OFFS_SPI_FLASH_SD_STEP2_PERC_TRASM				34

//	Primo blocco lettura del modulo radio.. add 0xDA
#define	OFFS_SPI_FLASH_SD_4TH_LETTURA					35
#define	OFFS_SPI_FLASH_SD_STEP2_DURATA_AUT_TRASM			35
#define	OFFS_SPI_FLASH_SD_RETRY_POS_TRAM				37
#define	OFFS_SPI_FLASH_SD_FREE_1					38
#define	OFFS_SPI_FLASH_SD_WAIT_FOR_ALARM				39
#define	OFFS_SPI_FLASH_SD_FILT_ALARM_DURING_RAMPA			40
#define	OFFS_SPI_FLASH_SD_FILT_RETE_ALIM_230				41
#define	OFFS_SPI_FLASH_SD_CRITICAL_CIRCUIT				42
#define	OFFS_SPI_FLASH_SD_CVPS_OPTICOM					43
#define	OFFS_SPI_FLASH_SD_FREE_2					45
#define	OFFS_SPI_FLASH_SD_FREE_3					46
#define	OFFS_SPI_FLASH_SD_FREE_4					47
#define	OFFS_SPI_FLASH_SD_TEMPO_POLL_OPTICOM				48
#define	OFFS_SPI_FLASH_SD_FLAGS_EN_OPTICOM				49
#define	OFFS_SPI_FLASH_SD_FLAGS2_EN_OPTICOM				50


//	Patche per pianeta sole (il settore all'indirizzo 2000H che risulta bruciato viene rimappato a questo indirizzo 1F000)
#define		SPI_FLASH_ADDR_PAR_PSOLE_PATCH_SECT_2000		0x1F000
//	Lunghezza in spi flash riservata all' APN network (default AT+CSTT="m2m.vodafone.it")
#define		SPI_FLASH_PAR_CSTT_APN_LEN				64
//	Lunghezza in spi flash riservata alla tcp-upd connection modalita' client  (defaultdel server Becar  AT+CIPSTART="TCP","sdserver.beghelli.it","1225")
#define		SPI_FLASH_PAR_CIPSTART_TCP_UDP_CONN_LEN			64

#define		ROUTER_SD_SERVER_ADDR_DEFAULT                           "sdserver.beghelli.it"
#define		ROUTER_SD_SERVER_PORT_DEFAULT                           1225

//	Lunghezza dei parametri di configurazione del singolo nodo (SPI_FLASH_ADDR_PAR_CONFIG_NODES)
//	Parametri di configurazione di ogni singolo nodo effettivamente usati
#define		SIZEOF_PAR_CONFIG_NODES					32
#define		LEN_USED_PAR_CONFIG_NODES				27
//	Attenzione mantenere compatibilita tra questa struttura e la struttura ram in Radio.h (vedi Radio.h ...TConfig)
//	Offset dei campi di configurazione dei nodi
#define		OFFSET_PAR_CONFIG_NODES_ADDR_NODE			0
#define		OFFSET_PAR_CONFIG_NODES_FATHER				4
#define		OFFSET_PAR_CONFIG_NODES_NODE_TYPE			6
#define		OFFSET_PAR_CONFIG_NODES_TIPO_LAMP      			7
#define		OFFSET_PAR_CONFIG_NODES_TIPO_HW				8
#define		OFFSET_PAR_CONFIG_NODES_TIPO_BATT			9
#define		OFFSET_PAR_CONFIG_NODES_SW_VER_MOD_RAD			10
#define		OFFSET_PAR_CONFIG_NODES_SW_VER_PROXY			12
#define		OFFSET_PAR_CONFIG_NODES_LOCALIZZAZIONE			22
#define		OFFSET_PAR_CONFIG_NODES_RIGA_LOC			23
#define		OFFSET_PAR_CONFIG_NODES_COLONNA_LOC			24
#define		OFFSET_PAR_CONFIG_NODES_QUADRO_ELETTRICO		25
#define		OFFSET_PAR_CONFIG_NODES_TIPO_LAMP_EXT			26
#define		OFFSET_PAR_CONFIG_NODES_NON_USABILE_FACILE_0            27
#define		OFFSET_PAR_CONFIG_NODES_NON_USABILE_FACILE_1            28
#define		OFFSET_PAR_CONFIG_NODES_NON_USABILE_FACILE_2            29
#define		OFFSET_PAR_CONFIG_NODES_NON_USABILE_FACILE_3            30
#define		OFFSET_PAR_CONFIG_NODES_NON_USABILE_FACILE_4            31



//------------------------------------------------------------------------------
//	Programmazione degli eventi/automazioni automazioni
//------------------------------------------------------------------------------
//	Lunghezza del messaggio radio di automazione memorizzabile in spi flash
#define		NRO_TOT_EVENTI_TEMPO					40

#define		SPI_PAR_LEN_MSG_RADIO_AUTOMAZIONE			52
#define		SPI_PAR_LEN_MSG_RADIO_AUTOMAZIONE_EXT			10
#define		SPI_PAR_LEN_DESCR_EV_AUTOMAZIONE_TEMPO			10

#define		SPI_PAR_LEN_PROGR_INPUTS				2

#define		EVENTO_AUTOMAZIONE_DISABLED				0xff

//------------------------------------------------------------------------------
//	Costanti di programmazione degli eventi a tempo delle uscite di cui deve essere fatto il set clear
#define		EVENT_TIME_MSG_PROP_SYNCH				0x00
#define		EVENT_TIME_MSG_PROP_SYNCH_AND_OUT_MIN			0x01
#define		EVENT_TIME_MSG_PROP_SYNCH_AND_OUT_MAX			0x40
#define		EVENT_TIME_MIN_CONFIG_OUT				0x80
#define		EVENT_TIME_MAX_CONFIG_OUT				0xbf

//------------------------------------------------------------------------------
//	Offset della descizione degli eventi a tempo
//------------------------------------------------------------------------------
#define		OFFSET_EVENTI_TEMPO_DAYWEEK				0
#define		OFFSET_EVENTI_TEMPO_HOUR     				1
#define		OFFSET_EVENTI_TEMPO_MIN     				2
#define		OFFSET_EVENTI_TEMPO_DAY_EXACT     			3
#define		OFFSET_EVENTI_TEMPO_MONTH_EXACT     			4
#define		OFFSET_EVENTI_TEMPO_DAY_FROM		    		5
#define		OFFSET_EVENTI_TEMPO_MONTH_FROM		    		6
#define		OFFSET_EVENTI_TEMPO_DAY_TO		    		7
#define		OFFSET_EVENTI_TEMPO_MONTH_TO		    		8
#define		OFFSET_EVENTI_TEMPO_PROG_OUT_EXT	    		9

//	Evento di automazione legato all'alba e al tramonto... del campo HOUR 
//	Nel campo MIN  .. c'e' quanti minuti prima o dopo alba/tramonto attivo l'evento (valore +127min/-128 min.)
#define		MASK_BIT_EV_ALBA		     			0x80
#define		MASK_BIT_EV_TRAMONTO	     				0x40



//	-----------------------------------------
//	Costanti di programmazione degli ingressi
//	-----------------------------------------
#define 	INPUT_PROG_EVENTO_AUTOMAZIONE				0
#define 	INPUT_PROG_MIN_OUT_E_EVENTI				0x01
#define 	INPUT_PROG_MAX_OUT_E_EVENTI				0x40
#define 	INPUT_PROG_CALENDARIO_ENABLED				0x41
#define 	INPUT_PROG_CALENDARIO_DISABLED				0x42
#define 	INPUT_PROG_AND_EV_SPECIAL_DAY_ONLY			0x43
#define 	INPUT_PROG_AND_EV_SPECIAL_NIGHT_ONLY			0x44
#define 	INPUT_PROG_AND_EV_SPECIAL_TIME_INTERVAL_ONLY            0x45
#define 	INPUT_PROG_AND_EV_SPECIAL_IN1_CC_ONLY			0x46
#define 	INPUT_PROG_AND_EV_SPECIAL_IN1_CA_ONLY			0x47
#define 	INPUT_PROG_AND_EV_SPECIAL_IN2_CC_ONLY			0x48
#define 	INPUT_PROG_AND_EV_SPECIAL_IN2_CA_ONLY			0x49
#define 	INPUT_PROG_AND_EV_SPECIAL_IN3_CC_ONLY			0x4A
#define 	INPUT_PROG_AND_EV_SPECIAL_IN3_CA_ONLY			0x4B

#define		INPUT_PROG_MIN_CONFIG_OUT				0x80
#define		INPUT_PROG_MAX_CONFIG_OUT				0xbf
#define 	INPUT_PROG_FREE_n					0xfe
#define 	INPUT_NO_PROG						0xff



//	-----------------------------------------
//	Localizzazione
//	-----------------------------------------
// Lunghezza massima stringa di localizzazione	
#define 	MAX_LEN_STR_LOCALIZATION					32
// Numero massimo di localizzazioni	
#define 	MAX_NR_LOCALIZATION						256


//	-----------------------------------------
//	Numero massimo di telefoni abilitati alla comunicazione ip/gprs
//	-----------------------------------------
#define 	NUM_MAX_TEL_IP_GPRS_ENABLED					8


//------------------------------------------------------------------------------
//	Lingue selezionabili
//------------------------------------------------------------------------------
#define		LINGUA_ITALIANO							0
#define		LINGUA_INGLESE   						1
#define		LINGUA_TEDESCO							2

#define		NRO_MAX_LINGUE							3


//	Numero max di parametri dei sensori di automazione previsto in spi flash
#define		NRO_MAX_PAR_SENS_AUT						40
#define		NRO_MAX_PAR_SENS_AUT_USED					36

//	Numero max di parametri dei trasmettitori domotici previsti in spi flash
#define		NRO_MAX_PAR_TRASM_DOMOTICO					128
//	Lunghezza descrizione del quadro di appartenenza dei nodi
#define		LEN_DESCR_QUADRO_NODO						5



//	-----------------------------------------------
//	Dettaglio maschera degli errori delle logica FM (Byte di Error)
//	-----------------------------------------------
#define		MASK_ERR_LOGICA_FM_BATT_RIC					0x00000001
#define		MASK_ERR_LOGICA_FM_BATT_FUNZ					0x00000002
#define		MASK_ERR_LOGICA_FM_BATT_AUT					0x00000004
#define		MASK_ERR_LOGICA_FM_BATT_RIC_INCOMPLETA                          0x00000008
#define		MASK_ERR_LOGICA_FM_TUBO						0x00000010
#define		MASK_ERR_LOGICA_FM_HW						0x00000020
#define		MASK_ERR_LOGICA_FM_ERR_SA					0x00000040
#define		MASK_ERR_LOGICA_FM_LAMPS_ERR					0x0000007F

#define		MASK_ERR_LOGICA_FM_COMM_MOD_RAD_LAMP                            0x40000000
#define		MASK_ERR_LOGICA_FM_COMMUNICATION_24H                            0x80000000

//	--------------------------------------------------
//	Dettaglio maschera degli errori delle lampade UMDL (Stato ballast)
//	--------------------------------------------------
#define		BIT_ERR_UMDL_COMMUNICATION_24H					0
#define		BIT_ERR_UMDL_COMM_MOD_RAD_LAMP					1

#define		BIT_ERR_UMDL_VADC_LOW						16
#define		BIT_ERR_UMDL_VADC_HIGH						17
#define		BIT_ERR_UMDL_V_STD_BY						18
#define		BIT_ERR_UMDL_CHANGE_DIMMER					24
#define		BIT_ERR_UMDL_ACCESO       					25
#define		BIT_ERR_UMDL_START_UP     					26
#define		BIT_ERR_UMDL_SPENTO       					27
#define		BIT_ERR_UMDL_UNDER_VOLTAGE					28
#define		BIT_ERR_UMDL_OVER_VOLTAGE					29
#define		BIT_ERR_UMDL_UNDER_CURRENT					30
#define		BIT_ERR_UMDL_OVER_CURRENT					31


//------------------------------------------------------------------------------
//	Password admin/user
//------------------------------------------------------------------------------
#define		NO_PASSWORD							0x00
#define		PASSWORD_ERROR							0x01
#define		PASSWORD_ADMIN							0x02
#define		PASSWORD_USER							0x03
#define		PASSWORD_OK_MA_ALTRO_UTENTE_GIA_COLLEGATO                       0x04
#define		USER_NAME_ERROR							0x05

#define		PASSWORD_BACK_DOOR						"UMDLTOOLS"

#define		MAX_LEN_PASSWORD						16
#define		MAX_LEN_USERNAME						32
#define		NRO_MAX_USERS							32

#define		MAX_LEN_NOME_SCENARIO						16

//------------------------------------------------------------------------------
//	User name
//------------------------------------------------------------------------------
#define		USERNAME_ERROR							0x01
#define		USERNAME_OK_INSERTED						0x02
#define		USERNAME_OK_MODIFIED						0x03
#define		USERNAME_OK_DELETED 						0x04

//------------------------------------------------------------------------------
//	Progrmmazione delle uscite di allarme OUT1, OU2, OUT3
//------------------------------------------------------------------------------
#define		PROG_OUT_NONE							0xFF
#define		PROG_OUT_ERR_COMM_LOGICA_FM					0x01
#define		PROG_OUT_ERR_COMM_UMDL      					0x02
#define		PROG_OUT_ERR_COMM_LOGICA_FM_OR_UMDL				0x03
#define		PROG_OUT_ERR_LAMP_LOGICA_FM					0x04
#define		PROG_OUT_ERR_LAMP_UMDL						0x05
#define		PROG_OUT_ERR_LAMP_LOGICA_FM_OR_UMDL				0x06
#define		PROG_OUT_ERR_COMM_LAMP_LOGICA_FM_OR_UMDL                        0x07

//	Definisce la logica.. se bit7=1.... quando attivo metto a zero
//      0 = normalmente aperto
//      1 = normalmente chiuso
#define		MASK_AND_LOGIC_PROG_OUT						0x80


//------------------------------------------------------------------------------
//	Parametri del sistema di default 				
//------------------------------------------------------------------------------
#define		SPI_PAR_PALETTO_HIGH						0xe7
#define		SPI_PAR_PALETTO_LOW						0xb5
#define		SPI_PAR_NR_RING_AUTO_ANSWER_DFLT				1
#define		SPI_PAR_ADDR_BUS_SUPINV_DFTL					0x00
#define		SPI_PAR_RADIO_ID_DEFAULT					0x9b4c
#define		SPI_PAR_FLAGS_DEFAULT						0x00
#define		SPI_PAR_NTEL_SMS_DEFAULT					0x00
#define		SPI_PAR_DIS_CAUSE_ALLARME_DEFAULT				0x00
#define		SPI_PAR_PERC_POWER_DISEGUALE_DEFAULT                            0x41
#define		SPI_PAR_DAYS_CONSEC_WH_ZERO_DEFAULT				0x02
//	Autoesclusione di default = 1 giorno
#define		SPI_PAR_TOUT_AUTOESCLUSIONE_DEFAULT				(86400000L)
#define		SPI_PAR_TOUT_MEM_LOG_DIG_DAY					1800000
#define		SPI_PAR_TOUT_MEM_LOG_DIG_NIGHT					7200000
#define		SPI_PAR_COEFF_DAY_POT_PICCO_DEFAULT				8
#define		SPI_PAR_COEFF_MONTH_POT_PICCO_DEFAULT                           27
#define		SPI_PAR_COEFF_YEAR_POT_PICCO_DEFAULT                            12
#define		SPI_PAR_WATCHDOG_TIMER_DEFAULT					10000
#define		SPI_PAR_ORARIO_SNAPSHOT_DEFAULT					(0x163b00L)
#define		SPI_PAR_FREQ_MEM_SNAPSHOT_DEFAULT				0x00015180
#define		SPI_PAR_ERR_COMM_12H_DEFAULT					(3600L*12)
#define		SPI_PAR_ERR_COMM_24H_DEFAULT					(3600L*24)
//	default float (14.5) =   0x41680000
#define		SPI_PAR_K_TAR_VRETE_SUPINV_DEFAULT				0x41680000
#define		SPI_PAR_POLL_FAST_DIGS_DEFAULT					0x00001388
#define		SPI_PAR_POLL_SLOW_DIGS_DEFAULT					0x0036EE80
//	Fai molta attenzione.. questa costante dipende dal valore della costante SPI_FLASH_OPC_LOG_MODEM_TX_AT e SPI_FLASH_OPC_LOG_MODEM_RX_AT
//	Corrispondono ai bit 0 e 1 del quinto bytes di  SPI_FLASH_ADDR_PAR_MAP_BIT_LOG_DISABLED visto che
//	SPI_FLASH_OPC_LOG_MODEM_TX_AT definito = 0x26 e SPI_FLASH_OPC_LOG_MODEM_RX_AT definito = 0x27
#define		SPI_PAR_LOG_MODEM_TX_RX_AT_DISABLED_DEFAULT                     0x03
#define		SPI_PAR_LOG_SCENARI_DISABLED_DEFAULT                            0x08

//	Percentuale aumento potenza minima delle lampade umdl dimmerabili durante la procedura di precalibrazione (40%)
#define		SPI_PAR_PERC_ADD_POT_MIN_PRECALIB_DEFAULT                       0x28
//	Percentuale diminuzione del set point delle lampade umdl dimmerabili durante la procedura di precalibrazione fatta di giorno (20%)
#define		SPI_PAR_PERC_SUB_SP_PRECALIB_DEFAULT                            0x14
//	Percentuale di aumento del set point (rispetto LUM letta) all'atto della calibrazione notturna (quando inizialmente SP = 1279)
#define		SPI_PAR_PERC_ADD_SP_CALIB_DEFAULT				0x05
//	Calibrazione notturna: numero di bit di incr/decr del set point 	
#define		SPI_PAR_UMDL_TAR_INC_DEC_SP_DEFAULT				0x01
//	Calibrazione notturna:  percentuale dei Watt modifica del set point (95%) 	
#define		SPI_PAR_UMDL_TAR_PERC_WATT_MOD_SP_DEFAULT                       0x5f
//	Calibrazione notturna:  percentuale della Lum modifica del set point (90%) 	
#define		SPI_PAR_UMDL_TAR_PERC_LUM_MOD_SP_DEFAULT                        0x5a
//	Frequenza di controllo.. test del campo del gsm (default = 30 secondi) 1 bit = 1 msec.
#define		SPI_PAR_FREQ_TEST_MODEM_REG					30000
//	Address di default del modbus (0x01)
#define		SPI_PAR_MODBUS_ADDR_DEFAULT					0x01
//	Delay del poll tra nodi  (default 300 msec)
#define		SPI_PAR_DELAY_POLL_TRA_NODI_DEFAULT				300				
//	Lingua selezionata di default
#define		SPI_PAR_LINGUA_SEL_DEFAULT					LINGUA_ITALIANO
//	Delay tra un comando di automazione ricevuto via modbus (write coils) e il successivo.. (1 bit = 1 msec).. max 65535 msec
#define		SPI_PAR_MB_HR_DELAY_ATT_EVENTO_DEFAULT                          3000
//	Frequenza di default test autonomia / funzionale
#define		SPI_PAR_LG_FM_FREQ_TEST_FUNZ_DEFAULT                            28
#define		SPI_PAR_LG_FM_FREQ_TEST_AUT_DEFAULT				175
//	Frequenza minima del test di autonomia / funzionale
#define		SPI_PAR_LG_FM_FREQ_MIN_TEST_FUNZ				182
#define		SPI_PAR_LG_FM_FREQ_MIN_TEST_AUT					(365*2)
//	N.ro di giri di poll dopo il quale si fa un giro con tutte le letture complete
#define		SPI_PAR_CTR_POLL_TOUR_DEFAULT					1
//	Delay tra un giro di poll e il successivo (default 1 sec)
#define		SPI_PAR_DELAY_START_POLL_DEFAULT				1000
//  Lunghezza della password del cenlog
#define		SPI_PAR_LEN_PASSWORD_CENLOG                                     6
//  Default della porta Eth0
#define		SPI_PAR_ETH0_PORT_DEFAULT					1001
//  Default della porta Wifi
#define		SPI_PAR_WIFI_PORT_DEFAULT					1002


//	Costanti di configurazione dell'impianto che segnalano cambiamenti in un qualche tipo di configurazione
#define		K_PAR_CONFIG_CHANGED						0
#define		K_PAR_POT_CHANGED						1
#define		K_PAR_GRUPPI_CHANGED						2
#define		K_PAR_CVPS_CHANGED						3
#define		K_PAR_INTERF_CHANGED						4
#define		K_PAR_SCENE_CHANGED						5
#define		K_PAR_LOGICA_FM_CHANGED						6
#define		K_PAR_CALIB_CHANGED						7

#define		TOT_PAR_CONFIG_CHANGED						8


//------------------------------------------------------------------------------
//	Password admin/user
//------------------------------------------------------------------------------
#define		NO_PASSWORD                                                     0x00
#define		PASSWORD_ERROR							0x01
#define		PASSWORD_ADMIN							0x02
#define		PASSWORD_USER							0x03
#define		PASSWORD_OK_MA_ALTRO_UTENTE_GIA_COLLEGATO                       0x04
#define		USER_NAME_ERROR							0x05

#define		PASSWORD_BACK_DOOR                                              "UMDLTOOLS"



//------------------------------------------------------------------------------
//		Mappa a bit dei flags della centrale SUPINV (64 bit disponibili)
//------------------------------------------------------------------------------
#define		SPI_FLAGS_MANCANZA_RETE						0
#define		SPI_FLAGS_CALIBRAZIONE_NOTTURNA_ENABLED				1
#define		SPI_FLAGS_POLL_RADIO_DISABLED					2
#define		SPI_FLAGS_DHCP_ETHO_DISABLED                                    3
#define		SPI_FLAGS_SMS_ALARMS_ENABLED					4
#define		SPI_FLAGS_LOG_RADIO_DISABLED					5
#define		SPI_FLAGS_VIS_ENERGY_FROM_DOWN					6
#define		SPI_FLAGS_AUTOMAZIONE_EVENTI_ENABLED				7
#define		SPI_FLAGS_ATTIVITA_PENDING_DISABLED				8
#define		SPI_FLAGS_RIBADIRE_EVENTI_ENABLED				9
#define		SPI_FLAGS_TEST_AUT_DIFFERITO					10
#define		SPI_FLAGS_RETRY_RADIO_MSG_HIGH 					11
#define		SPI_FLAGS_NO_CHECK_STATE_IO_EXT_AT_RESET			12
#define		SPI_FLAGS_SEARCH_ORPHAN_DISABLED				13
#define		SPI_FLAGS_SUPINV_ALWAYS_CONNECTED_IN_TCP_UDP_CLIENT             14
#define		SPI_FLAGS_MORE_ATTEMPTS_NODES_NOT_COMM				15
#define		SPI_FLAGS_FREE_16                               		16
#define		SPI_FLAGS_ACTIVATE_LAST_SCENE_AT_RESET				17
#define		SPI_FLAGS_VIS_TACCHE_GSM					18
#define		SPI_FLAGS_SIGN_COMM_ERR_DISABLED				19
#define		SPI_FLAGS_SCENARI_DISPLAY_DISABLED				20
#define		SPI_FLAGS_SEARCH_ALT_FATHER_DISABLED				21
#define		SPI_FLAGS_USE_ALT_FATHER_DISABLED				22
#define		SPI_FLAGS_FREE_23                                               23
#define		SPI_FLAGS_PASSWORD_CENLOG_ENABLED                               24
#define		SPI_FLAGS_ETHO_CLIENT_INTERNET                                  25
#define		SPI_FLAGS_WIFI_ENABLED                                          26
#define		SPI_FLAGS_WIFI_STA_MODE                                         27
#define		SPI_FLAGS_WIFI_SERVER_BEGHELLI_ENABLED                          28
#define		SPI_FLAGS_WIFI_DHCP_AP_DISABLED                                 29
#define		SPI_FLAGS_WIFI_DHCP_STA_DISABLED                                30
#define		SPI_FLAGS_FREE_31						31
#define		SPI_FLAGS_FREE_32						32
#define		SPI_FLAGS_FREE_33						33
#define		SPI_FLAGS_FREE_34						34
#define		SPI_FLAGS_FREE_35						35
#define		SPI_FLAGS_FREE_36						36
#define		SPI_FLAGS_FREE_37						37
#define		SPI_FLAGS_FREE_38						38
#define		SPI_FLAGS_FREE_39						39
#define		SPI_FLAGS_FREE_40						40
#define		SPI_FLAGS_FREE_41						41
#define		SPI_FLAGS_FREE_42						42
#define		SPI_FLAGS_FREE_43						43
#define		SPI_FLAGS_FREE_44						44
#define		SPI_FLAGS_FREE_45						45
#define		SPI_FLAGS_FREE_46						46
#define		SPI_FLAGS_FREE_47						47
#define		SPI_FLAGS_FREE_48						48
#define		SPI_FLAGS_FREE_49						49
#define		SPI_FLAGS_FREE_50						50
#define		SPI_FLAGS_FREE_51						51
#define		SPI_FLAGS_FREE_52						52
#define		SPI_FLAGS_FREE_53						53
#define		SPI_FLAGS_FREE_54						54
#define		SPI_FLAGS_FREE_55						55
#define		SPI_FLAGS_FREE_56						56
#define		SPI_FLAGS_FREE_57						57
#define		SPI_FLAGS_FREE_58						58
#define		SPI_FLAGS_FREE_59						59
#define		SPI_FLAGS_FREE_60						60
#define		SPI_FLAGS_FREE_61						61
#define		SPI_FLAGS_FREE_62						62
#define		SPI_FLAGS_FREE_63						63



//	Funzioni
extern void InitAllFilesSpiFlash (void);
extern int ReadAllFilesSpiFlash (void);
extern int CreateAllFilesSpiFlash (void);

extern int SpiFlashWrite (u32 Addr, u8 *p, u32 NroBytes);
extern u32 SpiFlashRead (u32 Addr, u8 *p, u32 NroBytes);
extern u32 SpiFlashErase (u32 Addr, u8 OpcErase);
extern u32 SpiFlashWriteSameValue (u32 Addr, u8 *p, u32 NroBytes);
extern boolean 	SpiFlashInitParam (void);
extern u32 SpiFlashReadParam (u32 AddrParam, u8 *p, u32 NroBytes);
extern int SpiFlashWriteParam (u32 AddrParam, u8 *p, u32 NroBytes, boolean bSameValue);
extern void 	ReloadSpiFlashInitParam (void);
extern boolean	GetSPIFlags (short	NroFlag);
extern boolean	SetSPIFlags (short	NroFlag, boolean bValue);
extern boolean	SetNodeReWriteGruppi (short NroNode, boolean bValue);
extern boolean	SetNodeReWriteProgLogicaFM (short NroNode, boolean bValue);
extern boolean	SetNodeEraseWriteCVPS (short NroNode, boolean bValue);
extern boolean	GetNodeEraseWriteCVPS (short NroNode);
extern boolean	GetNodeWriteMappaEESensAut (short NroNode);
extern boolean	SetNodeWriteMappaEESensAut (short NroNode, boolean bValue);
extern boolean SpiFlashWriteLog (u8 TipoLog, u32 *pAddrLog, u32 *pAddrLogOldest, u8 *p, u16 NroBytes);
extern void SpiFlashWriteLogBook (u8 Opc, u8 *p, u16 NroBytes);
extern void MemLogSnapshot(void);
extern void MemLogDatiConsumo(void);
extern void MemLogDatiConsumoInit(void);
extern void MemLogDigs (void);
extern boolean AddPtrLogSpiFlash (u8 TipoLog, u32 *pAddrLog, u16 Quanto);
extern void SubPtrLogSpiFlash (u8 TipoLog, u32 *pAddrLog, u16 Quanto);
extern boolean SearchLogSpiFlash (u8 TipoLog, u32 NSecStart, u32 NSecEnd, u32 *pStartSpiFlash, u32 *pEndSpiFlash);
extern void LoadLogPtrSpiFlash (u8 TipoLog);
extern boolean  UpdateNextDataOraTestFunz(void);
extern boolean  UpdateNextDataOraTestAut(void);
extern void AdjustCVPSForNodes (short NroSerialChannel, u16 uiCvps, u8 *p);
extern void GetPassword (u8 WhichPassw, char *sPassw);
extern boolean VerifyPassword (u8 WhichPassw, char *sPasswToCompare);
extern u8 SetPassword (u8 QualePasswMod, char *sOldPassw, char *sNewPassw);
extern void DecryptPassword (char *sPassw);
extern u32 ReadEtichettaCentrale (void);


//      Variabili
extern u32  TOutRibadireEventi;
extern u32  TOutAlarmCauseProg[];
extern tTabFilesExSpiFlash TabFilesExSpiFlash[];
extern u8 Param[];


#endif /* EEPROM_H */

