/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Ram.h
 * Author: variscite
 *
 * Created on December 5, 2018, 4:57 PM
 */

#ifndef RAM_H
#define RAM_H

//	Buffer update (upgrade software centrale)
extern      u8          BufferUpdate[];
extern      u8          TipoCentrale;
extern      u8          SupinvFlags[8];
extern      u16		CVPSSupInv;
extern      u8		CodiceImpiantoSupInv;
extern      u32		EtichettaSupInv;
//extern      sem_t       semStatoNodi;
extern      u32         MaskSmsAlarm;
//extern      sem_t       semSmsAlarm;
extern      u32		lLenProgramUpdate;
extern      u8          RamProgNewCode[1000];
extern      u8		MapBitLogDisabled[];
extern      u8		AddressBusCentraleSupInv[];
extern      u32		AddrSpiFlashLogBook;
extern      u32		AddrSpiFlashLogSnapshot;
extern      u32		AddrSpiFlashLogDig;
extern      u8          MapBitScene[];

#endif /* RAM_H */

