#include "controller.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//
static struct Controller Manager;
static struct worker_pool Workers;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//
void destroyController();
void watchMailDirLoop();

//==========================//
//      PUBLIC METHODS      //
//==========================//

void InitController() 
{
    printf("Init main thread\n");
    printf("Init workers...");
    int err;
    Workers.pool = (struct worker *)calloc(COUNT_THREADS - 1, sizeof(struct worker *));
    if (Workers.pool == NULL) 
    {
        printf("Fail\n\t Pool don't create.\n Exit");
        exit(-1);
    }

    //  1-main, 1 logger
    Workers.count = COUNT_THREADS - 2;
    if (Workers.count <= 0) {
        Workers.count = 1;
    }

    for (int I = 0; I < Workers.count; I++) 
    {
        Workers.pool[I].tasks = NULL;
        err = sem_init(&Workers.pool[I].lock, 0, 1);
        if (err) 
        {
            printf("Fail to init semaphore of worker %d tasks", I);
            Dispose();
            return;
        }

        err = pthread_create(&Workers.pool[I].thread, NULL, InitWorker, NULL);
        if (err) 
        {
            printf("Fail\n\t Worker %d don't create.", I);
            Dispose();
            return;
        }
        printf("\tWorker %d is running\n", I);
    }

    printf("\nInit workers...Success\n");
    printf("Init FileViewer...");
    err = InitFileViewer();
    if (err != 0)  
    {
        printf("Fail\n\tErr: %d", err);
        Dispose();
    }
    printf("Success\n");
    watchMailDirLoop();
    return;
}

//  Dispose resource
void Dispose()
{
    for (int I = 0; I < Workers.count; I++) 
    {
        pthread_cancel(Workers.pool[I].thread);
    }
    // DisposeFileViewer();

}

struct worker_pool *GetWorkerPool()
{
    return &Workers;
}
//==========================//
//      PRIVATE METHODS     //
//==========================//

void destroyController() 
{
    if (!Workers.pool)
    {
        return 0;
    }

    for (int I = 0; I < COUNT_THREADS - 1; I++)
    {
        // Workers.pool[I].
    }

    free(Workers.pool);

    return 0;
}

void watchMailDirLoop() {

    printf("Start loop to search new files for send\n");
    while (1) 
    {
        printf("loop start\n");
        SearchNewFiles();
        printf("loop ended\n");
        sleep(30);
    }
    return;
}