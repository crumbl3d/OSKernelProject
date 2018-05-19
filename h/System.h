/*
 * System.h
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

// Only for VS Code to stop IntelliSense from crapping out.
#ifdef BCC_BLOCK_IGNORE
#define interrupt
#endif

typedef void interrupt (*pInterrupt)(...);

const unsigned TimerEntry = 0x08;
const unsigned NewTimerEntry = 0x60;

class KernelThr;

class System
{
public:
    enum Task { None, Dispatch }; // TODO: Write all tasks for interrupt.

    static void initialize();
    static void finalize();

    static void sleep(unsigned timeToSleep);
    static void dispatch();

    static void threadWrapper();
    static void threadExit();
private:    
    static void interrupt newTimer(...);

    static void hijack_timer();
    static void restore_timer();

    static pInterrupt oldTimer;
    static volatile Task currentTask;
    static volatile unsigned counter;
    static volatile KernelThr *initial, *idle, *running;
};

#endif /* _SYSTEM_H_ */
