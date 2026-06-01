#include "dgi_wrapper.h"
#include <DGI.h>

bool dgi_init(int w, int h, const char *title) {
    return DGI_InitWindow(w, h, title);
}

void dgi_poll_events(bool *quit, bool *up, bool *down, bool *left, bool *right, bool *shoot) {
    *up = *down = *left = *right = *shoot = false;

    DGI_Event e;
    while (DGI_PollEvent(&e)) {
        if (e.type == DGI_EVENT_QUIT) *quit = true;
        if (e.type == DGI_EVENT_KEYDOWN) {
            if (e.key == DGI_KEY_W) *up = true;
            if (e.key == DGI_KEY_S) *down = true;
            if (e.key == DGI_KEY_A) *left = true;
            if (e.key == DGI_KEY_D) *right = true;
            if (e.key == DGI_KEY_SPACE) *shoot = true;
        }
    }
}

void dgi_clear_screen() {
    DGI_Clear(0,0,0);
}

void dgi_present() {
    DGI_Render();
}

void dgi_cleanup() {
    DGI_CloseWindow();
}
