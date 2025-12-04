#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include "utils.h"

void deleteSharedMemory(int shmid) {
    // per ogni errore o la chiusura del programma bisogna togliere le risorse allocate precedentemente
    if (shmctl(shmid, IPC_RMID, NULL) == -1) 
    {
        perror("richiesta della cancellazione della memoria condivisa fallita.");
    }
    else
    {
        printf("memoria rimossa con successo.");
    }
}
void createKeys(key_t* key_req, key_t* key_res,  key_t* key_mem)
{
    // creo una chiave per ogni coda e per la memoria su cui comunicherrano il server e il client
    *key_req = ftok(KEY_PATH, ID_MSG_REQ);
    *key_res = ftok(KEY_PATH, ID_MSG_RES);
    *key_mem = ftok(KEY_PATH, ID_SHM);
    if (*key_req == -1 || *key_res == -1 || *key_mem == -1) 
    {
        perror("creazioni chiavi fallita.");
        exit(1);
    }
}
void createMessageQueue(key_t* key, int* id)
{
    // qui sto effetivamente sto creando la coda dei messaggi
    *id = msgget(*key, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (*id == -1) 
    {
        perror("La creazione di coda di messaggi fallita.");
        exit(1);
    }
}
char* createAndGetMemoryAddress(key_t* key, int* id)
{
    char* addr;
    // creo la memoria condivisa
    *id = shmget(*key, SHM_SIZE, IPC_CREAT |  S_IRUSR | S_IWUSR);
    if (*id == -1) 
    {
        perror("shmget fallita");
        exit(1);
    }

    // prendo l'indirizzo di dove è stata creata lo spazio 
    addr = (char *)shmat(*id, NULL, 0);
    if (addr == (char *)-1) 
    {
        perror("shmat fallita");
        deleteSharedMemory(*id);
        exit(1);
    }
    return addr;
}