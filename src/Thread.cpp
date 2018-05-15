#include <stdio.h>

#include "Thread.h"
#include "SCHEDULE.H"

void Thread::start() {}

void Thread::waitToComplete() {}

Thread::~Thread() {}

void Thread::sleep(Time timeToSleep)
{
    printf("timeToSleep: %d\n", timeToSleep);
}

Thread::Thread(StackSize stackSize, Time timeSlice)
{
    printf("stackSize: %d timeSlice: %d\n", stackSize, timeSlice);
}