#ifndef HANDLERS_CLI_H
#define HANDLERS_CLI_H

#include <signal.h>
#include <stdio.h>

/* Variables globales - DÉCLARATION EXTERNE */
extern volatile sig_atomic_t reveil;
extern volatile sig_atomic_t arret_client;

/* Handler pour le signal SIGUSR1
 * Ce handler est appelé lorsque le client reçoit SIGUSR1 du serveur
 * Il permet de réveiller le client pour qu'il puisse lire la réponse
 */
void hand_reveil() {
    reveil = 1;
}

/* Handler pour l'interruption (Ctrl+C)
 * Permet d'arrêter proprement le client
 */
void fin_client(int sig) {
    printf("\n[CLIENT] Interruption demandée (Ctrl+C)\n");
    arret_client = 1;
}

#endif /* HANDLERS_CLI_H */