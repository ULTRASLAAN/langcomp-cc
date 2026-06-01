#include "render3d.h"
#include <DGI.h>

static int iso_x(int x, int y) { return x - y + 400; }
static int iso_y(int x, int y) { return (x + y) / 2 + 100; }

void render_arena_3d(GameState *g) {
    DGI_SetColor(0x303030);
    DGI_FillRect(0, 0, 1024, 768);
}

void render_players_3d(GameState *g) {
    for (int i = 0; i < g->player_count; i++) {
        Player *p = &g->players[i];
        if (!p->active) continue;

        int sx = iso_x(p->x, p->y);
        int sy = iso_y(p->x, p->y);

        DGI_SetColor(p->color);
        DGI_FillRect(sx, sy, 40, 40);
    }
}

void render_bullets_3d(GameState *g) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *b = &g->bullets[i];
        if (!b->active) continue;

        int sx = iso_x(b->x, b->y);
        int sy = iso_y(b->x, b->y);

        DGI_SetColor(0xFFFFFF);
        DGI_FillRect(sx, sy, 10, 10);
    }
}
