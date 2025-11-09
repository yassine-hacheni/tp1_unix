#include "serv_cli_fifo.h"
#include "Handlers_Cli.h"

/* Définition des variables globales */
volatile sig_atomic_t reveil = 0;
volatile sig_atomic_t arret_client = 0;

int main() {
    /* Déclarations */
    int fd1, fd2;              // Descripteurs des tubes
    Question question;          // Structure pour envoyer la question
    Reponse reponse;           // Structure pour recevoir la réponse
    pid_t pid_serveur;         // PID du serveur
    struct sigaction action_reveil, action_fin;
    int n;                     // Nombre saisi par l'utilisateur
    
    // Bloquer les interruptions clavier (optionnel)
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    
    printf("[CLIENT] Démarrage du client (PID: %d)\n", getpid());
    
    /* Ouverture des tubes nommés */
    printf("[CLIENT] Ouverture des tubes...\n");
    
    // Ouverture de fifo1 en écriture (client -> serveur)
    fd1 = open(FIFO1, O_WRONLY);
    if (fd1 == -1) {
        perror("[CLIENT] Erreur ouverture fifo1");
        printf("[CLIENT] Assurez-vous que le serveur est démarré!\n");
        exit(1);
    }
    
    // Ouverture de fifo2 en lecture (serveur -> client)
    fd2 = open(FIFO2, O_RDONLY);
    if (fd2 == -1) {
        perror("[CLIENT] Erreur ouverture fifo2");
        close(fd1);
        exit(1);
    }
    
    printf("[CLIENT] Tubes ouverts avec succès\n");
    
    /* Installation des Handlers */
    printf("[CLIENT] Installation des handlers...\n");
    
    // Handler pour SIGUSR1 (réveil du client)
    action_reveil.sa_handler = hand_reveil;
    sigemptyset(&action_reveil.sa_mask);
    action_reveil.sa_flags = 0;
    
    if (sigaction(SIGUSR1, &action_reveil, NULL) == -1) {
        perror("[CLIENT] Erreur installation handler SIGUSR1");
        close(fd1);
        close(fd2);
        exit(1);
    }
    
    // Handler pour SIGINT (Ctrl+C)
    action_fin.sa_handler = fin_client;
    sigemptyset(&action_fin.sa_mask);
    action_fin.sa_flags = 0;
    
    if (sigaction(SIGINT, &action_fin, NULL) == -1) {
        perror("[CLIENT] Erreur installation handler SIGINT");
        close(fd1);
        close(fd2);
        exit(1);
    }
    
    printf("[CLIENT] Handlers installés avec succès\n\n");
    
    /* Boucle principale pour plusieurs requêtes */
    while(!arret_client) {
        /* Construction et envoi d'une question */
        printf("=== CLIENT - Interaction avec le serveur ===\n");
        printf("Entrez un nombre entre 1 et %d (0 pour quitter): ", NMAX);
        
        if (scanf("%d", &n) != 1) {
            printf("[CLIENT] Erreur de saisie\n");
            // Vider le buffer
            while(getchar() != '\n');
            continue;
        }
        
        // Vérifier si interruption pendant la saisie
        if (arret_client) {
            printf("[CLIENT] Interruption détectée\n");
            break;
        }
        
        // Vérification de la valeur saisie
        if (n < 0 || n > NMAX) {
            printf("[CLIENT] Erreur: le nombre doit être entre 0 et %d\n", NMAX);
            continue;  // Redemander un nombre
        }
        
        // Si n == 0, le client veut quitter
        if (n == 0) {
            printf("[CLIENT] Demande d'arrêt...\n");
        }
        
        // Construction de la question
        question.pid_client = getpid();
        question.n = n;
        
        printf("[CLIENT] Envoi de la question (PID: %d, n=%d)\n", 
               question.pid_client, question.n);
        
        // Envoi de la question au serveur
        if (write(fd1, &question, sizeof(Question)) == -1) {
            perror("[CLIENT] Erreur envoi question");
            break;
        }
        
        printf("[CLIENT] Question envoyée au serveur\n");
        
        /* Attente de la réponse */
        printf("[CLIENT] Attente du signal du serveur...\n");
        
        reveil = 0;  // Réinitialisation du flag
        
        // Attente du signal SIGUSR1 du serveur
        while (!reveil && !arret_client) {
            pause();  // Bloque jusqu'à réception d'un signal
        }
        
        // Vérifier si interruption
        if (arret_client) {
            printf("[CLIENT] Attente annulée par l'utilisateur\n");
            break;
        }
        
        printf("[CLIENT] Signal reçu du serveur!\n");
        
        /* Lecture de la réponse */
        if (read(fd2, &reponse, sizeof(Reponse)) == -1) {
            perror("[CLIENT] Erreur lecture réponse");
            break;
        }
        
        printf("[CLIENT] Réponse reçue du serveur\n");
        
        /* Envoi du signal SIGUSR1 au serveur */
        // Méthode: utiliser pgrep pour trouver le PID du serveur
        FILE *fp = popen("pgrep -n Serveur", "r");
        if (fp != NULL) {
            if (fscanf(fp, "%d", &pid_serveur) == 1) {
                printf("[CLIENT] PID du serveur trouvé: %d\n", pid_serveur);
                
                if (kill(pid_serveur, SIGUSR1) == -1) {
                    perror("[CLIENT] Erreur envoi signal au serveur");
                } else {
                    printf("[CLIENT] Signal SIGUSR1 envoyé au serveur\n");
                }
            } else {
                printf("[CLIENT] Impossible de trouver le PID du serveur\n");
            }
            pclose(fp);
        }
        
        /* Traitement local de la réponse */
        if (n == 0) {
            printf("\n=== FIN ===\n");
            printf("Arrêt du client demandé\n");
            printf("==========\n\n");
            break;  // Sortir de la boucle
        } else {
            printf("\n=== RÉSULTAT ===\n");
            printf("Vous avez demandé un nombre aléatoire entre 1 et %d\n", n);
            printf("Le serveur a généré: %d\n", reponse.resultat);
            printf("================\n\n");
        }
    }
    
    // Fermeture des tubes
    printf("[CLIENT] Fermeture des connexions...\n");
    close(fd1);
    close(fd2);
    
    printf("[CLIENT] Fin du client\n");
    
    return 0;
}