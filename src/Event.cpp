/*
 * Event.cpp
 * 
 * Created on: May 26, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>

#include "Macro.h"
#include "Event.h"
#include "System.h"

Event::Event (IVTNo ivtNo)
{
    mIVTNo = ivtNo;
    SysCallData data;
    data.reqType = RequestType::ECreate;
    data.object = this;
    data.number = ivtNo;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

Event::~Event ()
{
    SysCallData data;
    data.reqType = RequestType::EDestroy;
    data.object = (void*) mIVTNo;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

void Event::wait ()
{
    SysCallData data;
    data.reqType = RequestType::EWait;
    data.object = (void*) mIVTNo;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}

void Event::signal()
{
    SysCallData data;
    data.reqType = RequestType::ESignal;
    data.object = (void*) mIVTNo;
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    sysCall(data);
    asmUnlock();
    #endif
}