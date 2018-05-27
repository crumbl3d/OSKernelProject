/*
 * Event.h
 * 
 * Created on: May 26, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#ifndef _EVENT_H_
#define _EVENT_H_

#include "KEvent.h"

typedef unsigned char IVTNo;

class KernelEv;

class Event
{
public:
    Event (IVTNo ivtNo);
    ~Event ();

    void wait ();
protected:
    friend class KernelEv;

    void signal ();
private:
    IVTNo mIVTNo;
};

#endif /* _EVENT_H_ */