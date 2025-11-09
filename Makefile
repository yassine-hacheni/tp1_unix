# Makefile pour le projet Client/Serveur avec FIFO
# ================================================

# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS =

# Fichiers sources
SERV_SRC = Serveur.c
CLI_SRC = Client.c

# Fichiers headers
HEADERS = serv_cli_fifo.h handlers_serv.h handlers_cli.h

# Fichiers objets
SERV_OBJ = $(SERV_SRC:.c=.o)
CLI_OBJ = $(CLI_SRC:.c=.o)

# Exécutables
SERV_EXEC = Serveur
CLI_EXEC = Client

# Tubes nommés
FIFO1 = /tmp/fifo1
FIFO2 = /tmp/fifo2

# Couleurs pour l'affichage (optionnel)
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
NC = \033[0m # No Color

# ================================================
# Règles principales
# ================================================

# Règle par défaut : compile tout
all: $(SERV_EXEC) $(CLI_EXEC)
	@echo "$(GREEN)✓ Compilation terminée avec succès!$(NC)"
	@echo "$(YELLOW)Utilisez 'make run-serv' pour lancer le serveur$(NC)"
	@echo "$(YELLOW)Utilisez 'make run-cli' pour lancer le client$(NC)"

# Compilation du serveur
$(SERV_EXEC): $(SERV_OBJ)
	@echo "$(GREEN)Linking serveur...$(NC)"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation du client
$(CLI_EXEC): $(CLI_OBJ)
	@echo "$(GREEN)Linking client...$(NC)"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation des fichiers objets serveur
# Compilation des fichiers objets serveur
Serveur.o: Serveur.c serv_cli_fifo.h handlers_serv.h
	@echo "$(GREEN)Compiling Serveur.c...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation des fichiers objets client
# Compilation des fichiers objets client
Client.o: Client.c serv_cli_fifo.h handlers_cli.h
	@echo "$(GREEN)Compiling Client.c...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# ================================================
# Règles d'exécution
# ================================================

# Lancer le serveur
run-serv: $(SERV_EXEC)
	@echo "$(GREEN)Démarrage du serveur...$(NC)"
	./$(SERV_EXEC)

# Lancer le client
run-cli: $(CLI_EXEC)
	@echo "$(GREEN)Démarrage du client...$(NC)"
	./$(CLI_EXEC)

# Lancer le serveur en arrière-plan
run-serv-bg: $(SERV_EXEC)
	@echo "$(GREEN)Démarrage du serveur en arrière-plan...$(NC)"
	./$(SERV_EXEC) &
	@echo "$(YELLOW)PID du serveur: $$!$(NC)"

# Test complet : lance serveur puis client
test: $(SERV_EXEC) $(CLI_EXEC)
	@echo "$(GREEN)Lancement du test complet...$(NC)"
	@echo "$(YELLOW)Démarrage du serveur en arrière-plan...$(NC)"
	@./$(SERV_EXEC) & echo $$! > /tmp/serveur.pid
	@sleep 1
	@echo "$(YELLOW)Démarrage du client...$(NC)"
	@./$(CLI_EXEC) || true
	@echo "$(YELLOW)Arrêt du serveur...$(NC)"
	@kill `cat /tmp/serveur.pid` 2>/dev/null || true
	@rm -f /tmp/serveur.pid

# ================================================
# Règles de nettoyage
# ================================================

# Nettoyer les fichiers objets
clean:
	@echo "$(RED)Nettoyage des fichiers objets...$(NC)"
	rm -f $(SERV_OBJ) $(CLI_OBJ)
	@echo "$(GREEN)✓ Fichiers objets supprimés$(NC)"

# Nettoyer tout (objets + exécutables)
fclean: clean
	@echo "$(RED)Nettoyage des exécutables...$(NC)"
	rm -f $(SERV_EXEC) $(CLI_EXEC)
	@echo "$(GREEN)✓ Exécutables supprimés$(NC)"

# Nettoyer les tubes nommés
clean-fifo:
	@echo "$(RED)Suppression des tubes nommés...$(NC)"
	rm -f $(FIFO1) $(FIFO2)
	@echo "$(GREEN)✓ Tubes supprimés$(NC)"

# Nettoyage complet (tout + tubes)
mrproper: fclean clean-fifo
	@echo "$(GREEN)✓ Nettoyage complet terminé$(NC)"

# ================================================
# Règles de rebuild
# ================================================

# Recompiler tout
re: fclean all

# ================================================
# Règles utilitaires
# ================================================

# Vérifier la syntaxe sans compiler
check:
	@echo "$(YELLOW)Vérification de la syntaxe...$(NC)"
	$(CC) $(CFLAGS) -fsyntax-only $(SERV_SRC)
	$(CC) $(CFLAGS) -fsyntax-only $(CLI_SRC)
	@echo "$(GREEN)✓ Pas d'erreur de syntaxe$(NC)"

# Afficher les informations de compilation
info:
	@echo "$(YELLOW)========================================$(NC)"
	@echo "$(YELLOW)Informations de compilation$(NC)"
	@echo "$(YELLOW)========================================$(NC)"
	@echo "Compilateur      : $(CC)"
	@echo "Options          : $(CFLAGS)"
	@echo "Serveur source   : $(SERV_SRC)"
	@echo "Client source    : $(CLI_SRC)"
	@echo "Headers          : $(HEADERS)"
	@echo "Exécutable serv  : $(SERV_EXEC)"
	@echo "Exécutable cli   : $(CLI_EXEC)"
	@echo "$(YELLOW)========================================$(NC)"

# Afficher l'aide
help:
	@echo "$(YELLOW)========================================$(NC)"
	@echo "$(GREEN)Makefile - Aide$(NC)"
	@echo "$(YELLOW)========================================$(NC)"
	@echo "$(GREEN)Compilation:$(NC)"
	@echo "  make               - Compile tout"
	@echo "  make all           - Compile tout"
	@echo "  make serveur       - Compile seulement le serveur"
	@echo "  make client        - Compile seulement le client"
	@echo ""
	@echo "$(GREEN)Exécution:$(NC)"
	@echo "  make run-serv      - Lance le serveur"
	@echo "  make run-cli       - Lance le client"
	@echo "  make run-serv-bg   - Lance le serveur en arrière-plan"
	@echo "  make test          - Lance un test complet"
	@echo ""
	@echo "$(GREEN)Nettoyage:$(NC)"
	@echo "  make clean         - Supprime les fichiers objets"
	@echo "  make fclean        - Supprime objets + exécutables"
	@echo "  make clean-fifo    - Supprime les tubes nommés"
	@echo "  make mrproper      - Nettoyage complet"
	@echo "  make re            - Recompile tout"
	@echo ""
	@echo "$(GREEN)Utilitaires:$(NC)"
	@echo "  make check         - Vérifie la syntaxe"
	@echo "  make info          - Affiche les informations"
	@echo "  make help          - Affiche cette aide"
	@echo "$(YELLOW)========================================$(NC)"

# ================================================
# Règles spéciales
# ================================================

# Déclarer les règles qui ne sont pas des fichiers
.PHONY: all clean fclean mrproper re run-serv run-cli run-serv-bg test clean-fifo check info help

# Ne pas supprimer les fichiers intermédiaires
.PRECIOUS: %.o