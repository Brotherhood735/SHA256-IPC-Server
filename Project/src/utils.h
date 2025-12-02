#include <sys/ipc.h>
// Chiave che usano sia il client e il server per generare le chiavi. 
#define KEY_PATH "."
// per la coda di richiesta
#define ID_MSG_REQ 10
// per la coda di risposta
#define ID_MSG_RES 20
// per la memoria condivisa
#define ID_SHM 30

// il valore rappresenta 1MB, che è la dimensione massima consentito per il file condiviso dal client.
#define SHM_SIZE 1048576 // 1 MB (1024 * 1024)

// dimensione della impronta SHA-256
#define HASH_LENGTH 32

// struttura dati del client per la comunicazione con il server
typedef struct {
    long mtype;
    pid_t client_pid;
    char filename[256];
    size_t file_size;
} msg_req;

// struttura dati del server per la comunicazione con il client
typedef struct {
    long mtype;
    unsigned char hash[HASH_LENGTH];
} msg_res;


void deleteSharedMemory(int shmid);
void createKeys(key_t* keyRisposte, key_t* keyRichieste,  key_t* keyMemoria);
void createMessageQueue(key_t* key, int* id);
char* createAndGetMemoryAddress(key_t* key, int* id);