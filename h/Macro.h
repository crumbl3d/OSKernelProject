/*
 * Macro.h
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _MACRO_H_
#define _MACRO_H_

#define softlock() asm cli
#define softunlock() asm sti

#define lock() asm { pushf; cli; }
#define unlock() asm popf

#define callint(N) asm int N

#endif /* _MACRO_H_ */