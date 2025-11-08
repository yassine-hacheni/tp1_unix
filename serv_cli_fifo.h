#ifndef SERV_CLI_FIFO_H
#define SERV_CLI_FIFO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>

/* Constantes */
#define NMAX 100
#define FIFO1 "/tmp/fifo1"
#define FIFO2 "/tmp/fifo2"

/* Structure pour la question */
typedef struct {
    pid_t pid_client;  /* PID du client pour identification */
    int n;             /* Le nombre n envoyé par le client */
} Question;

/* Structure pour la réponse */
typedef struct {
    pid_t pid_client;  /* PID du client destinataire */
    int resultat;      /* Le nombre aléatoire entre 1 et n */
} Reponse;

#endif /* SERV_CLI_FIFO_H */