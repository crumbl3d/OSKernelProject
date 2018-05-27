/*
 * KEvent.h
 * 
 * Created on: May 26, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _KEVENT_H_
#define _KEVENT_H_

// Only for VS Code to stop IntelliSense from crapping out.
#ifdef BCC_BLOCK_IGNORE
#define interrupt
#endif

typedef unsigned char IVTNo;
typedef void interrupt (*InterruptRoutine) (...);

const unsigned NumOfIVTEntries = 256;

class PCB;
class IVTEntry;
class Event;

class KernelEv
{
public:
    // Used for creating kernel events. Parameters:
    //      ivtNo - interrupt vector table entry
    KernelEv (IVTNo ivtNo);
    
    // Used for creating user events. Parameters:
    //      userEv - user events this object is created for
    //      ivtNo - interrupt vector table entry
    KernelEv (Event *userEv, IVTNo ivtNo);

    ~KernelEv ();

    void wait ();
    void callSignal ();

    static KernelEv* at (unsigned char index);
protected:
    void signal ();
private:
    friend class System;

    // Common initialization
    void initialize (Event *userEv, IVTNo ivtNo);

    // Event value
    char mValue;

    // System data: mEvent (pointer to the user event)
    //              mCreator (the thread that created this event)
    //              mIVTNo (unique ID of this kernel event)
    Event *mEvent;
    PCB *mCreator;
    IVTNo mIVTNo;

    // Class data: objects (array of all the created objects of this class)
    static KernelEv *objects[];
};

class IVTEntry
{
public:
    IVTEntry (IVTNo ivtNo, InterruptRoutine routine);
    ~IVTEntry ();

    void callOldRoutine ();
    
    static IVTEntry* at (unsigned char index);
private:
    friend class System;
    
    // System data: mEvent (pointer to the kernel event)
    //              mOldRoutine (pointer to the old interrupt routine)
    //              mIVTNo (unique ID of this kernel event)
    KernelEv *mEvent;
    InterruptRoutine mOldRoutine;
    IVTNo mIVTNo;

    // Class data: objects (array of all the created objects of this class)
    static IVTEntry *objects[];
};

// Generates the interrupt routine for the specified entry.
#define PREPAREENTRY(IVTNO, CALLOLD) \
void interrupt IRoutine##IVTNO (...) \
{ \
    KernelEv::at(IVTNO)->callSignal(); \
    if (CALLOLD) IVTEntry::at(IVTNO)->callOldRoutine(); \
} \
IVTEntry ivtEntry##IVTNO(IVTNO, IRoutine##IVTNO);

#endif /* _KEVENT_H_ */