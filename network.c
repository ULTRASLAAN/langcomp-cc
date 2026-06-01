#include "network.h"
#include <winsock2.h>
#include <stdio.h>

static int role;
static SOCKET listenSocket = INVALID_SOCKET;
static SOCKET sock = INVALID_SOCKET;
static int connected = 0;

bool init_network(int r, const char *host, int port) {
    role = r;

    WSADATA w;
    int wsaErr = WSAStartup(MAKEWORD(2,2), &w);
    if (wsaErr != 0) {
        printf("WSAStartup échoué: %d\n", wsaErr);
        return false;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Création socket échouée: %ld\n", WSAGetLastError());
        return false;
    }

    if (role == NETWORK_SERVER) {
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Bind échoué: %ld\n", WSAGetLastError());
            return false;
        }

        if (listen(sock, 1) == SOCKET_ERROR) {
            printf("Listen échoué: %ld\n", WSAGetLastError());
            return false;
        }

        listenSocket = sock;
        printf("Serveur en attente sur port %d...\n", port);
        connected = 0;
    }
    else {
        // Mode client
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(host);

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            printf("Connexion échouée: %ld\n", WSAGetLastError());
            return false;
        }

        printf("Connecté au serveur %s:%d\n", host, port);
        connected = 1;
    }

    // Rendre la socket non-bloquante
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    return true;
}

void network_poll(bool *quit, GameState *g, bool up, bool down, bool left, bool right, bool shoot) {
    // Mode serveur : accepter les connexions
    if (role == NETWORK_SERVER && !connected && listenSocket != INVALID_SOCKET) {
        struct sockaddr_in addr;
        int addrLen = sizeof(addr);
        SOCKET clientSocket = accept(listenSocket, (struct sockaddr*)&addr, &addrLen);

        if (clientSocket != INVALID_SOCKET) {
            sock = clientSocket;
            connected = 1;
            printf("Client connecté!\n");
            
            // Rendre la socket non-bloquante
            u_long mode = 1;
            ioctlsocket(sock, FIONBIO, &mode);
        }
    }

    if (!connected) return;

    // Envoyer l'état du joueur local
    if (send(sock, (char*)&g->players[g->local_id], sizeof(Player), 0) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            printf("Erreur send: %d\n", err);
        }
    }

    // Recevoir l'état des autres joueurs
    for (int i = 0; i < g->player_count; i++) {
        if (i == g->local_id) continue;  // Ne pas recevoir pour soi-même

        int recvBytes = recv(sock, (char*)&g->players[i], sizeof(Player), 0);
        if (recvBytes == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                printf("Erreur recv: %d\n", err);
            }
        }
    }
}

void shutdown_network() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    WSACleanup();
}
