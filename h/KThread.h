/*
 * KThread.h
 * 
 * Created on: May 17, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _KTHREAD_H_
#define _KTHREAD_H_

class Thread;
class System;

struct PCB
{
    enum State { New, Ready, Running, Blocked, Terminated };

    unsigned *stack;
    unsigned ss;
    unsigned sp;
    unsigned bp;
    unsigned quantum;
    State state;
};

class KernelThr
{
public:
    // stackSize - in BYTE (8b), quantum - in x55ms,
    // thread - if created by a Thread object
    KernelThr(unsigned long stackSize, unsigned quantum,
              Thread *thread = 0);

    virtual ~KernelThr();

    void start();
    void waitToComplete();
protected:
    friend class System;

    virtual void run() {}
private:
    PCB *mPCB;
    Thread *mThread;
    KernelThr *mNext;
};

#endif /* _KTHREAD_H_ */