#include "serv_cli_fifo.h"
#include "handlers_serv.h"

/* Définition des variables globales */
volatile sig_atomic_t reveil = 0;
volatile sig_atomic_t arret_serveur = 0;

int main() {
    /* Déclarations */
    int fd1, fd2;              // Descripteurs des tubes
    Question question;          // Structure pour recevoir la question
    Reponse reponse;           // Structure pour envoyer la réponse
    int nb_aleatoire;          // Nombre aléatoire généré
    struct sigaction action_reveil, action_fin;
    
    // Bloquer les interruptions clavier (optionnel)
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    printf("[SERVEUR] Démarrage du serveur (PID: %d)\n", getpid());
    
    /* Création des tubes nommés */
    printf("[SERVEUR] Création des tubes nommés...\n");
    
    // Suppression des tubes s'ils existent déjà
    unlink(FIFO1);
    unlink(FIFO2);
    
    // Création de fifo1 (client -> serveur)
    if (mkfifo(FIFO1, 0666) == -1) {
        perror("[SERVEUR] Erreur création fifo1");
        exit(1);
    }
    
    // Création de fifo2 (serveur -> client)
    if (mkfifo(FIFO2, 0666) == -1) {
        perror("[SERVEUR] Erreur création fifo2");
        unlink(FIFO1);
        exit(1);
    }
    
    printf("[SERVEUR] Tubes créés avec succès\n");
    
    /* Initialisation du générateur de nombres aléatoires */
    srand(getpid());
    printf("[SERVEUR] Générateur aléatoire initialisé\n");
    
    /* Ouverture des tubes nommés */
    printf("[SERVEUR] Ouverture des tubes...\n");
    
    // Ouverture de fifo1 en lecture (client -> serveur)
    fd1 = open(FIFO1, O_RDONLY);
    if (fd1 == -1) {
        perror("[SERVEUR] Erreur ouverture fifo1");
        unlink(FIFO1);
        unlink(FIFO2);
        exit(1);
    }
    
    // Ouverture de fifo2 en écriture (serveur -> client)
    fd2 = open(FIFO2, O_WRONLY);
    if (fd2 == -1) {
        perror("[SERVEUR] Erreur ouverture fifo2");
        close(fd1);
        unlink(FIFO1);
        unlink(FIFO2);
        exit(1);
    }
    
    printf("[SERVEUR] Tubes ouverts avec succès\n");
    
    /* Installation des Handlers */
    printf("[SERVEUR] Installation des handlers...\n");
    
    // Handler pour SIGUSR1 (réveil du serveur)
    action_reveil.sa_handler = hand_reveil;
    sigemptyset(&action_reveil.sa_mask);
    action_reveil.sa_flags = 0;
    
    if (sigaction(SIGUSR1, &action_reveil, NULL) == -1) {
        perror("[SERVEUR] Erreur installation handler SIGUSR1");
        close(fd1);
        close(fd2);
        unlink(FIFO1);
        unlink(FIFO2);
        exit(1);
    }
    
    // // Handler pour SIGINT (Ctrl+C) pour terminer proprement
    // action_fin.sa_handler = fin_serveur;
    // sigemptyset(&action_fin.sa_mask);
    // action_fin.sa_flags = 0;
    
    // if (sigaction(SIGINT, &action_fin, NULL) == -1) {
    //     perror("[SERVEUR] Erreur installation handler SIGINT");
    //     close(fd1);
    //     close(fd2);
    //     unlink(FIFO1);
    //     unlink(FIFO2);
    //     exit(1);
    // }
    
    printf("[SERVEUR] Handlers installés avec succès\n");
    printf("[SERVEUR] En attente de requêtes...\n");
    
    while(!arret_serveur) {
        /* Lecture d'une question */
        reveil = 0;  // Réinitialisation du flag
        
        ssize_t bytes_read = read(fd1, &question, sizeof(Question));
        
        if (bytes_read > 0) {
            printf("[SERVEUR] Question reçue du client (PID: %d, n=%d)\n", 
                   question.pid_client, question.n);
            
            /* Construction de la réponse */
            // Vérification que n est dans les limites
            if (question.n < 0 || question.n > NMAX) {
                printf("[SERVEUR] Erreur: n hors limites (n=%d)\n", question.n);
                continue;
            }
            
            // Si n == 0, le client veut arrêter
            if (question.n == 0) {
                printf("[SERVEUR] Client %d demande l'arrêt\n", question.pid_client);
                
                // Envoyer une réponse pour débloquer le client
                reponse.pid_client = question.pid_client;
                reponse.resultat = 0;
                write(fd2, &reponse, sizeof(Reponse));
                
                // Envoyer signal au client pour qu'il puisse lire la réponse
                kill(question.pid_client, SIGUSR1);
                
                // Attendre le signal de confirmation du client
                while (!reveil && !arret_serveur) {
                    pause();
                }
                
                printf("[SERVEUR] Client %d terminé, en attente d'autres clients...\n\n", 
                       question.pid_client);
                continue;
            }
            
            // Génération d'un nombre aléatoire entre 1 et n
            nb_aleatoire = (rand() % question.n) + 1;
            
            reponse.pid_client = question.pid_client;
            reponse.resultat = nb_aleatoire;
            
            printf("[SERVEUR] Nombre aléatoire généré: %d (entre 1 et %d)\n", 
                   nb_aleatoire, question.n);
            
            /* Envoi de la réponse */
            if (write(fd2, &reponse, sizeof(Reponse)) == -1) {
                perror("[SERVEUR] Erreur envoi réponse");
                continue;
            }
            
            printf("[SERVEUR] Réponse envoyée au client %d\n", question.pid_client);
            
            /* Envoi du signal SIGUSR1 au client concerné */
            if (kill(question.pid_client, SIGUSR1) == -1) {
                perror("[SERVEUR] Erreur envoi signal au client");
            } else {
                printf("[SERVEUR] Signal SIGUSR1 envoyé au client %d\n", 
                       question.pid_client);
            }
            
            // Attente du signal du client
            printf("[SERVEUR] Attente du signal du client...\n");
            while (!reveil && !arret_serveur) {
                pause();  // Attente du signal SIGUSR1 du client ou SIGINT
            }
            
            if (arret_serveur) {
                printf("[SERVEUR] Interruption demandée\n");
                break;
            }
            
            printf("[SERVEUR] Signal reçu du client, prêt pour la prochaine requête\n\n");
        } else if (bytes_read == 0) {
            // Le client a fermé sa connexion
            printf("[SERVEUR] Interruption par le client (tube fermé)\n");
            fin_serveur(0);
            // Pas de break, on continue à attendre
        } else {
            // Erreur de lecture
            if (!arret_serveur) {
                perror("[SERVEUR] Erreur lecture");
            }
            break;
        }
        
        // Vérifier si arrêt demandé
        if (arret_serveur) {
            break;
        }
    }
    
    // Nettoyage avant de quitter
    printf("\n[SERVEUR] Nettoyage et fermeture...\n");
    close(fd1);
    close(fd2);
    unlink(FIFO1);
    unlink(FIFO2);
    printf("[SERVEUR] Tubes fermés et supprimés. Au revoir!\n");
    
    return 0;
}