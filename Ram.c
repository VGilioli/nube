#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>   // File Control Definitions           
#include <termios.h> // POSIX Terminal Control Definitions 
#include <unistd.h>  // UNIX Standard Definitions 	   
#include <errno.h>   // ERROR Number Definitions           
#include <time.h>
#include <pthread.h>        
#include <semaphore.h>
#include "def.h"
#include "Ram.h"
#include "_serial.h"

u8      	BufferUpdate[512*1024];
//      Tipo di centrale FM oppure SD
u8              TipoCentrale;
u8		SupinvFlags[8];
//	CVPS della centrale
u16		CVPSSupInv;
//	Codice di impianto della centrale (1..92)
u8		CodiceImpiantoSupInv;
//      Address sul bus Rs485 della centrale Supinv
u8		AddressBusCentraleSupInv[MAX_NUM_UART];
//      Mappa a bit dei log disabilitati
u8		MapBitLogDisabled[32];
//      Puntatore in spi dei log
u32		AddrSpiFlashLogBook;
u32		AddrSpiFlashLogSnapshot;
u32		AddrSpiFlashLogDig;

//	Codice di identificazione della centrale (etichetta)
u32		EtichettaSupInv;
sem_t           semStatoNodi;
//  Allarme sms
u32             MaskSmsAlarm;
sem_t           semSmsAlarm;
//	Lunghezza del programma di cui e stato fatto upgrade 
u32		lLenProgramUpdate;
//	ATTENZIONE ---------- OBBLIGATORIO tenere variabile RamProgNewCode[1000] in ultima posizione -----------------
//	Spazio ProgNewCode in ram (spazio codice)
u8              RamProgNewCode[1000];
//	ATTENZIONE ---------- OBBLIGATORIO tenere variabile RamProgNewCode[1000] in ultima posizione -----------------

u8              MapBitScene[5];
