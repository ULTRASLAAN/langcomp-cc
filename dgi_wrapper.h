#ifndef DGI_WRAPPER_H
#define DGI_WRAPPER_H

#include <stdbool.h>

bool dgi_init(int w, int h, const char *title);
void dgi_poll_events(bool *quit, bool *up, bool *down, bool *left, bool *right, bool *shoot);
void dgi_clear_screen();
void dgi_present();
void dgi_cleanup();

#endif
