#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>   // File Control Definitions           
#include <termios.h> // POSIX Terminal Control Definitions 
#include <unistd.h>  // UNIX Standard Definitions 	   
#include <errno.h>   // ERROR Number Definitions           
#include <time.h>
#include <pthread.h>        
#include <semaphore.h>
#include <sys/stat.h>
#include "def.h"
#include "Parameter.h"
#include "Utility.h"
#include "Timeout.h"
#include "_serial.h"
#include "Ram.h"
#include "com6.h"
#include "Lcd.h"
#include "RadioPoll.h"


tTabFilesExSpiFlash TabFilesExSpiFlash[] = {
    {SPI_FLASH_PARAM_START_ADDR,                SPI_FLASH_PARAM_END_ADDR,               "Param.bin"},
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


//  Parametri del file di configuazione
u8              Param[SPI_FLASH_LEN];
pthread_mutex_t SpiFlashMutex;

//	Cadenza di ribadire eventi di automazione
u32         TOutRibadireEventi;
u32         TOutAlarmCauseProg[MAX_ALARM_TYPES];


//	Offset su TabSpiParam[] inizializzazione parametri dipendente dalla versione
#define	INIT_PARAM_ALL					0

//	Struttura:
//	1° campo = Address spi flash
//	2° campo = N.ro di bytes da inizializzare al valore di default
//	3° campo = Valore di default
const tTabSpiParam TabSpiParam[] = 
{
	SPI_FLASH_ADDR_PAR_VER_SW,				1,								((VER_SW_FM >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_VER_SW+1,				1,								(VER_SW_FM & 0xff),

	SPI_FLASH_ADDR_PAR_NR_RING_AUTO_ANSWER,			1,								SPI_PAR_NR_RING_AUTO_ANSWER_DFLT,
	SPI_FLASH_ADDR_PAR_ADDR_BUS_SUPINV_UART,                MAX_NUM_UART,                                                   SPI_PAR_ADDR_BUS_SUPINV_DFTL,
	SPI_FLASH_ADDR_PAR_RADIO_ID,				1,								((SPI_PAR_RADIO_ID_DEFAULT >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_RADIO_ID+1,				1,								(SPI_PAR_RADIO_ID_DEFAULT & 0xff),	
	SPI_FLASH_ADDR_PAR_FLAGS,				8,								SPI_PAR_FLAGS_DEFAULT,
	SPI_FLASH_ADDR_PAR_NTEL_SMS,				(TOT_TEL_NRS*MAX_LEN_NRO_TEL),                                  SPI_PAR_NTEL_SMS_DEFAULT,
	SPI_FLASH_ADDR_PAR_DIS_CAUSE_ALLARME, 			4,								SPI_PAR_DIS_CAUSE_ALLARME_DEFAULT,
	SPI_FLASH_ADDR_PAR_LOGO_BEGHELLI,			22,								0,
	SPI_FLASH_ADDR_PAR_NOME_IMPIANTO,			16,								0,
	SPI_FLASH_ADDR_PAR_TOT_NODI,				2,								0,
	SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST+0,		1,								((SPI_FLASH_LOG_BOOK_START_ADDR >> 24) & 0xff),
	SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST+1,		1,								((SPI_FLASH_LOG_BOOK_START_ADDR >> 16) & 0xff),
	SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST+2,		1,								((SPI_FLASH_LOG_BOOK_START_ADDR >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST+3,		1,								(SPI_FLASH_LOG_BOOK_START_ADDR & 0xff),
	SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST+0,		1,								((SPI_FLASH_SNAPSHOT_START_ADDR >> 24) & 0xff),	
	SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST+1,		1,								((SPI_FLASH_SNAPSHOT_START_ADDR >> 16) & 0xff),	
	SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST+2,		1,								((SPI_FLASH_SNAPSHOT_START_ADDR >> 8) & 0xff),	
	SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST+3,		1,								(SPI_FLASH_SNAPSHOT_START_ADDR & 0xff),
	SPI_FLASH_ADDR_PAR_FREQ_MEM_SNAPSHOT+0,			1,								((SPI_PAR_FREQ_MEM_SNAPSHOT_DEFAULT >> 24) & 0xff),	
	SPI_FLASH_ADDR_PAR_FREQ_MEM_SNAPSHOT+1,			1,								((SPI_PAR_FREQ_MEM_SNAPSHOT_DEFAULT >> 16) & 0xff),	
	SPI_FLASH_ADDR_PAR_FREQ_MEM_SNAPSHOT+2,			1,								((SPI_PAR_FREQ_MEM_SNAPSHOT_DEFAULT >> 8) & 0xff),	
	SPI_FLASH_ADDR_PAR_FREQ_MEM_SNAPSHOT+3,			1,								(SPI_PAR_FREQ_MEM_SNAPSHOT_DEFAULT & 0xff),
	SPI_FLASH_ADDR_PAR_ERR_COMM_12H+0,			1,								((SPI_PAR_ERR_COMM_12H_DEFAULT >> 24) & 0xff),	
	SPI_FLASH_ADDR_PAR_ERR_COMM_12H+1,			1,								((SPI_PAR_ERR_COMM_12H_DEFAULT >> 16) & 0xff),	
	SPI_FLASH_ADDR_PAR_ERR_COMM_12H+2,			1,								((SPI_PAR_ERR_COMM_12H_DEFAULT >> 8) & 0xff),	
	SPI_FLASH_ADDR_PAR_ERR_COMM_12H+3,			1,								(SPI_PAR_ERR_COMM_12H_DEFAULT & 0xff),
	SPI_FLASH_ADDR_PAR_ERR_COMM_24H+0,			1,								((SPI_PAR_ERR_COMM_24H_DEFAULT >> 24) & 0xff),	
	SPI_FLASH_ADDR_PAR_ERR_COMM_24H+1,			1,								((SPI_PAR_ERR_COMM_24H_DEFAULT >> 16) & 0xff),	
	SPI_FLASH_ADDR_PAR_ERR_COMM_24H+2,			1,								((SPI_PAR_ERR_COMM_24H_DEFAULT >> 8) & 0xff),	
	SPI_FLASH_ADDR_PAR_ERR_COMM_24H+3,			1,								(SPI_PAR_ERR_COMM_24H_DEFAULT & 0xff),
	SPI_FLASH_ADDR_PAR_MAP_BIT_LOG_DISABLED,		32,								0,
	SPI_FLASH_ADDR_PAR_ENERGIA_CONSUMATA,			8,								0,
	SPI_FLASH_ADDR_PAR_POT_SOST_NODI_UMDL,			(MAX_NODE*2),                                                   0,	
	SPI_FLASH_ADDR_PAR_PERC_ADD_POT_MIN_PRECALIB,           1,                                                              SPI_PAR_PERC_ADD_POT_MIN_PRECALIB_DEFAULT,
	SPI_FLASH_ADDR_PAR_PERC_SUB_SP_PRECALIB,		1,								SPI_PAR_PERC_SUB_SP_PRECALIB_DEFAULT,
	SPI_FLASH_ADDR_PAR_UMDL_TAR_INC_DEC_SP,			1,								SPI_PAR_UMDL_TAR_INC_DEC_SP_DEFAULT,
	SPI_FLASH_ADDR_PAR_UMDL_TAR_PERC_WATT_MOD_SP,           1,                                                              SPI_PAR_UMDL_TAR_PERC_WATT_MOD_SP_DEFAULT,
	SPI_FLASH_ADDR_PAR_UMDL_TAR_PERC_LUM_MOD_SP,            1,                                                              SPI_PAR_UMDL_TAR_PERC_LUM_MOD_SP_DEFAULT,
	SPI_FLASH_ADDR_FREQ_TEST_MODEM_REG+0,			1,                                                              ((SPI_PAR_FREQ_TEST_MODEM_REG >> 24) & 0xff),
	SPI_FLASH_ADDR_FREQ_TEST_MODEM_REG+1,			1,								((SPI_PAR_FREQ_TEST_MODEM_REG >> 16) & 0xff),
	SPI_FLASH_ADDR_FREQ_TEST_MODEM_REG+2,			1,								((SPI_PAR_FREQ_TEST_MODEM_REG >> 8) & 0xff),
	SPI_FLASH_ADDR_FREQ_TEST_MODEM_REG+3,			1,								(SPI_PAR_FREQ_TEST_MODEM_REG & 0xff),
	SPI_FLASH_ADDR_PAR_MASK_BIT_SCENE_ENABLED,		5,								0,
	SPI_FLASH_ADDR_PAR_MODBUS_ADDR_UART,			MAX_NUM_UART,                                                   SPI_PAR_MODBUS_ADDR_DEFAULT,
	SPI_FLASH_ADDR_PAR_CSTT_APN,				1,                                                              0,
	//	Seguente inizializza 1H/3H, PARI/DISPARI, CENTRAL DIMMER/AUTO DIMMER, DIMMER POS/DIMMER NEG
	SPI_FLASH_ADDR_PAR_LG_FM_AUTONOMIA_1H_3H,		(MAX_NODE/8) * 4,                                               0,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ,            1,                                                              31,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ+1,          1,                                                              12,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ+2,          1,                                                              99,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ+3,          1,                                                              0,
        SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ+4,          1,                                                              0,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ+5,          1,                                                              0,
	SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_FUNZ+0,		1,								((SPI_PAR_LG_FM_FREQ_TEST_FUNZ_DEFAULT >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_FUNZ+1,		1,								(SPI_PAR_LG_FM_FREQ_TEST_FUNZ_DEFAULT & 0xff),

	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT,		1,								31,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT+1,           1,                                                              12,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT+2,           1,                                                              99,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT+3,           1,                                                              0,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT+4,           1,                                                              0,
	SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT+5,           1,                                                              0,
	SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_AUT+0,		1,								((SPI_PAR_LG_FM_FREQ_TEST_AUT_DEFAULT >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_AUT+1,		1,								(SPI_PAR_LG_FM_FREQ_TEST_AUT_DEFAULT & 0xff),
	SPI_FLASH_ADDR_PAR_ORARIO_SNAPSHOT+0,			1,								((SPI_PAR_ORARIO_SNAPSHOT_DEFAULT >> 16) & 0xff),	
	SPI_FLASH_ADDR_PAR_ORARIO_SNAPSHOT+1,			1,								((SPI_PAR_ORARIO_SNAPSHOT_DEFAULT >> 8) & 0xff),	
	SPI_FLASH_ADDR_PAR_ORARIO_SNAPSHOT+2,			1,								(SPI_PAR_ORARIO_SNAPSHOT_DEFAULT & 0xff),
	SPI_FLASH_ADDR_PAR_RADIO_ID_IN_FIND_NODES+0,		1,								((SPI_PAR_RADIO_ID_DEFAULT >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_RADIO_ID_IN_FIND_NODES+1,            1,                                                              (SPI_PAR_RADIO_ID_DEFAULT & 0xff),
	SPI_FLASH_ADDR_PAR_CIPSTART_TCP_UDP_CONN,		1,								0,	
	SPI_FLASH_ADDR_PAR_MB_HR_DELAY_ATT_EVENTO+0,		1,								((SPI_PAR_MB_HR_DELAY_ATT_EVENTO_DEFAULT >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_MB_HR_DELAY_ATT_EVENTO+1,            1,								(SPI_PAR_MB_HR_DELAY_ATT_EVENTO_DEFAULT & 0xff),		
	SPI_FLASH_ADDR_PAR_DELAY_POLL_TRA_NODI+0,		1,                                                              ((SPI_PAR_DELAY_POLL_TRA_NODI_DEFAULT >> 24) & 0xff),
	SPI_FLASH_ADDR_PAR_DELAY_POLL_TRA_NODI+1,		1,								((SPI_PAR_DELAY_POLL_TRA_NODI_DEFAULT >> 16) & 0xff),
	SPI_FLASH_ADDR_PAR_DELAY_POLL_TRA_NODI+2,		1,								((SPI_PAR_DELAY_POLL_TRA_NODI_DEFAULT >> 8) & 0xff),
	SPI_FLASH_ADDR_PAR_DELAY_POLL_TRA_NODI+3,		1,								(SPI_PAR_DELAY_POLL_TRA_NODI_DEFAULT & 0xff),
        SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG+0,                   1,                                                              '2',
        SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG+1,                   1,                                                              '3',
        SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG+2,                   1,                                                              '2',
        SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG+3,                   1,                                                              '3',
        SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG+4,                   1,                                                              '2',
        SPI_FLASH_ADDR_PAR_PASSWORD_CENLOG+5,                   1,                                                              '3',
        SPI_FLASH_ADDR_PAR_ETH0_PORT_NR,                        1,                                                              ((SPI_PAR_ETH0_PORT_DEFAULT >> 8) & 0xff),
        SPI_FLASH_ADDR_PAR_ETH0_PORT_NR+1,                      1,                                                              (SPI_PAR_ETH0_PORT_DEFAULT & 0xff),
        SPI_FLASH_ADDR_PAR_WIFI_PORT_NR,                        1,                                                              ((SPI_PAR_WIFI_PORT_DEFAULT >> 8) & 0xff),
        SPI_FLASH_ADDR_PAR_WIFI_PORT_NR+1,                      1,                                                              (SPI_PAR_WIFI_PORT_DEFAULT & 0xff),

	//	Ultima scrittura da eseguire il paletto...
	SPI_FLASH_ADDR_PAR_PALETTO,				1,								SPI_PAR_PALETTO_HIGH,
	SPI_FLASH_ADDR_PAR_PALETTO+1,				1,								SPI_PAR_PALETTO_LOW,
};


void InitAllFilesSpiFlash (void) {
    int     res;

    //  Inizializza i mutex delle uart 
    res = pthread_mutex_init(&SpiFlashMutex, NULL);
    if (res != 0) {
       debug_print_string ("InitParamBin() Errore inizializzazione mutex SpiFlashMutex\n");
    }
    //  Crea tutti i files ... se necessario
    CreateAllFilesSpiFlash ();
    //  Lettura in memoria dei files di configurazione
    ReadAllFilesSpiFlash ();
}


//  ReadAllFileParamBin() : Legge l'intero file di configurazione e lo memorizza nell'array dei parametri
//  Output: 1..n ==> Lunghezza del file Param.bin
//          0 ==> Errore in lettura del file (oppure 0 bytes letti dal file)
//          -1 ==> File Param.bin non trovato oppure errore in apertura
//          
int ReadAllFilesSpiFlash (void) {
    FILE*           fSpiFlash;
    int             Res;
    int             i;
    struct stat     stFile;
    int             LenParamBin;
    
    pthread_mutex_lock(&SpiFlashMutex);    
    Res = 0;
    //  Verifico quale file devo andare a scrivere
    for (i=0 ; TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff ; i++) {
        //  Verifica se file esiste
        if (access(TabFilesExSpiFlash[i].pNomeFileBin, F_OK) != -1) {
            //  Il file esiste .. verificane la lunghezza
            stat (TabFilesExSpiFlash[i].pNomeFileBin, &stFile);
            // Se l'apertura del file fallisce allora esco
            fSpiFlash = fopen(TabFilesExSpiFlash[i].pNomeFileBin, "rb");
            if (fSpiFlash == NULL) {
                debug_print_string("ReadAllFilesSpiFlash(): Errore apertura in lettura file %s\n", TabFilesExSpiFlash[i].pNomeFileBin);
                Res = -1;
                //  Ho avuto un errore nell'apertura del file.. Prossimo file..
            }
            else {
                // Posiziono il cursore ad inizio file
                fseek(fSpiFlash, 0, SEEK_SET);
                LenParamBin = fread (&Param[TabFilesExSpiFlash[i].AddrStartSpiFlash], 1, stFile.st_size, fSpiFlash);
                fclose (fSpiFlash);
                if (Res != -1)
                    Res += LenParamBin;
            }
        }
        else
            Res = -1;
    }
    pthread_mutex_unlock(&SpiFlashMutex);    
    return (Res);
}


//  CreateAllFilesSpiFlash() : Crea tutti i files di configurazione della spi flash di lunghezza corretta
//  Output: 1..n ==> Lunghezza di tutti i files creati
//          -1 ==> Errore nella creazione di uno o più files
//          
int CreateAllFilesSpiFlash (void) {
    FILE            *fSpiFlash;
    int             Res;
    int             i;
    int             LenParamBin;
    

    pthread_mutex_lock(&SpiFlashMutex);
    memset (Param, 0xff, sizeof(Param));
    Res = 0;
    //  Verifico quale file devo andare a scrivere
    for (i=0 ; TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff ; i++) {
        //  Verifica se file esiste
        if (access(TabFilesExSpiFlash[i].pNomeFileBin, F_OK) != -1) {
                continue;
        }
        //  Il file non esiste. oppure è stato cancellato
        // Se l'apertura del file fallisce allora esco
        fSpiFlash = fopen(TabFilesExSpiFlash[i].pNomeFileBin, "wb");
        if (fSpiFlash == NULL) {
            debug_print_string("CreateAllFilesSpiFlash(): Errore apertura in scrittura file %s\n", TabFilesExSpiFlash[i].pNomeFileBin);
            Res = -1;
            //  Ho avuto un errore nell'apertura del file.. Prossimo file..
            continue;
        }
        LenParamBin = fwrite (&Param[TabFilesExSpiFlash[i].AddrStartSpiFlash], 1, ((TabFilesExSpiFlash[i].AddrEndSpiFlash - TabFilesExSpiFlash[i].AddrStartSpiFlash) + 1), fSpiFlash);
        fclose (fSpiFlash);
        if (Res != -1)
            Res += LenParamBin;
    }
    pthread_mutex_unlock(&SpiFlashMutex);    
    return (Res);
}


void DeleteFileSpiFlash (char *pNomeFileBin) {
    
    pthread_mutex_lock(&SpiFlashMutex);
    //  Verifica se file esiste
    if (access(pNomeFileBin, F_OK) != -1) {
        remove (pNomeFileBin);
    }
    pthread_mutex_unlock(&SpiFlashMutex);
}


int CreateFileSpiFlash(char *pNomeFileBin) {
    FILE            *fSpiFlash;
    int             Res;
    int             i;

    pthread_mutex_lock(&SpiFlashMutex);
    Res = 0;
    //  Verifico quale file devo andare a creare
    for (i=0 ; TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff ; i++) {
        if (strncmp(pNomeFileBin, TabFilesExSpiFlash[i].pNomeFileBin, strlen (pNomeFileBin)) == 0)
            break;
    }
    if (TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff) {
        //  Verifica se file esiste
        if (access(pNomeFileBin, F_OK) != -1) {
            debug_print_string("CreateFileSpiFlash(): Il file esiste già.. %s strana cosa\n", pNomeFileBin);
            Res = -1;
        }
        else {
            //  Il file non esiste. oppure è stato cancellato
            // Se l'apertura del file fallisce allora esco
            fSpiFlash = fopen(pNomeFileBin, "wb");
            if (fSpiFlash == NULL) {
                debug_print_string("CreateFileSpiFlash(): Errore apertura in scrittura file %s\n", pNomeFileBin);
                Res = -1;
            }
            else {
                memset (&Param[TabFilesExSpiFlash[i].AddrStartSpiFlash], 0xff, (TabFilesExSpiFlash[i].AddrEndSpiFlash - TabFilesExSpiFlash[i].AddrStartSpiFlash) + 1);
                Res = fwrite (&Param[TabFilesExSpiFlash[i].AddrStartSpiFlash], 1, ((TabFilesExSpiFlash[i].AddrEndSpiFlash - TabFilesExSpiFlash[i].AddrStartSpiFlash) + 1), fSpiFlash);
                fclose (fSpiFlash);
            }
        }
    }
    else  {
        debug_print_string("CreateFileSpiFlash(): Mi è stato chiesto di creare un file che non esiste in lista %s\n", pNomeFileBin);
        Res = -1;
    }
    pthread_mutex_unlock(&SpiFlashMutex);    
    return (Res);
}


//	Input: 
//		Addr		<== Indirizzo della flash da scrivere (indirizzo sul file che simula la flash)
//		p 		<== Indirizzo sorgente dei bytes da scrivere
//		NroBytes	<== Numero dei bytes da scrivere
//	Output:
//		-2              <== (-2) Se ho chiesto di scrivere indirizzo impossibile
//		-1              <== (-1) Se errore nella scrittura
//		Numero dei bytes scritti

int SpiFlashWrite (u32 Addr, u8 *p, u32 NroBytes)
{
    FILE        *fSpiFlash;
    int         Res;
    int         i;
    int         j;
    long        lPos;
    int         bytesWritten;

    pthread_mutex_lock(&SpiFlashMutex);
    //  Verifico quale file devo andare a scrivere
//    debug_print_string ("Scrivo addr: %08X\n",Addr);
    for (i=0 ; TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff ; i++) {
        if (Addr >= TabFilesExSpiFlash[i].AddrStartSpiFlash && Addr <= TabFilesExSpiFlash[i].AddrEndSpiFlash)
            break;
    }
    if (TabFilesExSpiFlash[i].AddrStartSpiFlash == 0xffffffff) {
        debug_print_string("SpiFlashWrite(): Errore ho chiesto di scrivere un indirizzo che non va bene. Addr=%08x\n",Addr);
        Res = -2;
    }
    else {
        
        // Se l'apertura del file fallisce allora esco
        fSpiFlash = fopen(TabFilesExSpiFlash[i].pNomeFileBin, "r+b");
        if (fSpiFlash == NULL) {
            debug_print_string("SpiFlashWrite(): Errore apertura in lettura/scrittura file %s\n", TabFilesExSpiFlash[i].pNomeFileBin);
            Res = -1;
        }
        else {
            //  Punta il file nella posizione corretta da scrivere
            lPos = Addr - TabFilesExSpiFlash[i].AddrStartSpiFlash;
            fseek(fSpiFlash, lPos, SEEK_SET);
            bytesWritten = fwrite (p, 1, NroBytes, fSpiFlash);
            
            if (bytesWritten != NroBytes) {
                debug_print_string("SpiFlashWrite(): Errore volevo scrivere %d bytes, ma ne ho scritti %d\n", NroBytes, bytesWritten);
            }
            //  Aggiorna anche la ram
            memcpy (&Param[Addr], p, bytesWritten);
            fclose (fSpiFlash);
            Res = bytesWritten;
        }
    }
    pthread_mutex_unlock(&SpiFlashMutex);    
    return (Res);
}


//	Input: 
//		Addr		<== Indirizzo della flash da scrivere
//		p 			<== Indirizzo sorgente dello stesso byte da scrivere
//		NroBytes	<== Numero dei bytes da scrivere allo stesso valore
u32 SpiFlashWriteSameValue (u32 Addr, u8 *p, u32 NroBytes)
{
    u32		i;
    u32		Result;

    for (i=0 ; i<NroBytes ; i++)
        Result = SpiFlashWrite ((Addr+i), p, 1);
    return (Result);
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
    
    pthread_mutex_lock(&SpiFlashMutex);    
    Res = 0;
    //  Verifico quale file devo andare a leggere
    for (i=0 ; TabFilesExSpiFlash[i].AddrStartSpiFlash != 0xffffffff ; i++) {
        if (Addr >= TabFilesExSpiFlash[i].AddrStartSpiFlash && Addr <= TabFilesExSpiFlash[i].AddrEndSpiFlash) {
            //  Verifica se file esiste
            if (access(TabFilesExSpiFlash[i].pNomeFileBin, F_OK) != -1) {
                // Se l'apertura del file fallisce allora esco
                fSpiFlash = fopen(TabFilesExSpiFlash[i].pNomeFileBin, "rb");
                if (fSpiFlash == NULL) {
                    debug_print_string("ReadAllFilesSpiFlash(): Errore apertura in lettura file %s\n", TabFilesExSpiFlash[i].pNomeFileBin);
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
            else
                Res = -1;
            //  Lettura eseguita.. esci dal loop
            break;
        }
    }
    pthread_mutex_unlock(&SpiFlashMutex);    
    return (Res);

}


u32 SpiFlashErase (u32 Addr, u8 OpcErase)
{
    u8          b;
    u32         NroBytes;
    
    b = 0xff;
    switch (OpcErase) {
        case SPI_OPC_4K_ERASE:
            NroBytes = 4096;
            break;
        case SPI_OPC_32K_ERASE:
            NroBytes = 32768;
            break;
        case SPI_OPC_64K_ERASE:
            NroBytes = 65536;
            break;
        case SPI_OPC_CHIP_ERASE:
            NroBytes = 2097152;
            break;
        default:
        return (0);
    }
    SpiFlashWriteSameValue (Addr, &b, NroBytes);;
    return (NroBytes);

}



//-----------------------------------------------------------------------------
//		Gestione lettura / scrittura parametri
//-----------------------------------------------------------------------------
static void WriteSpiFlashInitParam (u32 iStart)
{
    u32			i;
    u8			b;

    for (i=iStart ; i<sizeof(TabSpiParam)/sizeof (tTabSpiParam) ; i++)
            SpiFlashWriteSameValue (TabSpiParam[i].Addr, (u8 *)(&TabSpiParam[i].Val), TabSpiParam[i].NroBytes);			
    //	Di default disabilita il log dei comandi AT in tx e rx
    b = SPI_PAR_LOG_MODEM_TX_RX_AT_DISABLED_DEFAULT;
    SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_MAP_BIT_LOG_DISABLED+(SPI_FLASH_OPC_LOG_MODEM_TX_AT/8), &b, 1, FALSE);
 
}



boolean SpiFlashInitParam (void)
{
    u8					b[2];
    u16					VerSwSpiFlash;
    u16					VerRom;

    //	Inizializzazione dei parametri della spi flash
    //	----------------------------------------------
    SpiFlashRead (SPI_FLASH_ADDR_PAR_PALETTO, b, 2);
    debug_print_string ("b[0]=%02x, b[1]=%02x\n", b[0], b[1]);
    
    if (b[0] != SPI_PAR_PALETTO_HIGH || b[1] != SPI_PAR_PALETTO_LOW)
    {
        DeleteFileSpiFlash("Param.bin");
        DeleteFileSpiFlash("ParamExt.bin");
        CreateFileSpiFlash("Param.bin");
        CreateFileSpiFlash("ParamExt.bin");
        WriteSpiFlashInitParam (INIT_PARAM_ALL);
    }
    //	Verifica ed esegui inizializzazioni spi flash dipendenti dalla versione sw della centrale SupInv
    //	------------------------------------------------------------------------------------------------
    SpiFlashRead (SPI_FLASH_ADDR_PAR_VER_SW, b, 2);
    debug_print_string("SpiFlashInitParam(): Ver sw %d.%d\n", b[0],b[1]);
    
    //	Prendi solo il byte meno significativo della versione sw
    VerSwSpiFlash = b[1];
    VerRom = GetVerRom();
    if (VerSwSpiFlash < (VerRom & 0xff)) {
        //	Ver. x.23  ... e tutte le versioni successive
//            if (VerSwSpiFlash < 23) {
//                WriteSpiFlashInitParam (INIT_PARAM_VER_23);
//            }
        //	Scrivi la versione caricato in eeprom
        b[0] = (u8) ((VerRom>>8) & 0xff);
        b[1] = (u8) (VerRom & 0xff);
        SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_VER_SW, b, 2, FALSE);
    }
    return (TRUE);
}


u32 SpiFlashReadParam (u32 AddrParam, u8 *p, u32 NroBytes)
{
    u32		Result;

    Result = SpiFlashRead (AddrParam, p, NroBytes);
    return (Result);
}


int SpiFlashWriteParam (u32 AddrParam, u8 *p, u32 NroBytes, boolean bSameValue)
{
    int         i;
    int         Res;

    debug_print_string ("SpiFlashWriteParam() : Addr=%08X - N.Bytes=%d\n", AddrParam, NroBytes);
/*
    debug_print_string ("Scrittura bytes SpiFlashWrite Addr: %08x, Data: ", AddrParam);
    debug_print_array (p, NroBytes);
*/

    if (!bSameValue) {
        Res = SpiFlashWrite (AddrParam, p, NroBytes);
    }
    else {
        for (i=0 ; i<NroBytes ; i++) {
            Res = SpiFlashWrite ((AddrParam+i), p, 1);
        }
    }
    //  Se ho scritto il numero totale dei nodi.. abilita al riordinamento
    if (AddrParam==SPI_FLASH_ADDR_PAR_TOT_NODI)
        //  Abilita al riordinamento dei codici radio
        SortNodeNotValid = TRUE;
    
    return (Res);
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



void ReloadSpiFlashInitParam (void)
{
    u32				i;
    u8                          b[10];
    
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_ADDR_BUS_SUPINV_UART, AddressBusCentraleSupInv, MAX_NUM_UART);
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_CODICE_IMPIANTO, &CodiceImpiantoSupInv, 1);

    //	Load etichetta Supinv
    EtichettaSupInv = ReadEtichettaCentrale ();
/*
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_ETICHETTA_CENTRALE, b, 4);
    EtichettaSupInv = (u32) ( ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3]);
    EtichettaSupInv = 70001;            //##### provvisorio
*/
    
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_RADIO_ID, b, 2);
    RadioIDSupInv = (u16) (((u16)b[0] << 8) | b[1]);
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_RADIO_ID_IN_FIND_NODES, b, 2);
    RadioIDSupInvInFindNodes = (u16) (((u16)b[0] << 8) | b[1]);

    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_CVPS_WORK_WITH, b, 2);
    CVPSSupInv = (u16) (((u16)b[0] << 8) | b[1]);
    
    
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_FLAGS, SupinvFlags, sizeof(SupinvFlags));

    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_ERR_COMM_12H, b, 4);
    NSecErrCom12H = (u32) ( ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3]);
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_ERR_COMM_24H, b, 4);
    NSecErrCom24H = (u32) ( ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3]);

    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_TOT_NODI, b, 2);
    gTotNodi = (u16) (((u16)b[0] << 8) | b[1]);
    gTotNodi = MIN (gTotNodi, MAX_NODE);
    for (i=0 ; i<gTotNodi ; i++) {
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_CONFIG_NODES + (i*SIZEOF_PAR_CONFIG_NODES) + OFFSET_PAR_CONFIG_NODES_ADDR_NODE, (u8 *)&Node[i].Config.Addr, 4);
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_CONFIG_NODES + (i*SIZEOF_PAR_CONFIG_NODES) + OFFSET_PAR_CONFIG_NODES_FATHER, b, 2);
        Node[i].Config.ProgrPadre = (u16) ((u16)(b[0]) << 8) | b[1];
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_CONFIG_NODES + (i*SIZEOF_PAR_CONFIG_NODES) + OFFSET_PAR_CONFIG_NODES_NODE_TYPE, (u8 *)&Node[i].Config.NodeType, LEN_USED_PAR_CONFIG_NODES-6);
        
        if (IsLampTypeUMDL((u16)i)) {
                SpiFlashReadParam (SPI_FLASH_ADDR_PAR_POT_SOST_NODI_UMDL + (i * 2), b, 2);
                Node[i].Stato.Contarisparmio.PotSost = (u16) ((u16)(b[0]) << 8) | (u16)(b[1]);
        }
        if (GetTipoDispositivo((u16)i) == RELAIS_DOMOTICO) {
                //	Nel caso di rele domotico 20108.. la potenza in emergenza contiene la potenza minima che il rele deve consumare.. altrimenti segnalerà Fail.
                SpiFlashReadParam (SPI_FLASH_ADDR_PAR_WATT_IN_EMERGENZA + (i * 2), b, 2);
                Node[i].Stato.Contarisparmio.PotEmerg = (u16) ((u16)(b[0]) << 8) | (u16)(b[1]);
        }
    }
    for (i=0 ; i<TOT_ALARM_TYPES ; i++) {
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_TOUT_FILT_CAUSE_ALLARME+(i*4), b, 4);
        TOutAlarmCauseProg[i] = (u32) ( ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3]);
    }
    LoadTimeout ((short)(TOUT_FILTR_ALRM_SMS+ALARM_TYPE_NODO_ANOMALIA), TOutAlarmCauseProg[ALARM_TYPE_NODO_ANOMALIA]);
    LoadTimeout ((short)(TOUT_FILTR_ALRM_SMS+ALARM_TYPE_NODO_COMM_ERR), TOutAlarmCauseProg[ALARM_TYPE_NODO_COMM_ERR]);


    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_MAP_BIT_LOG_DISABLED, MapBitLogDisabled, 32);
    //	Lettura tipo protocollo attivo sulla n linee seriali virtuali (FD, modbus rtu, modbus tcp-ip)
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_TIPO_PROT_UART, TipoProtUart, MAX_NUM_UART);
    for (i=0 ; i<MAX_NUM_UART ; i++) {
        TipoProtUart[i] = (TipoProtUart[i] < NRO_MAX_TIPO_PROT ? TipoProtUart[i] : TIPO_PROT_CENLOG);
    }
    //	Address del supinv nel protocollo modbus
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_MODBUS_ADDR_UART, ModbusAddr, MAX_NUM_UART);
    //	Lingua selezionata
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LINGUA_SEL, &LinguaSelected, 1);
    //	Se lingua non selezionata, usa quella di default che e' italiano
    LinguaSelected = (LinguaSelected < NRO_MAX_LINGUE ? LinguaSelected : SPI_PAR_LINGUA_SEL_DEFAULT);
    //	Default dell'holding register di modbus che indica delay tra attuazione di eventi di automazione da modbus usando coils
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_MB_HR_DELAY_ATT_EVENTO, b, 2);
    HoldingRegistersModbus[ADD_MB_HR_DELAY_ATT_EVENTO] = (u16) (((u16)b[0] << 8) | b[1]);
    //	Load time out di ribadire scenari (eventi di propagazione)
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_TOUT_RIBADIRE_EVENTI, b, 4);
    TOutRibadireEventi = (u32) ( ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3]);
    if (TOutRibadireEventi == 0xffffffff)
            TOutRibadireEventi = MIN_020;
    //  Lettura del tipo di centrale
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_TIPO_CENTRALE, &TipoCentrale, 1);
    if (TipoCentrale >= NRO_TIPI_CENTRALE)
        TipoCentrale = TIPO_CENTRALE_FM;
}


boolean	SetNodeReWriteProgLogicaFM (short NroNode, boolean bValue) {
    u32			Offset;
    u32			BitOffset;
    u8			b;

    Offset = (u32)(NroNode / 8);
    BitOffset = (u32)(NroNode % 8);

    SpiFlashReadParam ((SPI_FLASH_ADDR_PAR_LOGICA_FM_WRITE+Offset), &b, 1);
    BitOp ((bValue ? SET_BIT : RESET_BIT), &b, (short)BitOffset);
    SpiFlashWriteParam ((SPI_FLASH_ADDR_PAR_LOGICA_FM_WRITE+Offset), &b, 1, FALSE);	
}


boolean	SetNodeReWriteGruppi (short NroNode, boolean bValue) {
    u32			Offset;
    u32			BitOffset;
    u8			b;

    Offset = (u32)(NroNode / 8);
    BitOffset = (u32)(NroNode % 8);

    SpiFlashReadParam ((SPI_FLASH_ADDR_PAR_GRUPPI_WRITE+Offset), &b, 1);
    BitOp ((bValue ? SET_BIT : RESET_BIT), &b, (short)BitOffset);
    SpiFlashWriteParam ((SPI_FLASH_ADDR_PAR_GRUPPI_WRITE+Offset), &b, 1, FALSE);	
}


boolean	GetNodeEraseWriteCVPS (short NroNode) {
u32			Offset;
    u32			BitOffset;
    u8			b;

    Offset = (u32)(NroNode / 8);
    BitOffset = (u32)(NroNode % 8);

    SpiFlashReadParam ((SPI_FLASH_ADDR_ERASE_WRITE_CVPS_AUT_DISABLED+Offset), &b, 1);
    return (BitOp (TEST_BIT, &b, (short)BitOffset));

}


boolean	SetNodeEraseWriteCVPS (short NroNode, boolean bValue) {
    u32			Offset;
    u32			BitOffset;
    u8			b;

    Offset = (u32)(NroNode / 8);
    BitOffset = (u32)(NroNode % 8);

    SpiFlashReadParam ((SPI_FLASH_ADDR_ERASE_WRITE_CVPS_AUT_DISABLED+Offset), &b, 1);
    BitOp ((bValue ? SET_BIT : RESET_BIT), &b, (short)BitOffset);
    SpiFlashWriteParam ((SPI_FLASH_ADDR_ERASE_WRITE_CVPS_AUT_DISABLED+Offset), &b, 1, FALSE);	
}


boolean	GetNodeWriteMappaEESensAut (short NroNode) {
    u32			Offset;
    u32			BitOffset;
    u8			b;

    Offset = (u32)(NroNode / 8);
    BitOffset = (u32)(NroNode % 8);

    SpiFlashReadParam ((SPI_FLASH_ADDR_WRITE_MAPPA_EE_SENS_AUT_DISABLED+Offset), &b, 1);
    return (BitOp (TEST_BIT, &b, (short)BitOffset));
}

boolean	SetNodeWriteMappaEESensAut (short NroNode, boolean bValue) {
    u32			Offset;
    u32			BitOffset;
    u8			b;

    Offset = (u32)(NroNode / 8);
    BitOffset = (u32)(NroNode % 8);

    SpiFlashReadParam ((SPI_FLASH_ADDR_WRITE_MAPPA_EE_SENS_AUT_DISABLED+Offset), &b, 1);
    BitOp ((bValue ? SET_BIT : RESET_BIT), &b, (short)BitOffset);
    SpiFlashWriteParam ((SPI_FLASH_ADDR_WRITE_MAPPA_EE_SENS_AUT_DISABLED+Offset), &b, 1, FALSE);	

}


//	p --> Mappa a bit dei nodi da resincronizzare (NULL = su tutti i nodi)
void AdjustCVPSForNodes (short NroSerialChannel, u16 uiCvps, u8 *p) {
    u8			b[NRO_MAX_CVPS_AUT_SENS_LAMPS*4];
    u16			CvpsSpiFlash;
    u16                 LampIndex;
    u32			i;
    u32			j;
    u32			k;
    u32			TipoDisp;
    u8			NroGiri;		

    //	Interruzione del poll per 40 secondi circa.
    StopTimeoutValue = SEC_040;
    gRadioPoll.boolReqStopTimeout = TRUE;

    for (i=0 ; i<gTotNodi ; i++) {
        NroGiri = 0;
        if (NroSerialChannel >= 0)
            //	Stai ancora connesso per 120 secondi
            LoadTimeout ((short)(TOUT_REMOTE_CONNECTION+NroSerialChannel), SEC_120);					
        TipoDisp = GetTipoDispositivo((u16)(i));
        if ((NULL==p || BitOp (TEST_BIT, p, (short)i)) && 
            (LAMP_LOGICA_FM==TipoDisp || CONTARISPARMIO==TipoDisp || LAMPADE_LED==TipoDisp || LAMPADE_LED_BALERA==TipoDisp  || RELAIS_DOMOTICO==TipoDisp || ZIGBAL==TipoDisp  || AMADORI==TipoDisp || MISPOT==TipoDisp || TRASMETTITORE_DOMOTICO==TipoDisp)) {
            //	Se c'e' un cvps di centrale da scrivere
                if (uiCvps != 0xffff) {
                    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_EXT_CVPS_AUT_SENS+(i*NRO_MAX_CVPS_AUT_SENS_LAMPS*4), b, (NRO_MAX_CVPS_AUT_SENS_LAMPS*4));
                    for (j=0 ; j<NRO_MAX_CVPS_AUT_SENS_LAMPS ; j++) {
                        CvpsSpiFlash = ((u16)(b[j*4]) << 8) | b[(j*4)+1];
                        //	Se trovato un vecchio Cvps di centrale oppure uno spazio vuoto lo occupo con il nuovo
                        if ((CvpsSpiFlash>=CVPS_CU_RESERVED_START && CvpsSpiFlash<=CVPS_CU_RESERVED_END) || CvpsSpiFlash==0xffff) {
                            NroGiri++;
                            //	Il CVPS è uguale sia per le lampade che per i Tx 20104
                            b[j*4] = (u8)((uiCvps >> 8) & 0xff);
                            b[(j*4)+1] = (u8)(uiCvps & 0xff);
                            if (TRASMETTITORE_DOMOTICO==TipoDisp) {
                                if (NroGiri == 1) {
                                    b[(j*4)+2] = (u8)((MAX_NODE >> 8) & 0xff);
                                    b[(j*4)+3] = (u8)(MAX_NODE & 0xff);
                                }
                                else {
                                    b[(j*4)+2] = (u8)(((MAX_NODE | 0x8000) >> 8) & 0xff);
                                    b[(j*4)+3] = (u8)((MAX_NODE | 0x8000) & 0xff);
                                    break;
                                }
                            }
                            else {
                                //	Aggiorna con il nuovo cvps (indice sequenziale..)
                                LampIndex = GetLampIndexForPs((u16)i);
                                b[(j*4)+2] = (u8) ((LampIndex >> 8) & 0xff);
                                b[(j*4)+3] = (u8) (LampIndex & 0xff);
                                break;
                            }
                        }
                    }
                    SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_EXT_CVPS_AUT_SENS+(i*NRO_MAX_CVPS_AUT_SENS_LAMPS*4), b, (NRO_MAX_CVPS_AUT_SENS_LAMPS*4), FALSE);
                }
                else {
                    //	Se invece il cvps di centrale è praticamente da cancellare.. lo cancelli
                    //	Leggi i CVPS di automazione programmati sul nodo tutti in una volta.. e shift a sinistra...
                    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_EXT_CVPS_AUT_SENS+(i*NRO_MAX_CVPS_AUT_SENS_LAMPS*4), b, (NRO_MAX_CVPS_AUT_SENS_LAMPS*4));
                    for (j=0,k=0 ; j<NRO_MAX_CVPS_AUT_SENS_LAMPS ; j++) {
                        CvpsSpiFlash = ((u16)(b[j*4]) << 8) | b[(j*4)+1];
                        if (CvpsSpiFlash>=CVPS_CU_RESERVED_START && CvpsSpiFlash<=CVPS_CU_RESERVED_END)
                            continue;
                        else {
                            memcpy (&b[k*4], &b[j*4], 4);
                            k++;
                        }	
                    }
                    while (k<NRO_MAX_CVPS_AUT_SENS_LAMPS) {
                        memset (&b[k*4], 0xff, 4);
                        k++;
                    }
                    SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_EXT_CVPS_AUT_SENS+(i*NRO_MAX_CVPS_AUT_SENS_LAMPS*4), b, (NRO_MAX_CVPS_AUT_SENS_LAMPS*4), FALSE);
                }
                //	Abilita resync su questo nodo (solo se non è per tutti... altrimento lo faccio alla fine..)	
                if (NULL!=p)	
                        EnableEraseWriteCVPS ((i+1), TRUE);
        }	
    }
    if (NULL==p)	
        EnableEraseWriteCVPS (0, TRUE);
}


//	Leggi la password dall'spi flash
void GetPassword (u8 WhichPassw, char *sPassw) {
    u32		l;
    u8          b[4];

    if (PASSWORD_ADMIN == WhichPassw) {
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_PASSWORD_ADMIN, (u8 *)sPassw, MAX_LEN_PASSWORD);
        //	Se password admin non c'è ... il default è il numero dell'etichetta della centrale da leggere in spi flash
        if (0xff == *(sPassw+0)) {
            SpiFlashReadParam (SPI_FLASH_ADDR_PAR_ETICHETTA_CENTRALE, b, 4);
            l = (u32) ( ((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | b[3]);
            if (0xffffffff == l)
                *(sPassw+0) = 0;
            else
                sprintf(sPassw, "%ld", l);
        }
    }
    else  {
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_PASSWORD_USER, (u8 *)sPassw, MAX_LEN_PASSWORD);
        if (0xff == *(sPassw+0))
            *(sPassw+0) = 0;
    }

}


//	Cript della password : metodo: Ogni byte faccio add 0x0f e xor 0x5A 
//	Decript della password : metodo: Ogni byte faccio xor 0x5A e sub 0x0f
//	Qui faccio il decript: Il byte deve avere valore <=0xf0
//	
void DecryptPassword (char *sPassw) {
    u32			i;
    u8			Dato;

    for (i=0 ; i<strlen(sPassw) ; i++) {
        Dato = *(sPassw+i);
        Dato ^= 0x5a;
        Dato -= 0x0f;
        *(sPassw+i) = Dato;
    }
}


//	Verifica se password corretta
//	Output :
//			TRUE	==> Password corretta OK
//			FALSE	==> Password errata KO
boolean VerifyPassword (u8 WhichPassw, char *sPasswToCompare) {
    char	s[MAX_LEN_PASSWORD];

    //	Se entro con la back door "UMDLTOOLS" allora va sempre bene 
    if (0 == strcmp(sPasswToCompare, PASSWORD_BACK_DOOR))
        return (TRUE);

    //	Leggi la password memorizzata in centrale
    GetPassword (WhichPassw, s);
    if (0x00 == s[0])
        return (FALSE);

    if (0 == strcmp(s, sPasswToCompare))
        return (TRUE);
    else
        return (FALSE);
}


//	Scrivi la password dall'spi flash
//	Input : Quale password modificare
//			2 ==> Password admin
//			3 ==> Password user
//	Output :
//			1	==> Errore password
//			2	==> OK password admin
//			3	==> OK password user
u8 SetPassword (u8 QualePasswMod, char *sOldPassw, char *sNewPassw) {
    u8			TipoPassw;

    if (PASSWORD_ADMIN==QualePasswMod && VerifyPassword (PASSWORD_ADMIN, sOldPassw))
        TipoPassw = PASSWORD_ADMIN;
    else if (PASSWORD_USER==QualePasswMod && (VerifyPassword (PASSWORD_USER, sOldPassw) || VerifyPassword (PASSWORD_ADMIN, sOldPassw))) {
        TipoPassw = PASSWORD_USER;
    }
    else
        TipoPassw = PASSWORD_ERROR;

    if (TipoPassw != PASSWORD_ERROR)
        SpiFlashWriteParam ((PASSWORD_ADMIN==QualePasswMod ? SPI_FLASH_ADDR_PAR_PASSWORD_ADMIN : SPI_FLASH_ADDR_PAR_PASSWORD_USER), (u8 *)sNewPassw, (u32)(strlen(sNewPassw)+1), FALSE);	
    return (TipoPassw);
}

boolean AddPtrLogSpiFlash (u8 TipoLog, u32 *pAddrLog, u16 Quanto)
{
    boolean		bScavallamentoFineBuffer;

    bScavallamentoFineBuffer = FALSE;
    (*pAddrLog) += Quanto;
    switch (TipoLog)
    {
    case LOG_BOOK:
        if (*pAddrLog > SPI_FLASH_LOG_BOOK_END_ADDR) {
                *pAddrLog -= SPI_FLASH_LOG_BOOK_LEN;
                bScavallamentoFineBuffer = TRUE;
        }
        break;
    case LOG_SNAPSHOT:
        if (*pAddrLog > SPI_FLASH_SNAPSHOT_END_ADDR) {
                *pAddrLog -= SPI_FLASH_SNAPSHOT_LEN;
                bScavallamentoFineBuffer = TRUE;
        }
        break;
    }
    return (bScavallamentoFineBuffer);
}

void SubPtrLogSpiFlash (u8 TipoLog, u32 *pAddrLog, u16 Quanto)
{
    (*pAddrLog) -= Quanto;
    switch (TipoLog)
    {
    case LOG_BOOK:
        if (*pAddrLog < SPI_FLASH_LOG_BOOK_START_ADDR)
                *pAddrLog += SPI_FLASH_LOG_BOOK_LEN;
        break;
    case LOG_SNAPSHOT:
        if (*pAddrLog < SPI_FLASH_SNAPSHOT_START_ADDR)
                *pAddrLog += SPI_FLASH_SNAPSHOT_LEN;
        break;
    }
}



//	Scrivi in spi flash dei dati del log (log book, log dig, log snapshot)
//	Output : 	TRUE 	==> Scrittura OK
//				FALSE	==> Errore cancellazione / scrittura flash
boolean SpiFlashWriteLog (u8 TipoLog, u32 *pAddrLog, u32 *pAddrLogOldest, u8 *p, u16 NroBytes)
{
    u16				i;
    boolean			bResult;
    union
    {
            u8			b[2];
            u16			i;
    } LenLogRec;
    u32				AddrSectorLogToErase;
    u8                          b[4];
    u32                         l;

    bResult = TRUE;
    for (i=0 ; i<NroBytes ; i++)
    {
        if (SpiFlashWrite (*pAddrLog, (p+i), 1) < 0)
            bResult = FALSE;

        //	Incrementa il ptr al log in spi flash
        AddPtrLogSpiFlash (TipoLog, pAddrLog, 1);

        //	Se ptr si trova a fine settore -1 cancelli subito il prossimo che andrai a scrivere (e' il prossimo settore che andrai a scrivere)
        //	Tutto questo per lasciare almeno due bytes liberi (0xff,0xff che sono la lunghezza del record)
        AddrSectorLogToErase = 0;
        if (((*pAddrLog) & 0x000ffe) == 0x000ffe) {
            //	Decido di resettare il prossimo settore
            AddrSectorLogToErase = (*pAddrLog); 
            AddPtrLogSpiFlash (TipoLog, &AddrSectorLogToErase, 2);
        }

        //	Se ho deciso di resettare il prossimo settore...
        if (AddrSectorLogToErase) {
            //	Verifica se devo aggiornare il ptr oldest
            if (AddrSectorLogToErase == (*pAddrLogOldest & 0xfff000))
            {
                while (AddrSectorLogToErase == (*pAddrLogOldest & 0xfff000))	{
                    //	Leggi la lunghezza del record memorizzato in spi flash 1 byte alla volta
                    SpiFlashRead (*pAddrLogOldest, &LenLogRec.b[0], 1);
                    AddPtrLogSpiFlash (TipoLog, pAddrLogOldest, 1);		
                    SpiFlashRead (*pAddrLogOldest, &LenLogRec.b[1], 1);
                    AddPtrLogSpiFlash (TipoLog, pAddrLogOldest, (LenLogRec.i+1));		
                }
                //	Memorizza il nuovo ptr al piu vecchio record di log memorizzato in spi flash
                switch (TipoLog)
                {
                case LOG_BOOK:
                    l = (u32) (pAddrLogOldest);
                    b[0] = ((l >> 24) & 0xff);
                    b[1] = ((l >> 16) & 0xff);
                    b[2] = ((l >> 8) & 0xff);
                    b[3] = (l & 0xff);
                    SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST, b, 4, FALSE);
                    break;
                case LOG_SNAPSHOT:
                    l = (u32) (pAddrLogOldest);
                    b[0] = ((l >> 24) & 0xff);
                    b[1] = ((l >> 16) & 0xff);
                    b[2] = ((l >> 8) & 0xff);
                    b[3] = (l & 0xff);
                    SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST, b, 4, FALSE);
                    break;
                }
            }
            //	Cancello finalmente il prossimo settore
            if (SpiFlashErase (AddrSectorLogToErase, SPI_OPC_4K_ERASE) < 0)
                bResult = FALSE;
        }
    }
    return (bResult);
}

//	Puntatore alla spi flash che dovro' andare a scrivere e NroBytes che mi accingero' a scrivere

static void AllineaPtrAFineLogMem (u8 TipoLog, u32 *Ptr, u32 *PtrOldest, u16 NroBytes)
{
    u16				i;
    u16				Nr;
    u8				OpcDummy;
    u8                          b[4];

    switch (TipoLog)
    {
    case LOG_BOOK:
        if ((((*Ptr)+(u32)NroBytes) != (SPI_FLASH_LOG_BOOK_END_ADDR+1)) && ((*Ptr)+(u32)NroBytes) > ((SPI_FLASH_LOG_BOOK_END_ADDR+1)-7)) {
            //	Completa il buffer del log con un evento DUMMY
            Nr = (u16)(((SPI_FLASH_LOG_BOOK_END_ADDR+1) - *Ptr) - 2);
            b[0] = (u8) ((Nr & 0xff00) >> 8);
            b[1] = (u8) (Nr & 0x00ff);
            SpiFlashWriteLog (LOG_BOOK, Ptr, PtrOldest, b, 2);

            b[0] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0xff000000) >> 24);
            b[1] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x00ff0000) >> 16);
            b[2] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x0000ff00) >> 8);
            b[3] = (u8) (DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x000000ff);
            //	Scrivi data/ora di memorizzazione del log dig
            SpiFlashWriteLog (LOG_BOOK, Ptr, PtrOldest, b, 4);
            //	Opcode
            OpcDummy = SPI_FLASH_OPC_LOG_DUMMY;
            SpiFlashWriteLog (LOG_BOOK, Ptr, PtrOldest, &OpcDummy, 1);
            //	Scrivi il buffer dummy riempiendolo di OpcDummy..
            for (i=0 ; i<(Nr-5) ; i++)
                SpiFlashWriteLog (LOG_BOOK, Ptr, PtrOldest, &OpcDummy, 1);
        }
        break;

    case LOG_SNAPSHOT:
        if ((((*Ptr)+(u32)NroBytes) != (SPI_FLASH_SNAPSHOT_END_ADDR+1)) && ((*Ptr)+(u32)NroBytes) > ((SPI_FLASH_SNAPSHOT_END_ADDR+1)-7)) {
            //	Completa il buffer del log con un evento DUMMY
            Nr = (u16)(((SPI_FLASH_SNAPSHOT_END_ADDR+1) - *Ptr) - 2);
            b[0] = (u8) ((Nr & 0xff00) >> 8);
            b[1] = (u8) (Nr & 0x00ff);
            
            SpiFlashWriteLog (LOG_SNAPSHOT, Ptr, PtrOldest, b, 2);
            b[0] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0xff000000) >> 24);
            b[1] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x00ff0000) >> 16);
            b[2] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x0000ff00) >> 8);
            b[3] = (u8) (DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x000000ff);

            //	Scrivi data/ora di memorizzazione del log dig
            SpiFlashWriteLog (LOG_SNAPSHOT, Ptr, PtrOldest, b, 4);
            //	Opcode
            OpcDummy = SPI_FLASH_OPC_LOG_DUMMY;
            SpiFlashWriteLog (LOG_SNAPSHOT, Ptr, PtrOldest, &OpcDummy, 1);
            //	Scrivi il buffer dummy riempiendolo di OpcDummy..
            for (i=0 ; i<(Nr-5) ; i++)
                SpiFlashWriteLog (LOG_SNAPSHOT, Ptr, PtrOldest, &OpcDummy, 1);
        }
        break;
    default:
        break;
    }
}


//	Memorizzazione del log book
void SpiFlashWriteLogBook (u8 Opc, u8 *p, u16 NroBytes)
{
    u32				l;
    u32				l1;
    u16				Nr;
    u8                          b[4];

    //	Il log boook lo memorizzo solo se la data e ora valida
    if (!IsDateTimeValid (&DateTimeSupinv))
            return;

    if (!BitOp (TEST_BIT, MapBitLogDisabled, Opc)) {
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST, b, 4);
        l1 = (u32)((u32)b[0] << 24) | ((u32)b[1] << 16) | ((u32)b[2] << 8) | ((u32)b[0]);
        l = AddrSpiFlashLogBook;
        AllineaPtrAFineLogMem (LOG_BOOK, &l, &l1, 5 + 2 + NroBytes);

        Nr = (u16)(5 + NroBytes); 
        b[0] = (u8) ((Nr & 0xff00) >> 8);
        b[1] = (u8) (Nr & 0x00ff);
        SpiFlashWriteLog (LOG_BOOK, &l, &l1, b, 2);
        //	Scrivi data/ora di memorizzazione del log dig
        b[0] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0xff000000) >> 24);
        b[1] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x00ff0000) >> 16);
        b[2] = (u8) ((DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x0000ff00) >> 8);
        b[3] = (u8) (DateTimeSupinvLegale_NsecFrom_1_1_2000 & 0x000000ff);
        SpiFlashWriteLog (LOG_BOOK, &l, &l1, b, 4);
        //	Opcode
        SpiFlashWriteLog (LOG_BOOK, &l, &l1, &Opc, 1);
        //	Scrivi tutti in dati del buffer (se ce ne sono)
        SpiFlashWriteLog (LOG_BOOK, &l, &l1, p, NroBytes);
        //	Memorizza il nuovo ptr del log book
        AddrSpiFlashLogBook = l;
    }
}


void MemLogDatiConsumoInit(void) {
u16				i;
    u8				b[SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN_USED];

    //	Il primo salvataggio dei dati di consumo lo farà dopo una ora dal reset
    LoadTimeout (TOUT_MEM_DATI_CONSUMO, MIN_060);
    for (i=0 ; i<gTotNodi ; i++) {
        SpiFlashRead ((u32)(SPI_FLASH_DATI_CONSUMO_NODI_START_ADDR+(i*SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN)) , b, SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN_USED);
        //	Se ci sono dei dati in spi flash li preleva dopo un reset
        if (!VerifyBufferFullOf (b, 4, 0xff)) {
            //	Copia ora dell'ultimo poll
            memcpy ((u8 *)(&Node[i].Poll.DatetimeLastPollOkNsec), &b[DATI_SD_CONS_DATA], 4);
            if (IsLampTypeUMDL(i)) 	{
                //	Copia Watt e KWh
                memcpy ((u8 *)(&Node[i].Stato.Contarisparmio.Watt), &b[DATI_SD_CONS_WATT], 10);
                //	Copia tempo ON
                memcpy ((u8 *)(&Node[i].Stato.Contarisparmio.TempoOn), &b[DATI_SD_CONS_TIME_ON], 4);
                //	Copia Pot min e pot max.
                memcpy ((u8 *)(&Node[i].Stato.Contarisparmio.PotMin), &b[DATI_SD_CONS_POT_MIN], 4);
                //	Copia Stato Ballast e StatoOnOffErrLamp
                Node[i].Stato.Contarisparmio.StatoBallast = b[DATI_SD_CONS_STATO_BALLAST];
                Node[i].Stato.Contarisparmio.StatoOnOffErrLampada = b[DATI_SD_CONS_STATO_ON_OFF_ERR_LAMP];
                if (*((u16 *)(&b[DATI_SD_CONS_STATUS])) != 0xffff)
                    Node[i].Poll.Status = *((u16 *)(&b[DATI_SD_CONS_STATUS]));
            }
            else if (GetTipoDispositivo(i) == LAMP_LOGICA_FM) {
                //	Copia Stato, Error e Stato SA
                memcpy ((u8 *)(&Node[i].Stato.LampLogicaFM.StatoFM) ,&b[DATI_FM_STATO], 3);
                Node[i].Stato.LampLogicaFM.Stato2FM = b[DATI_FM_STATO_2];
                Node[i].Stato.LampLogicaFM.StatoOnOffErrLampada = b[DATI_FM_STATO_ON_OFF_ERR_LAMP];
                if (*((u16 *)(&b[DATI_FM_STATUS])) != 0xffff)
                    Node[i].Poll.Status = *((u16 *)(&b[DATI_FM_STATUS]));
            }
            else if (GetTipoDispositivo(i) == TRASMETTITORE_DOMOTICO) {
                if (*((u16 *)(&b[DATI_20104_STATUS])) != 0xffff)
                    Node[i].Poll.Status = *((u16 *)(&b[DATI_20104_STATUS])) ;
            }
        }
    }
}


//	Memorizzazione periodica dello stato delle lampade da ricaricare al reset
void MemLogDatiConsumo(void)
{
    u8				b[SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN_USED];
    u16				i;

    //	Lo snapshot lo memorizzo solo se la data e ora valida
    if (!IsDateTimeValid (&DateTimeSupinv) || 0 == gTotNodi)
            return;

    //	Cancella il vecchio blocco da 32K ... tutti i dati di consumo di tutte le lampade
    SpiFlashErase (SPI_FLASH_DATI_CONSUMO_NODI_START_ADDR, SPI_OPC_32K_ERASE);

    for (i=0 ; i<gTotNodi ; i++) {
        memset (b, 0, SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN_USED);
        //	Copia ora dell'ultimo poll
        memcpy (&b[DATI_SD_CONS_DATA], (u8 *)(&Node[i].Poll.DatetimeLastPollOkNsec), 4);
        if (IsLampTypeUMDL(i)) 	{
            //	Copia Watt e KWh
            memcpy (&b[DATI_SD_CONS_WATT], (u8 *)(&Node[i].Stato.Contarisparmio.Watt), 10);
            //	Copia tempo ON
            memcpy (&b[DATI_SD_CONS_TIME_ON], (u8 *)(&Node[i].Stato.Contarisparmio.TempoOn), 4);
            //	Copia Pot min e pot max.
            memcpy (&b[DATI_SD_CONS_POT_MIN], (u8 *)(&Node[i].Stato.Contarisparmio.PotMin), 4);
            //	Copia StatoBallast e StatoOnOffErrLamp
            b[DATI_SD_CONS_STATO_BALLAST] = Node[i].Stato.Contarisparmio.StatoBallast;
            //	Copia stato on/off della lampada
            b[DATI_SD_CONS_STATO_ON_OFF_ERR_LAMP] = Node[i].Stato.Contarisparmio.StatoOnOffErrLampada;
            //	Copia lo stato dei bit di errore di comunicazione
            *((u16 *)(&b[DATI_SD_CONS_STATUS])) = Node[i].Poll.Status;
        }
        else if (GetTipoDispositivo(i) == LAMP_LOGICA_FM) {
            //	Copia Stato, Error e Stato SA
            memcpy (&b[DATI_FM_STATO], (u8 *)(&Node[i].Stato.LampLogicaFM.StatoFM), 3);
            //	Copia Stato2 delle lampade logica FM opticom
            b[DATI_FM_STATO_2] = Node[i].Stato.LampLogicaFM.Stato2FM;
            //	Copia stato on/off della lampada
            b[DATI_FM_STATO_ON_OFF_ERR_LAMP] = Node[i].Stato.LampLogicaFM.StatoOnOffErrLampada;
            //	Copia lo stato dei bit di errore di comunicazione
            *((u16 *)(&b[DATI_FM_STATUS])) = Node[i].Poll.Status;
        }
        else if (GetTipoDispositivo(i) == TRASMETTITORE_DOMOTICO) {
            //	Copia lo stato dei bit di errore di comunicazione
            *((u16 *)(&b[DATI_20104_STATUS])) = Node[i].Poll.Status;
        }
        //	Memorizza in spi flash
        SpiFlashWrite ((u32)(SPI_FLASH_DATI_CONSUMO_NODI_START_ADDR+(i*SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN)) , b, SPI_FLASH_DATI_CONSUMO_NODI_1_RECORD_LEN_USED);
    }
}


//	Memorizzazione dello snapshot
void MemLogSnapshot(void)
{
    u8				b;
    u32				l;
    u32				l1;
    u16				Nr;
    u16				i;
    u32				DataOraSnapshot;	
    u8                          b1[4];
    

    //	Lo snapshot lo memorizzo solo se la data e ora valida
    if (!IsDateTimeValid (&DateTimeSupinv))
            return;

    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST, b1, 4);
    l1 = (u32) ((u32)b1[0] << 24) | ((u32)b1[1] << 16) | ((u32)b1[2] << 8) | ((u32)b1[3]);
    l = AddrSpiFlashLogSnapshot;
    //	Salvo adesso la data e ora perche' voglio che tutti i record degli snapshot abbiano la stessa data e ora
    DataOraSnapshot = DateTimeSupinvLegale_NsecFrom_1_1_2000;
    for (i=0 ; i<gTotNodi ; i++)
    {
        //	l = Address della spi flash al quale memorizzare il log

        // Identifico il tipo del dispositivo:
        switch (GetTipoDispositivo((u16)i))
        {
            case CONTARISPARMIO:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_CONTARISPARMIO)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TContaRisparmio));
                    // Numero di byte:
                    Nr = 20 + sizeof(TContaRisparmio);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log dig
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_CONTARISPARMIO;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TContaRisparmio
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.Contarisparmio), sizeof(TContaRisparmio));
                }
                break;

            case MISRAD:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_MISRAD)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TMisRad));
                    // Numero di byte:
                    Nr = 20 + sizeof(TMisRad);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log dig
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_MISRAD;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TMisRad
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.MisRad), sizeof(TMisRad));
                }
                break;

            case MISPOT:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_MISPOT)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TMisPot));
                    // Numero di byte:
                    Nr = 20 + sizeof(TMisPot);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log dig
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_MISPOT;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TMisPot
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.MisPot), sizeof(TMisPot));
                }
                break;

            case LAMPADE_LED_BALERA:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_BALERA)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TLampadeLedBalera));
                    // Numero di byte:
                    Nr = 20 + sizeof(TLampadeLedBalera);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log dig
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_BALERA;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TLampadeLedBalera
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.Balera), sizeof(TLampadeLedBalera));
                }
                break;

            case ZIGBAL:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_ZIGBAL)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TZigBal));
                    // Numero di byte:
                    Nr = 20 + sizeof(TZigBal);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log zigbal
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_ZIGBAL;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    //	Sugli ZigBal.. aggiungi informazione sul TipoExt.. visto che il tipo degli smartdriver (ZigBal) è su due bytes
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.TipoExt), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status)+1, 1);	

                    // TStato: TZigBal
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.ZigBal), sizeof(TZigBal));
                }
                break;

            case AMADORI:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_AMADORI)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TAmadori));
                    // Numero di byte:
                    Nr = 20 + sizeof(TAmadori);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log zigbal
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_AMADORI;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    //	Sugli Amadori.. aggiungi informazione sul TipoExt.. visto che il tipo dei driver è su due bytes
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.TipoExt), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status)+1, 1);	

                    // TStato: TAmadori
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.Amadori), sizeof(TAmadori));
                }
                break;

                
            case MISURATORE:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_MISURATORE)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TMisuratore));
                    // Numero di byte:
                    Nr = 20 + sizeof(TMisuratore);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log misuratore
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_MISURATORE;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TMisuratore
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.Misuratore), sizeof(TMisuratore));
                }
                break;

            case CONCENTRATORE:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_CONCENTRATORE)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TConcentratore));
                    // Numero di byte:
                    Nr = 20 + sizeof(TConcentratore);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log concentratore
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_CONCENTRATORE;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TConcentratore
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.Concentratore), sizeof(TConcentratore));
                }
                break;

            case SENSORE_FOTONICO:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_SENSORE_FOT)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TSensoreFotonico));
                    // Numero di byte:
                    Nr = 20 + sizeof(TSensoreFotonico);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log sensore fotonico
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_SENSORE_FOT;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TSensoreFotonico
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.SensoreFotonico), sizeof(TSensoreFotonico));
                }
                break;

            case LAMPADE_LED:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_LAMPADE_LED)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TLampadeLed));
                    // Numero di byte:
                    Nr = 20 + sizeof(TLampadeLed);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log lampade a led
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_LAMPADE_LED;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TLampadeLed
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.LampadeLed), sizeof(TLampadeLed));
                }
                break;
                
            case LAMP_LOGICA_FM:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_LOGICA_FM)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 22 + 2 + sizeof(TLampLogicaFM));
                    // Numero di byte:
                    Nr = 22 + sizeof(TLampLogicaFM);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log lampade logica FM
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_LOGICA_FM;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 8);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TLampLogicaFM
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.LampLogicaFM), sizeof(TLampLogicaFM));
                }
                break;

            case SENSORI_AUTOMAZIONE:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_SENS_AUT)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TSensoriAutomazione));
                    // Numero di byte:
                    Nr = 20 + sizeof(TSensoriAutomazione);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log lampade a led
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_SENS_AUT;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TSensoriAutomazione
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.SensoriAutomazione), sizeof(TSensoriAutomazione));
                }
                break;

            case TRASMETTITORE_DOMOTICO:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_TRASM_DOMOTICO)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TTrasmettitoreDomotico));
                    // Numero di byte:
                    Nr = 20 + sizeof(TTrasmettitoreDomotico);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log lampade a led
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_TRASM_DOMOTICO;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TTrasmettitoreDomotico
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.TrasmettitoreDomotico), sizeof(TTrasmettitoreDomotico));
                }
                break;

            case RELAIS_DOMOTICO:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_RELAIS_DOMOTICO)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TRelaisDomotico));
                    // Numero di byte:
                    Nr = 20 + sizeof(TRelaisDomotico);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log dig
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_RELAIS_DOMOTICO;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TRelaisDomotico
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.RelaisDomotico), sizeof(TRelaisDomotico));
                }
                break;

            case GP_ATMEL:
                if (!BitOp (TEST_BIT, MapBitLogDisabled, SPI_FLASH_OPC_LOG_SNAP_GP_ATMEL)) {
                    AllineaPtrAFineLogMem (LOG_SNAPSHOT, &l, &l1, 20 + 2 + sizeof(TGpAtmel));
                    // Numero di byte:
                    Nr = 20 + sizeof(TGpAtmel);			
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    //	Scrivi data/ora di memorizzazione del log dispositivo general purpouse ATMEL
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&DataOraSnapshot), 4);

                    //	Opcode
                    b = SPI_FLASH_OPC_LOG_SNAP_GP_ATMEL;
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, &b, 1);

                    //	Versione sw SupInv
                    Nr = GetVerRom();
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *)(&Nr), 2);

                    // TConfig:
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.Addr), 4);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Config.NodeType), 6);			

                    // TPoll
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.LinkQuality), 1);
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Poll.Status), 2);	

                    // TStato: TGpAtmel
                    SpiFlashWriteLog (LOG_SNAPSHOT, &l, &l1, (u8 *) (&Node[i].Stato.GpAtmel), sizeof(TGpAtmel));
                }
                break;


        }
    }
    //	Memorizza nuovo puntatore:
    AddrSpiFlashLogSnapshot = l;

}



//	SearchLogSpiFlash : Cerca in memoria il log
//	Input 	: 	Tipo di Log, NsecStart, NsecEnd
//	Output	:	Puntatore iniziale alla spi flash, puntatore finale alla spi flash 
//	Output	:	TRUE 	==> Log trovato
//			:	FALSE	==> Log NON trovato
boolean SearchLogSpiFlash (u8 TipoLog, u32 NSecStart, u32 NSecEnd, u32 *pStartSpiFlash, u32 *pEndSpiFlash)
{
    u32			PtrOldest;
    u32			Ptr;
    u32			AddrLogOldest;
    u32			AddrLog;
    u8			b[7];
    u8			b1[4];
    u16			LenRec;
    u32			NSec;
    u8			OpcLog;
    boolean		bFound;
    boolean		bEnd;
    u32			LenLog;
    u32			LenTotLog;

    *pStartSpiFlash = 0;
    *pEndSpiFlash = 0;
    switch (TipoLog)
    {
    case LOG_BOOK:
        AddrLogOldest = SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST;
        AddrLog = AddrSpiFlashLogBook;
        LenLog = SPI_FLASH_LOG_BOOK_LEN;
        break;
    case LOG_SNAPSHOT:
        AddrLogOldest = SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST;
        AddrLog = AddrSpiFlashLogSnapshot;
        LenLog = SPI_FLASH_SNAPSHOT_LEN;
        break;
    }
    //	Leggi i ptr al record piu vecchio del log e a quello attuale
    
    
    SpiFlashReadParam (AddrLogOldest, &b1[0], 4);
    PtrOldest = (u32) ((u32)(b1[0]) << 24) | ((u32)(b1[1]) << 16) | ((u32)(b1[2]) << 8) | (u32)(b1[0]);
    SpiFlashReadParam (AddrLog, &b1[0], 4);
    Ptr = (u32) ((u32)(b1[0]) << 24) | ((u32)(b1[1]) << 16) | ((u32)(b1[2]) << 8) | (u32)(b1[0]);
    //	Ricerca la prima data a partire dall'oldest >= NSec Start
    //	E' stato messo controllo sul ricircolo del buffer senza trovare nulla... evita loop infinito
    bFound = FALSE;
    LenTotLog = 0;
    while (!bFound && (PtrOldest!=Ptr) && LenTotLog < LenLog) {
        //	Leggi i primi 7 bytes del record
        SpiFlashRead (PtrOldest, b, 7);
        LenRec = (u16)(((u16)(b[0]) << 8) | (b[1]));
        NSec =  ((u32)(b[2]) << 24) | ((u32)(b[3]) << 16) | ((u32)(b[4]) << 8) | (b[5]);
        OpcLog = b[6];
        if (LenRec == 0xffff || (NSec >= NSecStart)) {
            *pStartSpiFlash = PtrOldest;
            *pEndSpiFlash = PtrOldest;
            bFound = TRUE;
        }
        else {	
            AddPtrLogSpiFlash (TipoLog, &PtrOldest, LenRec + 2);
            LenTotLog += (LenRec + 2);
        }	
    }

    //	Ricerca la data end a partire da quello appena trovato il piu grande <= di Nsec End
    if (bFound) {
        bEnd = FALSE;
        LenTotLog = 0;
        while (!bEnd && (PtrOldest!=Ptr) && LenTotLog < LenLog) {
            //	Leggi i primi 7 bytes del record
            SpiFlashRead (PtrOldest, b, 7);
            LenRec = (u16)(((u16)(b[0]) << 8) | (b[1]));
            NSec =  ((u32)(b[2]) << 24) | ((u32)(b[3]) << 16) | ((u32)(b[4]) << 8) | (b[5]);
            OpcLog = b[6];
            if (LenRec != 0xffff && NSec <= NSecEnd ) {
                AddPtrLogSpiFlash (TipoLog, &PtrOldest, LenRec + 2);		
                *pEndSpiFlash = PtrOldest;
                LenTotLog += (LenRec + 2);
            }
            else
                    bEnd = TRUE;
        }
    }
    //	Se i due ptr sono rimasti uguali non ho trovato nulla
    if (*pStartSpiFlash == *pEndSpiFlash)
        bFound = FALSE;
    else {
        bFound = TRUE;
        SubPtrLogSpiFlash (TipoLog, pEndSpiFlash, 1);		
    }	
    return (bFound);
}


void LoadLogPtrSpiFlash (u8 TipoLog)
{
    u32			AddrStartLog;
    u32			AddrEndLog;
    u32			AddrRunning;
    boolean                 bLogFail;
    u16			LenRecordLog;
    u8                  b[2];
    u32                 l;

    switch (TipoLog)
    {
    case LOG_BOOK:
        AddrStartLog = SPI_FLASH_LOG_BOOK_START_ADDR;
        AddrEndLog = SPI_FLASH_LOG_BOOK_END_ADDR;
        break;
    case LOG_SNAPSHOT:
        AddrStartLog = SPI_FLASH_SNAPSHOT_START_ADDR;
        AddrEndLog = SPI_FLASH_SNAPSHOT_END_ADDR;
        break;
    }
    AddrRunning = AddrStartLog;
    bLogFail = FALSE;
    while (!bLogFail) {
        SpiFlashRead (AddrRunning, b, 2);
        LenRecordLog = (u16) (((u16)b[0] << 8) | (b[1]));
        
        //	Se la lunghezza del record del log = 0xffff ...e' il posto dei pointer al log
        if (LenRecordLog == 0xffff)
            break;
        //	Se c'e' stato uno "scavallo" sulla fine del buffer circolare
        if (AddPtrLogSpiFlash (TipoLog, &AddrRunning, (LenRecordLog + 2)) == TRUE)
            bLogFail = TRUE;
    }
    switch (TipoLog)
    {
    case LOG_BOOK:
        if (bLogFail) {
            //	Il puntatore al log deve puntare a una zona gia' sicuramente cancellata
            SpiFlashErase (AddrStartLog, SPI_OPC_4K_ERASE);
            AddrSpiFlashLogBook = AddrStartLog;
            l = (u32) (AddrStartLog);
            b[0] = ((l >> 24) & 0xff);
            b[1] = ((l >> 16) & 0xff);
            b[2] = ((l >> 8) & 0xff);
            b[3] = (l & 0xff);
            SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_LOG_BOOK_PTR_OLDEST, b, 4, FALSE);
        }
        else
            AddrSpiFlashLogBook = AddrRunning;
        break;
    case LOG_SNAPSHOT:
        if (bLogFail) {
            //	Il puntatore al log deve puntare a una zona gia' sicuramente cancellata
            SpiFlashErase (AddrStartLog, SPI_OPC_4K_ERASE);
            AddrSpiFlashLogSnapshot = AddrStartLog;
            l = (u32) (AddrStartLog);
            b[0] = ((l >> 24) & 0xff);
            b[1] = ((l >> 16) & 0xff);
            b[2] = ((l >> 8) & 0xff);
            b[3] = (l & 0xff);
            SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_SNAPSHOT_PTR_OLDEST, b, 4, FALSE);
        }
        else
            AddrSpiFlashLogSnapshot = AddrRunning;
        break;
    }
}


//	Aggiorna data/ora del prossimo test funzionale se scattata l'ora
//	Output : TRUE ==> L'ora e' scattato ed e' stato aggiornato l'orario del prossimo test funzionale
boolean UpdateNextDataOraTestFunz(void) {
    u32				l;
    u32				l1;
    u32				l2;
    u8				b[6];
    u16				NroGiorni;
    tm_cenlin                   DateTimeNextFunzTimer;


    //	Lettura della data/ora prossimo test funzionale (gg-mm-aa hh:mm:ss)
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ, b, 6);
    l = GetAbsoluteSeconds (b[0], b[1], (u16)(b[2]+2000), b[3], b[4], b[5], TRUE);
    //	Verifica se l'orario del test funzionale e' passato 
    if (DateTimeSupinvLegale_NsecFrom_1_1_2000 >= l) {
        //	Memorizza data/ora dell'ultimo test funzionale eseguito
        SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_LG_FM_LAST_DATA_ORA_TEST_FUNZ, b, 6, FALSE);	
        //	Aggiorna prossima data/ora del test funzionale (leggi la frequenza di esecuzione del test funzionale
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_FUNZ, b, 2);
        NroGiorni = (u16) (((u16)b[0] << 8) | b[1]);
        if (0==NroGiorni ||  NroGiorni > SPI_PAR_LG_FM_FREQ_MIN_TEST_FUNZ)
                NroGiorni = SPI_PAR_LG_FM_FREQ_TEST_FUNZ_DEFAULT;
        //	Porta avanti l'orario del prossimo test funzionale, almeno fino a che' non supera la data/ora attuale
        while (l < DateTimeSupinvLegale_NsecFrom_1_1_2000)
            l += ((u32)NroGiorni * 86400);
        //	Se la data/ora calcolata per il prossimo test funzionale si incastra nel prossimo test autonomia (tieni conto anche se si tratta di test autonomia differito....)
        //	porto avanti il test funzionale almeno di 1 giorno dopo la scadenza del test di autonomia.

        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT, b, 6);
        //	l1 ==> data/ora inizio prossimo test autonomia (num. sec. da 1/1/2000)
        l1 = GetAbsoluteSeconds (b[0], b[1], (u16)(b[2]+2000), b[3], b[4], b[5], TRUE);
        //	l2 ==> data/ora fine prossimo test autonomia, della ricarica della batteria e della raccolta dei corrispondenti dati (num. sec. da 1/1/2000)
        l2 = l1 + 86400;
        if (BitOp (TEST_BIT, SupinvFlags, SPI_FLAGS_TEST_AUT_DIFFERITO))
            l2 +=  (DAY_7/1000);

        //	Programmo il prossimo test funzionale ad una data/ora che non si accavalli con prossimo test autonomia (ricarica e raccolta dati compresa) 
        while (l>l1 && l<l2)
            l += 86400;

        getDateTimeFromSec (&DateTimeNextFunzTimer, l);
        b[0] = (u8)DateTimeNextFunzTimer.tm_mday;
        b[1] = (u8)DateTimeNextFunzTimer.tm_mon;
        b[2] = (u8)(DateTimeNextFunzTimer.tm_year - 2000);
        b[3] = (u8)DateTimeNextFunzTimer.tm_hour;
        b[4] = (u8)DateTimeNextFunzTimer.tm_min;
        b[5] = (u8)DateTimeNextFunzTimer.tm_sec;
        //	Aggiorna data/ora prossimo test funzionale
        SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ, b, 6, FALSE);	
        return (TRUE);
    }
    else
        return (FALSE);
}


//	Aggiorna data/ora del prossimo test autonomia se scattata l'ora
//	Output : TRUE ==> L'ora e' scattato ed e' stato aggiornato l'orario del prossimo test autonomia
boolean UpdateNextDataOraTestAut(void) {
    u32				l;
    u32				l1;
    u32				l2;
    
    u8				b[6];
    u16				NroGiorni;
    tm_cenlin   		DateTimeNextAutTimer;


    //	Lettura della data/ora prossimo test autonomia (gg-mm-aa hh:mm:ss)
    SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT, b, 6);
    l = GetAbsoluteSeconds (b[0], b[1], (u16)(b[2]+2000), b[3], b[4], b[5], TRUE);
    //	Verifica se l'orario del test autonomia e' passato 
    if (DateTimeSupinvLegale_NsecFrom_1_1_2000 >= l) {
        //	Memorizza data/ora dell'ultimo test autonomia eseguito
        SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_LG_FM_LAST_DATA_ORA_TEST_AUT, b, 6, FALSE);	
        //	Aggiorna prossima data/ora del test autonomia (leggi la frequenza di esecuzione del test funzionale
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LG_FM_FREQ_TEST_AUT, b, 2);
        NroGiorni = (u16) (((u16)b[0] << 8) | b[1]);
        if (0==NroGiorni ||  NroGiorni > SPI_PAR_LG_FM_FREQ_MIN_TEST_AUT)
                NroGiorni = SPI_PAR_LG_FM_FREQ_TEST_AUT_DEFAULT;

        //	Porta avanti l'orario del prossimo test autonomia, almeno fino a che' non supera la data/ora attuale
        while (l < DateTimeSupinvLegale_NsecFrom_1_1_2000)
                l += ((u32)NroGiorni * 86400);
        
        //	Se la data/ora calcolata per il prossimo test autonomia si incastra nel prossimo test funzionale (tieni conto anche se si tratta di test autonomia differito....)
        //	porto avanti il test autonomia almeno di 1 giorno dopo la scadenza del test funzionale.
        SpiFlashReadParam (SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_FUNZ, b, 6);
        //	l1 ==> data/ora inizio prossimo test funzionale (num. sec. da 1/1/2000)
        l1 = GetAbsoluteSeconds (b[0], b[1], (u16)(b[2]+2000), b[3], b[4], b[5], TRUE) - 86400;
        //	l2 ==> data/ora fine prossimo test funzionale, della ricarica della batteria e della raccolta dei corrispondenti dati (num. sec. da 1/1/2000)
        l2 = l1 + (86400 * 2);
        //	Programmo il prossimo test funzionale ad una data/ora che non si accavalli con prossimo test funzionale (ricarica e raccolta dati compresa)
        while (l>l1 && l<l2)
                l += 86400;
        
        getDateTimeFromSec (&DateTimeNextAutTimer, l);
        b[0] = (u8)DateTimeNextAutTimer.tm_mday;
        b[1] = (u8)DateTimeNextAutTimer.tm_mon;
        b[2] = (u8)(DateTimeNextAutTimer.tm_year - 2000);
        b[3] = (u8)DateTimeNextAutTimer.tm_hour;
        b[4] = (u8)DateTimeNextAutTimer.tm_min;
        b[5] = (u8)DateTimeNextAutTimer.tm_sec;
        //	Aggiorna data/ora prossimo test autonomia
        SpiFlashWriteParam (SPI_FLASH_ADDR_PAR_LG_FM_DATA_ORA_TEST_AUT, b, 6, FALSE);	
        return (TRUE);
    }
    else
        return (FALSE);
}



