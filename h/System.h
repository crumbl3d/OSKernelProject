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
const unsigned SysCallEntry = 0x61;

class PCB;

struct SysCallData
{
    enum ClassName { Thread, Semaphore, Event };
    enum ThreadCallName { Dispatch }; // TODO: Write all calls...
    enum SemaphoreCallName { }; // TODO: Write all calls...
    enum EventCallName { }; // TODO: Write all calls...

    ClassName clsName;
    int callName;
    // TODO: add data...
};

class System
{
public:
    static void initialize();
    static void finalize();

    static void sleep(unsigned timeToSleep);
    static void dispatch();

    static void threadPut(PCB *thread);
    static void threadPriorityPut(PCB *thread);
    static PCB* threadGet();

    static void threadExit();
private:
    static void interrupt newTimer(...);
    static void interrupt sysCall(...);

    static void idleBody();

    static void lock();
    static void unlock();

    static pInterrupt oldTimer;
    // Global system call data.
    static volatile SysCallData *callData;
    // Kernel flags: locked (forbid preemption),
    //               changeContext (requested context change)
    static volatile unsigned locked, changeContext;
    // Counters: tickCount (number of timer ticks passed),
    //           threadCount (number of threads in the scheduler)
    static volatile unsigned tickCount, threadCount;
    // System threads: initial
    static volatile PCB *initial, *idle; // initial and idle threads
    static volatile PCB *running, *runningKernelThread; // current user and kernel threads
    static volatile PCB *prioritized, *sleeping, *blocked; // prioritized, sleeping and blocked thread queues
};

#endif /* _SYSTEM_H_ */
