/*
 * KThread.cpp
 * 
 * Created on: May 16, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <mem.h>
#include <stdlib.h>

#include "Macro.h"
#include "KThread.h"
#include "System.h"

unsigned PCB::capacity = InitialObjectCapacity, PCB::count = 0;
PCB **PCB::objects = 0;
static PCB *temp = 0;

PCB::PCB ()
{
    initialize(0, 0, 0, defaultTimeSlice);
}

PCB::PCB (ThreadBody body, StackSize stackSize, Time timeSlice)
{
    initialize(0, body, stackSize, timeSlice);
}

PCB::PCB (Thread *userThread, StackSize stackSize, Time timeSlice)
{
    initialize(userThread, 0, stackSize, timeSlice);
}

PCB::~PCB ()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // Only delete the stack if it was created.
    if (mStack) delete [] mStack;
    objects[mID] = 0;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
    // printf("ID to delete: %d\n", mID);
    // #ifndef BCC_BLOCK_IGNORE
    // if (objects)
    //     for (unsigned i = 0; i < count; ++i)
    //         printf("object[%d]: SEG = %d OFF = %d\n", i, FP_SEG(objects[i]), FP_OFF(objects[i]));
    // #endif
}

void PCB::start ()
{
    System::threadPut(this);
}

void PCB::waitToComplete ()
{
    // Only block the running thread if this thread is not terminated!
    if (mState != ThreadState::Terminated)
    {
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        #endif
        System::running->mState = ThreadState::Blocked;
        System::running->mNext = mBlocked;
        mBlocked = (PCB*) System::running;
        System::dispatch();
        #ifndef BCC_BLOCK_IGNORE
        asmUnlock();
        #endif
    }
}

void PCB::stop ()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    // printf("Stopping the thread with ID = %d!\n", System::running->mID);
    System::running->mState = ThreadState::Terminated;
    // Unblock any blocked threads by setting their state to Ready,
    // and putting them into the scheduler.
    while (System::running->mBlocked)
    {
        // printf("Unblocking the thread with ID = %d!\n", System::running->mBlocked->mID);
        // Setting the state to Running because threadPut will
        // then reset it to Ready. Otherwise it wont put it inside
        // the scheduler (by design).
        System::running->mBlocked->mState = ThreadState::Running;
        System::threadPut(System::running->mBlocked);
        temp = System::running->mBlocked;
        System::running->mBlocked = System::running->mBlocked->mNext;
        temp->mNext = 0;
    }
    System::dispatch();
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif
}

void PCB::sleep (unsigned timeToSleep)
{
    if (timeToSleep > 0)
    {
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        #endif
        // Adding the running thread to the sleeping list.
        temp = (PCB*) System::running;
        temp->mTimeLeft = timeToSleep;
        temp->mState = ThreadState::Blocked;
        PCB *previous = 0, *current = (PCB*) System::sleeping;
        while (current && temp->mTimeLeft >= current->mTimeLeft)
        {
            // No need to go through all of the same ones,
            // just put it at the front.
            temp->mTimeLeft -= current->mTimeLeft;
            previous = current;
            current = current->mNext;
            if (temp->mTimeLeft == 0) break;
        }
        if (previous) previous->mNext = temp;
        else System::sleeping = temp;
        if (current) current->mTimeLeft -= temp->mTimeLeft;
        temp->mNext = current;
        current = (PCB*) System::sleeping;
        // printf("Printing the list of sleeping threads!\n");
        // while (current)
        // {
        //     printf("ID = %d TimeLeft = %d\n", current->mID, current->mTimeLeft);
        //     current = current->mNext;
        // }
        System::dispatch();
        #ifndef BCC_BLOCK_IGNORE
        asmUnlock();
        #endif
    }
}

PCB* PCB::getAt (unsigned index)
{
    if (index < count) return objects[index];
    else return 0;
}

void PCB::initialize (Thread *userThread, ThreadBody body,
                      StackSize stackSize, Time timeSlice)
{
    if (!userThread && !body)
    {
        // No body, just initialize the private members.
        mStack = 0;
        mBP = mSP = mSS = 0;
    }
    else
    {
        if (stackSize < minStackSize) stackSize = minStackSize;
        if (stackSize > maxStackSize) stackSize = maxStackSize;
        stackSize /= sizeof(unsigned); // BYTE to WORD
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        #endif
        mStack = new unsigned[stackSize];
        if (body)
        {
            // Has body, but no user thread.
            mStack[stackSize - 1] = 0x200; // PSW
            #ifndef BCC_BLOCK_IGNORE
            mStack[stackSize - 2] = FP_SEG(body); // CS
            mStack[stackSize - 3] = FP_OFF(body); // PC
            mSS = FP_SEG(mStack + stackSize - 12); // BP
            mBP = mSP = FP_OFF(mStack + stackSize - 12); // BP
            #endif
        }
        else
        {
            // Has user thread - implicit body.
            mStack[stackSize - 5] = 0x200; // PSW
            #ifndef BCC_BLOCK_IGNORE
            mStack[stackSize - 1] = FP_SEG(userThread); // wrapper() param
            mStack[stackSize - 2] = FP_OFF(userThread); // wrapper() param
            mStack[stackSize - 6] = FP_SEG(Thread::wrapper); // CS
            mStack[stackSize - 7] = FP_OFF(Thread::wrapper); // PC
            mSP = mBP = FP_OFF(mStack + stackSize - 16); // BP
            mSS = FP_SEG(mStack + stackSize - 16); // BP
            #endif
        }
        #ifndef BCC_BLOCK_IGNORE
        asmUnlock();
        #endif
    }
    mTimeSlice = timeSlice;
    mTimeLeft = 0;
    mState = ThreadState::New;
    mThread = userThread;
    mNext = mBlocked = 0;
    mID = count++;
    if (count > capacity)
    {
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        #endif
        // printf("Resizing thread object array!\n");
        PCB **newObjects = (PCB**) calloc(capacity << 1, sizeof(PCB*));
        if (objects)
        {
            memcpy(newObjects, objects, capacity * sizeof(PCB*));
            free(objects);
        }
        if (newObjects)
        {
            objects = newObjects;
            capacity <<= 1;
        }
        // else printf("Failed to resize thread object array!\n");
        #ifndef BCC_BLOCK_IGNORE
        asmUnlock();
        #endif
    }
    if (objects)  objects[mID] = this;
    // DEBUG ONLY!!! REMOVE!!!
    // #ifndef BCC_BLOCK_IGNORE
    // for (unsigned i = 0; i < count; ++i)
    //     printf("object[%d]: SEG = %d OFF = %d\n", i, FP_SEG(objects[i]), FP_OFF(objects[i]));
    // #endif
    // else printf("Invalid thread object array!\n");
}

PCBQueue::PCBQueue () : mFirst(0), mLast(0) {}
PCBQueue::~PCBQueue ()
{
    // We only need to unlink the PCBs and put them into the scheduler.
    while (mFirst)
    {
        // Setting the state to Running because threadPut will
        // then reset it to Ready. Otherwise it wont put it inside
        // the scheduler (by design).
        mFirst->mState = ThreadState::Running;
        System::threadPut(mFirst);
        temp = mFirst;
        mFirst = mFirst->mNext;
        temp->mNext = 0;
    }
    mLast = 0;
}

void PCBQueue::put (PCB *thread)
{
    mLast = (!mFirst ? mFirst : mLast->mNext) = thread;
}

PCB* PCBQueue::get ()
{
    temp = mFirst;
    if (mFirst) mFirst = mFirst->mNext;
    else mFirst = mLast = 0;
    if (temp) temp->mNext = 0; // Remove from the queue!
    return temp;
}