/*
 * KEvent.cpp
 * 
 * Created on: May 26, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdlib.h>

#include "Macro.h"
#include "Event.h"
#include "KEvent.h"
#include "KThread.h"
#include "System.h"

// Static initialization required for static interrupt initialization.
KernelEv *KernelEv::objects[NumOfIVTEntries];
IVTEntry *IVTEntry::objects[NumOfIVTEntries];

KernelEv::KernelEv (IVTNo ivtNo)
{
    initialize(0, ivtNo);
}

KernelEv::KernelEv (Event *userEv, IVTNo ivtNo)
{
    initialize(userEv, ivtNo);
}

KernelEv::~KernelEv ()
{
    // Deblock the blocked thread.
    if (mValue == -1)
    {
        mCreator->mState = ThreadState::Running;
        System::threadPut(mCreator);
    }
    objects[mIVTNo] = 0;
}

void KernelEv::wait ()
{
    #ifndef BCC_BLOCK_IGNORE
    System::lock();
    #endif
    if (System::running->mID == mCreator->mID)
    {
        if (mValue) mValue = 0;
        else
        {
            mCreator->mState = ThreadState::Blocked;
            System::dispatch();
        }
    }
    #ifndef BCC_BLOCK_IGNORE
    System::unlock();
    #endif
}

void KernelEv::signal ()
{
    #ifndef BCC_BLOCK_IGNORE
    System::lock();
    #endif
    if (mCreator->mState != ThreadState::Blocked) mValue = 1;
    else
    {
        mCreator->mState = ThreadState::Running;
        System::threadPriorityPut(mCreator);
    }
    #ifndef BCC_BLOCK_IGNORE
    System::unlock();
    #endif
}

void KernelEv::callSignal ()
{
    if (System::kernelMode || !mEvent) signal();
    else mEvent->signal();
}

KernelEv* KernelEv::at (unsigned char index)
{
    return objects[index];
}

void KernelEv::initialize (Event *userEv, IVTNo ivtNo)
{
    mValue = 0;
    mEvent = userEv;
    mCreator = (PCB*) System::running;
    mIVTNo = ivtNo;
    objects[mIVTNo] = this;
}

IVTEntry::IVTEntry (IVTNo ivtNo, InterruptRoutine routine)
{
    mIVTNo = ivtNo;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    mOldRoutine = getvect(ivtNo);
    setvect(ivtNo, routine);
    asmUnlock();
    #endif
    objects[mIVTNo] = this;
}

IVTEntry::~IVTEntry ()
{
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    setvect(mIVTNo, mOldRoutine);
    asmUnlock();
    #endif
    objects[mIVTNo] = 0;
}

void IVTEntry::callOldRoutine ()
{
    if (mOldRoutine) mOldRoutine();
}

IVTEntry* IVTEntry::at (unsigned char index)
{
    return objects[index];
}