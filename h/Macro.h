/*
 * Macro.h
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _MACRO_H_
#define _MACRO_H_

// #define asmSingleLock() asm cli
// #define asmSingleUnlock() asm sti

#define asmLock() asm { pushf; cli; }
#define asmUnlock() asm popf

#define asmInterrupt(N) asm int N

#endif /* _MACRO_H_ */