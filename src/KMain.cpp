/*
 * KMain.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <stdio.h>

#include "Thread.h"
#include "Semaphor.h"
#include "System.h"

// extern int userMain (int argc, char* argv[]);

Semaphore *mutex;

class TestThread : public Thread
{
public:
    TestThread (char charToPrint, Time timeSlice = defaultTimeSlice, Time timeToSleep = 0) :
        Thread(defaultStackSize, timeSlice), mChar(charToPrint), mTimeToSleep(timeToSleep) {}
    virtual ~TestThread () { waitToComplete(); }
protected:
    virtual void run ()
    {
        if (mTimeToSleep > 0)
        {
            printf("Thread %c going to sleep for %d ticks!\n", mChar, mTimeToSleep);
            Thread::sleep(mTimeToSleep);
        }
        for (int i = 0; i < 30; ++i)
        {
            printf("Thread %c i = %d\n", mChar, i);
            if (i == 6 || i == 17) mutex->wait(1);
            if (i == 13 || i == 25) mutex->signal();
            for (int j = 0; j < 30000; ++j)
                for (int k = 0; k < 30000; ++k);
        }
        printf("Thread %c done!\n", mChar);
    }
private:
    char mChar;
    Time mTimeToSleep;
};

int userMain (int argc, char* argv[])
{
    int i;

    // TestThread *t[8];
    // t[0] = new TestThread('A', defaultTimeSlice, 17);
    // t[1] = new TestThread('B', defaultTimeSlice, 19);
    // t[2] = new TestThread('C', defaultTimeSlice, 15);
    // t[3] = new TestThread('D', defaultTimeSlice, 17);
    // t[4] = new TestThread('E', defaultTimeSlice, 22);
    // t[5] = new TestThread('F', defaultTimeSlice, 30);
    // t[6] = new TestThread('G', defaultTimeSlice, 25);
    // t[7] = new TestThread('H', defaultTimeSlice, 17);
    // for (i = 0; i < 8; ++i) t[i]->start();
    // for (i = 0; i < 8; ++i) t[i]->waitToComplete();
    
    TestThread a('A', 0), b('B', 20);
    a.start();
    b.start();

    mutex = new Semaphore(0);

    for (i = 0; i < 30; ++i)
    {
        printf("User main i = %d\n", i);
        if (i == 7 || i == 19) mutex->signal();
        for (int j = 0; j < 30000; ++j)
            for (int k = 0; k < 30000; ++k);
    }

    printf("Putting main to sleep for 50 ticks!\n");
    Thread::sleep(50);
    printf("Main woke up! Waiting for children to complete!\n");
    
    a.waitToComplete();
    b.waitToComplete();

    printf("User main done!\n");

    // for (i = 0; i < 8; ++i) delete t[i];
    delete mutex;

    return 0;
}

void tick ()
{
    // printf("*");
}

int main (int argc, char* argv[])
{
    System::initialize();
    printf("===============================================================================\n");
    printf("|--------------------------| Simple kernel project |--------------------------|\n");
    printf("===============================================================================\n");
    printf("> Starting function userMain!\n");
    int result = userMain(argc, argv);
    printf("> Function userMain returned: %d\n", result);
    printf("===============================================================================\n");
    printf("|-------------------------| Jovan Nikolov 2016/0040 |-------------------------|\n");
    printf("===============================================================================\n");
    System::finalize();
    return result;
}