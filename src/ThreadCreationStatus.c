#include <string.h>
#include <stdio.h>
#include "ThreadCreationStatus.h"

int checkThreadCreationStatus(int thread_creation_status)
{
    if(thread_creation_status != 0)
    {
        printf("An error ocurred while creating thread: %s\r\n", strerror(thread_creation_status));
        return -1;
    }

    return 0;
}

int threadAttributesCreationStatus(int attr_creation_status)
{
    if(attr_creation_status)
    {
        printf("An error ocurred while creating thread attributes variable: %s\r\n", strerror(attr_creation_status));
        return -1;
    }

    return 0;
}
