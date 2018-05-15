#include <stdio.h>

#include "Thread.h"

PCB **first, *idle;
volatile PCB *running;

int main()
{
    printf("Happy ending!\n");
    return 0;
}