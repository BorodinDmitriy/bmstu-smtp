#include "dictionary.h"

static struct domain_record *Dictionary;
static sem_t lock;

int InitDictionary() 
{
    Dictionary = NULL;
    int err;
    err = sem_init(&lock, 0, 1);
    if (err != 0)
    {
        Error("Dictionary: initialization fail. Exit...");
        return -1;
    }
    return 0;
}

int AddDomainRecordToDictionary(char *domain)
{
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;

    while (Dictionary && pointer->next != NULL)
    {
        pointer = pointer->next;
    }

    struct domain_record *new_record = (struct domain_record *)malloc(sizeof(struct domain_record));
    if (new_record == NULL)
    {
        Error("Dictionary: fail to create new record for dictionary");
        sem_post(&lock);
        return -1;
    }

    int len = strlen(domain) + 1;
    new_record->domain = (char *)calloc(len, sizeof(char));
    if (new_record->domain == NULL)
    {
        Error("Dictionary: fail to allocate memory for domain\n");
        free(new_record);
        sem_post(&lock);
        return -1;
    }

    memset(new_record->domain, '\0', len);
    strcpy(new_record->domain, domain);
    new_record->workerId = MostFreeWorker();
    new_record->next = NULL;

    if (Dictionary == NULL)
    {    
        Dictionary = new_record;
    } 
    else 
    {
        pointer->next = new_record;
    }

    sem_post(&lock);
    return 0;
}

int FindDomainInDictionary(char *domain)
{
    int result = -1;
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;
    while (pointer != NULL)
    {
        if (strcmp(pointer->domain, domain) == 0)
        {
            result = pointer->workerId;
            break;
        }

        pointer = pointer->next;
    }

    sem_post(&lock);
    return result;
}

void RemoveDomainRecordFromDictionary(char *domain)
{
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;
    struct domain_record *prev = NULL;
    int state = 0;

    while (pointer != NULL)
    {
        state = strcmp(domain, pointer->domain);
        if (state == 0) 
        {
            if (prev != NULL) 
            {
                prev->next = pointer->next;
            }
            else if (pointer->next)
            {
                Dictionary = Dictionary->next;
            }
            else if (pointer == Dictionary) 
            {
                free(Dictionary);
                Dictionary = NULL;
                break;
            }
            free(pointer);
            break;
        }
        
        prev = pointer;
        pointer = pointer->next;
    }

    sem_post(&lock);
    return;
}

void FreeDictionary()
{
    sem_wait(&lock);
    struct domain_record *pointer = Dictionary;
    while (pointer != NULL)
    {
        pointer = pointer->next;
        free(Dictionary);
        Dictionary = pointer;
    }
    sem_post(&lock);
    int err = sem_destroy(&lock);
    if (err != 0)
    {
        Error("Dictionary: fail to destroy semaphore.");
    }
    return;
}