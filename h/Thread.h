/*
 * Thread.h
 * 
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _THREAD_H_
#define _THREAD_H_

typedef unsigned long StackSize;
typedef unsigned int Time; // time, x 55ms
typedef int ID;

const StackSize defaultStackSize = 4096; // default = 4KB
const Time defaultTimeSlice = 2; // default = 2*55ms

class PCB; // Kernel's implementation of a user's thread

class Thread
{
public:
    void start();
    void waitToComplete();

    virtual ~Thread();

    static void sleep(Time timeToSleep);
protected:
    friend class PCB;

    Thread(StackSize stackSize = defaultStackSize,
           Time timeSlice = defaultTimeSlice);

    virtual void run() {}
private:
    ID mID;

    static void wrapper(Thread *running);

    // remove when syscall is done
    PCB* mPCB;
};

void dispatch();

#endif /* _THREAD_H_ */