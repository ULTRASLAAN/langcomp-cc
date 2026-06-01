#ifndef LOBBY_H
#define LOBBY_H

typedef struct {
    char name[32];
    int ready;
} LobbyPlayer;

typedef struct {
    LobbyPlayer players[4];
    int count;
} LobbyState;

void lobby_init(LobbyState *l);
void lobby_set_ready(LobbyState *l, int id, int ready);
void lobby_add_player(LobbyState *l, const char *name);

#endif
