/*
 * Macro.h
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _MACRO_H_
#define _MACRO_H_

// Lock/Unlock maskable interrupts.
#define asmLock() asm { pushf; cli; }
#define asmUnlock() asm popf

// Call interrupt routine with number N.
#define asmInterrupt(N) asm int N

// Used only for sysCall macro as temporary storage.
static volatile unsigned tempSEG, tempOFF;

// Call system routine for kernel level processing.
// TODO: remove printf altogether...
#define sysCall(DATA) tempSEG = FP_SEG(&DATA); \
tempOFF = FP_OFF(&DATA); \
/*printf("seg: %d off %d\n", tempSEG, tempOFF);*/ \
asm { push cx; push dx; mov cx, tempSEG; mov dx, tempOFF }; \
asmInterrupt(SysCallEntry); \
asm { pop dx; pop cx }

#endif /* _MACRO_H_ */