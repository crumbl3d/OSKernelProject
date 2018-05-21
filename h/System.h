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

struct ObjectType { enum ObjectTypeEnum { Thread, Semaphore, Event }; };
struct ThreadRequestType { enum ThreadRequestTypeEnum { Create, Destroy, Start, Stop, WaitToComplete, Sleep, Dispatch }; };
struct SemaphoreRequestType { enum SemaphoreRequestTypeEnum { Create, Destroy, Wait, Signal }; };
struct EventRequestType { enum EventRequestTypeEnum { Create, Destroy, Wait, Signal }; };

struct SysCallData
{
    unsigned objType;
    unsigned reqType;
    // TODO: add data...
};

class System
{
public:
    static void initialize();
    static void finalize();

    static void threadPut(PCB *thread);
    static void threadPriorityPut(PCB *thread);
    static PCB* threadGet();

    static void threadStop();

//private:
    static void interrupt newTimerRoutine(...);
    static void interrupt sysCallRoutine(...);

    static void idleBody();
    static void kernelBody();
    
    static void lock();
    static void unlock();

    static pInterrupt oldTimerRoutine;
    // Global system call data.
    static volatile SysCallData *callData;
    // Kernel flags: locked (forbid preemption),
    //               changeContext (requested context change)
    //               systemChangeContext (requested context change on system call)
    //               restoreUserThread (restore user thread after system call)
    static volatile unsigned locked, changeContext, systemChangeContext, restoreUserThread;
    // Counters: tickCount (number of timer ticks passed),
    //           readyThreadCount (number of threads in the scheduler)
    static volatile unsigned tickCount, readyThreadCount;
    // System threads: initial (the initial thread that runs main method)
    //                 idle (when there are no ready threads this thread must run)
    //                 running (current user thread)
    //                 runningKernelThread (kernel thread for processing system calls)
    static volatile PCB *initial, *idle;
    static volatile PCB *running, *runningKernelThread;
    // Thread lists: prioritized (contains the prioritized threads which will be run
    //                            before any thread in the scheduler)
    //               sleeping (contains the threads that are blocked with the sleep method)
    //               blocked (contains the threads that are blocked with the waitToComplete method)
    static volatile PCB *prioritized, *sleeping, *blocked;
};

#endif /* _SYSTEM_H_ */
