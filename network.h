#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include "game.h"

#define NETWORK_SERVER 1
#define NETWORK_CLIENT 2

bool init_network(int role, const char *host, int port, int local_id);
void network_poll(bool *quit, GameState *g, bool up, bool down, bool left, bool right, bool shoot);
void shutdown_network();
int network_get_connected_clients();

#endif
