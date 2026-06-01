#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PLAYERS 4
#define MAX_BULLETS 32

typedef struct {
    int x, y;
    int vx, vy;
    int hp;
    bool active;
    uint32_t color;
} Player;

typedef struct {
    int x, y;
    int vx, vy;
    bool active;
} Bullet;

typedef struct {
    Player players[MAX_PLAYERS];
    Bullet bullets[MAX_BULLETS];
    int player_count;
    int local_id;
} GameState;

void init_game(GameState *g, int count, int local_id);
void update_local_player(GameState *g, bool up, bool down, bool left, bool right, bool shoot);
void update_game(GameState *g);

#endif
