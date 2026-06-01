#include "game.h"

void init_game(GameState *g, int count, int local_id) {
    g->player_count = count;
    g->local_id = local_id;

    for (int i = 0; i < count; i++) {
        g->players[i].x = 200 + i * 80;
        g->players[i].y = 200;
        g->players[i].hp = 100;
        g->players[i].active = true;
        g->players[i].color = 0xFF0000 + i * 0x002200;
    }
}

void update_local_player(GameState *g, bool up, bool down, bool left, bool right, bool shoot) {
    Player *p = &g->players[g->local_id];

    if (up) p->y -= 4;
    if (down) p->y += 4;
    if (left) p->x -= 4;
    if (right) p->x += 4;
}

void update_game(GameState *g) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *b = &g->bullets[i];
        if (!b->active) continue;

        b->x += b->vx;
        b->y += b->vy;

        if (b->x < 0 || b->x > 1024 || b->y < 0 || b->y > 768)
            b->active = false;
    }
}
