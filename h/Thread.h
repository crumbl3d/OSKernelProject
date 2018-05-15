#ifndef _thread_h_
#define _thread_h_

typedef unsigned long StackSize;
typedef unsigned int Time; // time, x 55ms
typedef int ID;

const StackSize defaultStackSize = 4096;
//const StackSize maxStackSize = 65535;
const Time defaultTimeSlice = 2; // default = 2*55ms

// Kernel's implementation of a user's thread
class PCB
{
public:
    unsigned *stack;
    unsigned ss;
    unsigned sp;
    unsigned bp;
    unsigned timeSlice;
};

// User and kernel thread.
class Thread
{
public:
    void start();
    void waitToComplete();

    virtual ~Thread();

    static void sleep(Time timeToSleep);
protected:
    friend class PCB;

    Thread(StackSize stackSize = defaultStackSize, Time timeSlice =
           defaultTimeSlice);

    virtual void run() {}
private:
    PCB* myPCB;
};

void dispatch ();

#endif