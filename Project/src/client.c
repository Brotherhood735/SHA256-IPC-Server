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
#include "utils.h"


void hash_to_hex(const unsigned char *hash, char *hex_str);
void sendMessageAndRecieveHash(int msgid_req, int msgid_res, char* shm_addr, msg_req* req, msg_res* res);
void openFile(const char* filename, struct stat* st, int * fd);
void prepareRequestMessageMetaData(msg_req* req, const char* filename, size_t file_size);
void prepareRequestMessagePayload(ssize_t* bytes_read, int fd, char* shm_addr, struct stat* st, const char* filename);

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_file_input>\n", argv[0]);
        exit(1);
    }
    const char *filename = argv[1];

    key_t key_req;
    key_t key_res;
    key_t key_shm;
    
    int msgid_req;
    int msgid_res;
    int shmid;
    
    char *shm_addr;

    struct stat st;
    int fd;
    ssize_t bytes_read;

    // creo le chiavi
    createKeys(&key_res, &key_req, &key_shm);
    //uso le chiavi generate per creare le code in memoria
    createMessageQueue(&key_req, &msgid_req);
    createMessageQueue(&key_res, &msgid_res);

    //occupo in memoria un spazio per la memoria condivisa tra il server e il client
    shm_addr = createAndGetMemoryAddress(&key_shm, &shmid);

    msg_req req;
    msg_res res;
    // apro il file leggo il contenuto e preparo il messaggio
    openFile(filename, &st, &fd);
    prepareRequestMessageMetaData(&req, filename, st.st_size);
    prepareRequestMessagePayload(&bytes_read, fd, shm_addr, &st, filename);

    // invio il messaggio e atttendo una risposta
    sendMessageAndRecieveHash(msgid_req, msgid_res, shm_addr, &req, &res);

    // dal hash ricevuto calcolo la string in esadecimale
    char hex_hash[65];
    hash_to_hex(res.hash, hex_hash);

    printf("File inviato : %s\n", filename);
    printf("Impronta SHA-256 ricevuta: %s\n", hex_hash);

    //chiudo la memoria
    if(shmdt(shm_addr) == -1)
    {
        perror("shmdt fallito.");
        exit(1);
    }
    deleteSharedMemory(shmid);
    return 0;
}

void sendMessageAndRecieveHash(int msgid_req, int msgid_res, char* shm_addr, msg_req* req, msg_res* res)
{
    // invio il messaggio
    if (msgsnd(msgid_req, req, sizeof(msg_req) - sizeof(long), 0) == -1) 
    {
        perror("msgsnd richiesta fallita");
        shmdt(shm_addr);
        exit(1);
    }
    printf("Richiesta SHA-256 inviata al server (PID: %d). In attesa di risposta...\n", req->client_pid);

    // aspetto per una risposta
    if (msgrcv(msgid_res, res, sizeof(msg_res) - sizeof(long), getpid(), 0) == -1) 
    {
        perror("msgrcv risposta fallita");
        shmdt(shm_addr);
        exit(1);
    }
}
void prepareRequestMessageMetaData(msg_req* req, const char* filename, size_t file_size)
{
    // metto solo i meta data che riguardano il file e il client
    req->mtype = 1;
    req->client_pid = getpid();
    strncpy(req->filename, filename, sizeof(req->filename) - 1);
    req->filename[sizeof(req->filename) - 1] = '\0';
    req->file_size = file_size;
}
void prepareRequestMessagePayload(ssize_t* bytes_read, int fd, char* shm_addr, struct stat* st, const char* filename)
{
    //leggo i dati e li metto direttamente nella memoria condivisa.
    *bytes_read = read(fd, shm_addr, st->st_size);
    close(fd);

    if (*bytes_read != st->st_size) 
    {
        perror("Lettura file o copia in SHM fallita");
        shmdt(shm_addr);
        exit(1);
    }
    printf("File '%s' copiato nella memoria condivisa. Dimensione: %zu byte.\n", 
           filename, st->st_size);

    
}
void openFile(const char* filename, struct stat* st, int * fd)
{
    // prendo i dati che poi mi servirrano per il messaggio
    if (stat(filename, st) == -1) 
    {
        perror("stat fallita");
        exit(1);
    }
    if (st->st_size > SHM_SIZE) 
    {
        fprintf(stderr, "Errore: File troppo grande (%zu byte). Max supportato: %d byte.\n", 
                st->st_size, SHM_SIZE);
        exit(1);
    }
    // se tutto vabene apro il file
    // la lettura effetiva del contuneta verrà fatto al momento della
    // preparazione del messaggio
    *fd = open(filename, O_RDONLY);
    if (*fd == -1) 
    {
        perror("open file fallita");
        exit(1);
    }

}

// funzione per convertire l'hash da binario a stringa esadecimale
void hash_to_hex(const unsigned char *hash, char *hex_str) 
{
    for (int i = 0; i < HASH_LENGTH; i++) {
        sprintf(hex_str + (i * 2), "%02x", hash[i]);
    }
    hex_str[64] = '\0';
}