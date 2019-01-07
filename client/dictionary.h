#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "structs.h"
#include "logger.h"

int InitDictionary();
void RemoveDomainRecordFromDictionary(int workerId, char *domain);
int AddDomainRecordToDictionary(char *domain);
int FindDomainInDictionary(char *domain);
void FreeDictionary();


#endif // DICTIONARY_H