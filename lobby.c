#include "lobby.h"
#include <string.h>

void lobby_init(LobbyState *l) {
    l->count = 0;
}

void lobby_add_player(LobbyState *l, const char *name) {
    if (l->count >= 4) return;
    strcpy(l->players[l->count].name, name);
    l->players[l->count].ready = 0;
    l->count++;
}

void lobby_set_ready(LobbyState *l, int id, int ready) {
    if (id < 0 || id >= l->count) return;
    l->players[id].ready = ready;
}
