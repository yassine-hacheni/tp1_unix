#ifndef HANDLERS_SERV_H
#define HANDLERS_SERV_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/* Variable globale pour la synchronisation */
extern volatile sig_atomic_t reveil;
extern volatile sig_atomic_t arret_serveur;

/* Handler pour le signal SIGUSR1
 * Ce handler est appelé lorsque le serveur reçoit SIGUSR1 du client
 * Il permet de réveiller le serveur après qu'il a envoyé une réponse
 */
void hand_reveil(int sig) {
    reveil = 1;
}

/* Handler pour la fin du serveur
 * Ce handler permet de terminer proprement le serveur
 * Il supprime les tubes nommés et termine le processus
 */
void fin_serveur(int sig) {
    printf("\n[SERVEUR] Réception du signal %d - Arrêt du serveur\n", sig);
    arret_serveur = 1;  // ← Utilise la variable
    /* Suppression des tubes nommés */
    unlink("/tmp/fifo1");
    unlink("/tmp/fifo2");
    
    printf("[SERVEUR] Tubes supprimés. Au revoir!\n");
    exit(0);
}

#endif /* HANDLERS_SERV_H */