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
#include "KThread.h"
#include "Thread.h"

// extern int userMain(int argc, char* argv[]);

class TestThread : public Thread
{
public:
    TestThread(char charToPrint, Time timeSlice) :
        Thread(defaultStackSize, timeSlice), mChar(charToPrint) {}
    virtual ~TestThread() { waitToComplete(); }
protected:
    virtual void run()
    {
        for (int i = 0; i < 30; ++i)
        {
            System::lock();
            cout << "Thread " << mChar << " i = " << i << endl;
            System::unlock();
            for (int j = 0; j < 10000; ++j)
                for (int k = 0; k < 30000; ++k);
        }
        System::lock();
        cout << "Thread " << mChar << " done!" << endl;
        System::unlock();
        System::threadStop();
    }
private:
    char mChar;
};

int userMain(int argc, char* argv[])
{
    TestThread t1('A', 40), t2('B', 20);
    t1.start();
    t2.start();

    for (int i = 0; i < 30; ++i)
    {
        #ifndef BCC_BLOCK_IGNORE
        asmLock();
        cout << "User main i = " << i << endl;
        asmUnlock();
        #endif

        for (int j = 0; j < 30000; ++j)
            for (int k = 0; k < 30000; ++k);
    }
    
    #ifndef BCC_BLOCK_IGNORE
    asmLock();
    cout << "User main done!" << endl;
    asmUnlock();
    #endif

    return 0;
}

void tick()
{
    //printf("*");
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