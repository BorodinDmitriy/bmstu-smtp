#include "controller.h"

//==========================//
//    PRIVATE ATTRIBUTES    //
//==========================//
static struct worker_pool Workers;
static pthread_t logger;
static bool worked;

//==========================//
//  DEFINES PRIVATE METHOD  //
//==========================//
int createWorker(int index);
void destroyWorker(int index);

void destroyController();
void watchMailDirLoop();
void signalHandler(int signum);
int initSignalHandler();

//==========================//
//      PUBLIC METHODS      //
//==========================//

void InitController()
{
    printf("Init main thread\n");

    printf("Init logger...");
    int err;
    err = pthread_create(&logger, NULL, InitLogger, NULL);
    if (err) 
    {
        Dispose();
        printf("Fail\n\tFail to init logger. Exit");
        exit(err);
    }
    printf("Success\n");
    printf("Init workers pool...");
    Workers.pool = (struct worker **)calloc(COUNT_THREADS - 1, sizeof(struct worker *));
    if (Workers.pool == NULL)
    {
        printf("Fail\n\t Pool don't create.\n Exit");
        exit(-1);
    }
    printf("Success\n");

    //  1-main, 1 logger
    Workers.count = COUNT_THREADS - 2;
    if (Workers.count <= 0)
    {
        Workers.count = 1;
    }

    printf("Workers: %d\n", Workers.count);
    for (int I = 0; I < Workers.count; I++)
    {
        err = createWorker(I);
        if (err)
        {
            printf("Fail\n\t Worker %d don't create.", I);
            Dispose();
            return;
        }
        printf("\tWorker %d is running\n", I);
    }

    printf("\nInit workers...Success\n");

    printf("Init dictionary...");
    err = InitDictionary();
    if (err != 0)
    {
        printf("Fail\n\tErr: %d", err);
        Dispose();
        return;
    }
    printf("Success\n");

    printf("Init FileViewer...");
    err = InitFileViewer();
    if (err != 0)
    {
        printf("Fail\n\tErr: %d", err);
        Dispose();
        return;
    }
    printf("Success\n");

    printf("Init signal catcher...");
    err = initSignalHandler();
    if (err != 0)
    {
        printf("Fail\n\tErr: %d", err);
    }
    printf("Success\n");
    worked = true;
    sleep(1);
    watchMailDirLoop();
    return;
}

//  Dispose resource
void Dispose()
{
    printf("Disposing.... \n");
    if (!Workers.pool)
    {
        return;
    }

    for (int I = 0; I < Workers.count; I++)
    {
        printf("\tWait worker %d\n", I);
        Workers.pool[I]->worked = false;
        pthread_kill(Workers.pool[I]->thread, SIGUSR1);
        pthread_join(Workers.pool[I]->thread, NULL);
        destroyWorker(I);
        printf("\tWorker %d closing\n", I);
    }

    free(Workers.pool);
    FreeDictionary();

    printf("Success");
    return;
}

struct worker_pool *GetWorkerPool()
{
    return &Workers;
}

int MostFreeWorker()
{
    int min = Workers.pool[0]->count_task;
    int res = 0;
    for (int I = 1; I < Workers.count; I++)
    {
        if (min > Workers.pool[I]->count_task)
        {
            min = Workers.pool[I]->count_task;
            res = I;
        }
    }

    return res;
}

int DelegateTaskToWorker(int workerIndex, struct worker_task *task)
{
    if (workerIndex > Workers.count)
    {
        return -1;
    }

    sem_wait(&Workers.pool[workerIndex]->lock);
    struct worker_task *pointer = Workers.pool[workerIndex]->tasks;
    while (pointer && pointer->next != NULL)
    {
        pointer = pointer->next;
    }

    if (pointer != NULL)
    {
        pointer->next = task;
    }
    else
    {
        Workers.pool[workerIndex]->tasks = task;
    }

    Workers.pool[workerIndex]->count_task++;

    sem_post(&Workers.pool[workerIndex]->lock);
    pthread_kill(Workers.pool[workerIndex]->thread, SIGUSR1);
    return 0;
}

//==========================//
//      PRIVATE METHODS     //
//==========================//

int createWorker(int index)
{
    int state = 0;
    Workers.pool[index] = (struct worker *)malloc(sizeof(struct worker));
    if (!Workers.pool[index])
    {
        printf("\nFail to allocate memory for record of worker");
        return -1;
    }
    Workers.pool[index]->tasks = NULL;
    Workers.pool[index]->workerId = index;
    Workers.pool[index]->count_task = 0;
    Workers.pool[index]->worked = true;
    state = sem_init(&(Workers.pool[index]->lock), 0, 1);
    if (state)
    {
        printf("Fail to init semaphore of worker %d tasks", index);
        return -2;
    }

    state = pthread_create(&Workers.pool[index]->thread, NULL, InitWorker, (void *)Workers.pool[index]);
    if (state)
    {
        printf("Fail to create thread of worker %d\n", index);
        return -3;
    }

    return 0;
}

void destroyWorker(int index)
{
    sem_destroy(&(Workers.pool[index]->lock));
    free(Workers.pool[index]);
    return;
}

void watchMailDirLoop()
{

    printf("Start loop to search new files for send\n");
    while (worked)
    {
        printf("loop start\n");
        SearchNewFiles();
        printf("loop ended\n");
        sleep(30);
    }
    Dispose();
    printf("CANCEL PROGRAMM\n");
    return;
}

int initSignalHandler()
{
    struct sigaction actions;
    memset(&actions, 0, sizeof(actions));
    actions.sa_handler = signalHandler;

    sigemptyset(&actions.sa_mask);
    sigaddset(&actions.sa_mask, SIGINT);
    sigaction(SIGINT, &actions, NULL);
    return 0;
}

void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        printf("\nCatch SIGINT\n");
        worked = false;
    }
    return;
}