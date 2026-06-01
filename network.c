#include "network.h"
#include <winsock2.h>
#include <stdio.h>
#include <string.h>

static int role;
static SOCKET listenSocket = INVALID_SOCKET;
static SOCKET clientSockets[3] = {INVALID_SOCKET, INVALID_SOCKET, INVALID_SOCKET};
static int connectedClients = 0;

bool init_network(int r, const char *host, int port) {
    role = r;

    WSADATA w;
    int wsaErr = WSAStartup(MAKEWORD(2,2), &w);
    if (wsaErr != 0) {
        printf("WSAStartup échoué: %d\n", wsaErr);
        return false;
    }

    if (role == NETWORK_SERVER) {
        SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (mainSocket == INVALID_SOCKET) {
            printf("Création socket échouée: %ld\n", WSAGetLastError());
            return false;
        }

        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(mainSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Bind échoué: %ld\n", WSAGetLastError());
            return false;
        }

        if (listen(mainSocket, 2) == SOCKET_ERROR) {
            printf("Listen échoué: %ld\n", WSAGetLastError());
            return false;
        }

        listenSocket = mainSocket;
        printf("Serveur en attente sur port %d...\n", port);
        connectedClients = 0;

        // Rendre la socket non-bloquante
        u_long mode = 1;
        ioctlsocket(listenSocket, FIONBIO, &mode);
    }
    else {
        // Mode client
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            printf("Création socket échouée: %ld\n", WSAGetLastError());
            return false;
        }

        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(host);

        if (connect(clientSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Connexion échouée: %ld\n", WSAGetLastError());
            return false;
        }

        clientSockets[0] = clientSocket;
        printf("Connecté au serveur %s:%d\n", host, port);

        // Rendre la socket non-bloquante
        u_long mode = 1;
        ioctlsocket(clientSocket, FIONBIO, &mode);
    }

    return true;
}

void network_poll(bool *quit, GameState *g, bool up, bool down, bool left, bool right, bool shoot) {
    // Mode serveur : accepter les connexions
    if (role == NETWORK_SERVER && listenSocket != INVALID_SOCKET) {
        struct sockaddr_in addr;
        int addrLen = sizeof(addr);
        SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&addr, &addrLen);

        // Calcul du nombre de clients attendus (total joueurs - 1 pour le serveur)
        int expectedClients = g->player_count - 1;

        if (clientSocket != INVALID_SOCKET && connectedClients < expectedClients) {
            clientSockets[1 + connectedClients] = clientSocket;
            connectedClients++;
            printf("Client connecté! (%d/%d)\n", connectedClients, expectedClients);
            
            // Rendre la socket non-bloquante
            u_long mode = 1;
            ioctlsocket(clientSocket, FIONBIO, &mode);
        }
    }

    // Serveur envoie et reçoit pour tous les joueurs
    if (role == NETWORK_SERVER) {
        int expectedClients = g->player_count - 1;
        if (connectedClients < expectedClients) return;  // Attendre tous les clients

        // Envoyer et recevoir pour chaque client
        for (int i = 1; i < g->player_count; i++) {
            if (clientSockets[i] == INVALID_SOCKET) continue;

            // Envoyer en UN SEUL paquet : tous les joueurs + serpent
            char buffer[sizeof(Player) * 3 + sizeof(Snake)] = {0};
            int offset = 0;
            
            // Copier tous les joueurs
            for (int j = 0; j < g->player_count; j++) {
                memcpy(buffer + offset, &g->players[j], sizeof(Player));
                offset += sizeof(Player);
            }
            
            // Copier le serpent
            memcpy(buffer + offset, &g->snake, sizeof(Snake));
            offset += sizeof(Snake);
            
            send(clientSockets[i], buffer, offset, 0);

            // Recevoir la position du client i
            char recvBuffer[sizeof(Player)] = {0};
            int recvBytes = recv(clientSockets[i], recvBuffer, sizeof(Player), 0);
            if (recvBytes == sizeof(Player)) {
                memcpy(&g->players[i], recvBuffer, sizeof(Player));
            }
        }
    }
    else {
        // Mode client
        if (clientSockets[0] == INVALID_SOCKET) return;

        // Envoyer sa position
        send(clientSockets[0], (char*)&g->players[g->local_id], sizeof(Player), 0);

        // Recevoir EN UN SEUL paquet : tous les joueurs + serpent
        char buffer[sizeof(Player) * 3 + sizeof(Snake)] = {0};
        int totalSize = sizeof(Player) * g->player_count + sizeof(Snake);
        int recvBytes = recv(clientSockets[0], buffer, totalSize, 0);
        
        if (recvBytes == totalSize) {
            int offset = 0;
            
            // Décoder les joueurs (SAUF le joueur local)
            for (int j = 0; j < g->player_count; j++) {
                if (j != g->local_id) {  // Ne pas écraser sa propre position
                    memcpy(&g->players[j], buffer + offset, sizeof(Player));
                }
                offset += sizeof(Player);
            }
            
            // Décoder le serpent
            memcpy(&g->snake, buffer + offset, sizeof(Snake));
        }
    }
}

void shutdown_network() {
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    for (int i = 0; i < 3; i++) {
        if (clientSockets[i] != INVALID_SOCKET) {
            closesocket(clientSockets[i]);
        }
    }
    WSACleanup();
}
