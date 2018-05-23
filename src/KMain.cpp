/*
 * KMain.cpp
 *
 * Created on: May 14, 2018
 *     Author: Jovan Nikolov 2016/0040
 */

#include <dos.h>
#include <stdio.h>
#include <iostream.h>

#include "Macro.h"
#include "System.h"
#include "Thread.h"

// extern int userMain(int argc, char* argv[]);

class TestThread : public Thread
{
public:
    TestThread(char charToPrint, Time timeSlice = defaultTimeSlice, Time timeToSleep = 0) :
        Thread(defaultStackSize, timeSlice), mChar(charToPrint), mTimeToSleep(timeToSleep) {}
    virtual ~TestThread() { waitToComplete(); }
protected:
    virtual void run()
    {
        if (mTimeToSleep > 0)
        {
            System::lock();
            cout << "Thread " << mChar << " going to sleep for " << mTimeToSleep << " ticks!" << endl;
            System::unlock();
            Thread::sleep(mTimeToSleep);
        }
        for (int i = 0; i < 5; ++i)
        {
            System::lock();
            cout << "Thread " << mChar << " i = " << i << endl;
            System::unlock();
            for (int j = 0; j < 10000; ++j)
                for (int k = 0; k < 10000; ++k);
        }
        System::lock();
        cout << "Thread " << mChar << " done!" << endl;
        System::unlock();
    }
private:
    char mChar;
    Time mTimeToSleep;
};

int userMain(int argc, char* argv[])
{
    int i;

    TestThread *t[8];
    t[0] = new TestThread('A', defaultTimeSlice, 17);
    t[1] = new TestThread('B', defaultTimeSlice, 19);
    t[2] = new TestThread('C', defaultTimeSlice, 15);
    t[3] = new TestThread('D', defaultTimeSlice, 17);
    t[4] = new TestThread('E', defaultTimeSlice, 22);
    t[5] = new TestThread('F', defaultTimeSlice, 30);
    t[6] = new TestThread('G', defaultTimeSlice, 25);
    t[7] = new TestThread('H', defaultTimeSlice, 17);
    for (i = 0; i < 8; ++i) t[i]->start();
    for (i = 0; i < 8; ++i) t[i]->waitToComplete();
    
    // TestThread a('A', 40), b('B', 20);
    // a.start();
    // b.start();

    for (i = 0; i < 5; ++i)
    {
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        #endif
        cout << "User main i = " << i << endl;
        #ifndef BCC_BLOCK_IGNORE
        asmUnlock();
        #endif

        for (int j = 0; j < 30000; ++j)
            for (int k = 0; k < 30000; ++k);
    }

    printf("Putting main to sleep for 50 ticks!\n");
    Thread::sleep(50);
    
    for (i = 0; i < 8; ++i) delete t[i];

    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    #endif
    cout << "User main done!" << endl;
    #ifndef BCC_BLOCK_IGNORE
    asmUnlock();
    #endif

    return 0;
}

void tick()
{
    // printf("*");
}

int result;

int main(int argc, char* argv[])
{
    System::initialize();
    printf("===============================================================================\n");
    printf("|--------------------------| Simple kernel project |--------------------------|\n");
    printf("===============================================================================\n");
    printf("> Starting function userMain!\n");
    result = userMain(argc, argv);
    printf("> Function userMain returned: %d\n", result);
    printf("===============================================================================\n");
    printf("|-------------------------| Jovan Nikolov 2016/0040 |-------------------------|\n");
    printf("===============================================================================\n");
    System::finalize();
    return result;
}