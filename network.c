#include "network.h"
#include <winsock2.h>
#include <stdio.h>

static int role;
static SOCKET sock;

bool init_network(int r, const char *host, int port) {
    role = r;

    WSADATA w;
    WSAStartup(MAKEWORD(2,2), &w);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (role == NETWORK_SERVER) {
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(sock, (void*)&addr, sizeof(addr));
        listen(sock, 4);

        printf("Serveur en attente...\n");
        sock = accept(sock, NULL, NULL);
        printf("Client connecté\n");
    }
    else {
        struct sockaddr_in addr = {0};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(host);

        connect(sock, (void*)&addr, sizeof(addr));
        printf("Connecté au serveur\n");
    }

    return true;
}

void network_poll(bool *quit, GameState *g, bool up, bool down, bool left, bool right, bool shoot) {
    send(sock, (char*)&g->players[g->local_id], sizeof(Player), 0);

    for (int i = 0; i < g->player_count; i++) {
        recv(sock, (char*)&g->players[i], sizeof(Player), 0);
    }
}

void shutdown_network() {
    closesocket(sock);
    WSACleanup();
}
