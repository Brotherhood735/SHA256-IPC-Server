#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include "utils.h"

void generateSHA256(const unsigned char *data, unsigned char *hash, size_t len); 
void readAndRespond(int msgid_req, int msgid_res, char* shm_addr);


int main() {
    // le chiavi per le code e la memoria condivisa
    // le stessi che genererà anche il client
    key_t key_req;
    key_t key_res;
    key_t key_shm;

    // id che identificano le risorse allocate per questo programma (le code e la memoria condivisa) 
    int msgid_req;
    int msgid_res;
    int shmid;
    
    char* shm_addr;

    //creo le chiavi
    createKeys(&key_res, &key_req, &key_shm);
    
    //creo le code di messaggi
    createMessageQueue(&key_req, &msgid_req);
    createMessageQueue(&key_res, &msgid_res);

    //creo la memoria condivisa
    shm_addr = createAndGetMemoryAddress(&key_shm, &shmid);

    printf("in attesa per le risposte.\n");
    while (1) {
        // ciclo principale che elabora i dati ricevuti dal client e invia le risposta
        readAndRespond(msgid_req, msgid_res, shm_addr);
    }

    if(shmdt(shm_addr) == -1)
    {
        perror("shmdt fallito.");
    }
    deleteSharedMemory(shmid);
    return 0;
}

void generateSHA256(const unsigned char *data, unsigned char *hash, size_t len) 
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(hash, &ctx);
}
void readAndRespond(int msgid_req, int msgid_res, char* shm_addr)
{
    msg_req req;
    msg_res res;
    unsigned char hash[HASH_LENGTH];

    // ricevo la risposta
    if (msgrcv(msgid_req, &req, sizeof(msg_req) - sizeof(long), 1, 0) == -1) {
        perror("msgrcv fallita");
        exit(1);
    }

    printf("Richiesta da PID %d con il file '%s' e con dimensioni %zu byte.\n",
            req.client_pid, req.filename, req.file_size);

    // controllo se la dimensione del file ricevuta è corretta.
    if (req.file_size > SHM_SIZE) {
        fprintf(stderr, "errore, la dimensione del file (%zu) supera SHM_SIZE (%d).\n", 
                req.file_size, SHM_SIZE);
        return;
    }
    
    // calcolo l'hash
    generateSHA256((const unsigned char *)shm_addr, hash, req.file_size);

    // metto l'hash generato nel messaggio di risposta
    res.mtype = req.client_pid;
    memcpy(res.hash, hash, HASH_LENGTH);

    // invio la risposta
    if (msgsnd(msgid_res, &res, sizeof(msg_res) - sizeof(long), 0) == -1) {
        perror("msgsnd fallita");
    } else {
        printf("Hash generato e inviato al client con successo.%d.\n", req.client_pid);
    }
}