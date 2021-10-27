/*
 * @Author: CALM.WU 
 * @Date: 2021-10-27 16:45:19 
 * @Last Modified by: CALM.WU
 * @Last Modified time: 2021-10-27 16:48:27
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
void *mythread(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) {
        printf("thread is running\n");
    }
    printf("thread is not running\n");
    sleep(2);
}

int main()
{
    pthread_t t1;
    int       err;
    err = pthread_create(&t1, NULL, mythread, NULL);
    printf("cancel t1 thread before\n");
    pthread_cancel(t1);
    printf("cancel t1 thread after\n");
    pthread_join(t1, NULL);
    printf("Main thread is exit\n");
    return 0;
}