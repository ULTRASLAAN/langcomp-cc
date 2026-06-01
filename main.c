#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "game.h"
#include "network.h"

#define MAX_SEGMENTS 500

// ===== STRUCTURES =====

typedef struct {
    int x, y;
} Segment;

typedef struct {
    Segment body[MAX_SEGMENTS];
    int length;
    int dx, dy;
} Snake;

// ===== VARIABLES =====

GameState gameState;
Snake snake;
int gameStarted = 0;
int networkMode = 0;  // 0=none, 1=server, 2=client

// ===== INIT LOBBY =====

void initLobby() {
    init_game(&gameState, 2, 0);  // 2 joueurs, ce PC = joueur 0
}

// ===== INIT GAME =====

void initGame() {
    srand(time(NULL));

    // Joueurs positionnés
    gameState.players[0].x = 200; 
    gameState.players[0].y = 300;
    gameState.players[0].hp = 100;
    gameState.players[0].active = true;

    gameState.players[1].x = 400; 
    gameState.players[1].y = 300;
    gameState.players[1].hp = 100;
    gameState.players[1].active = true;

    // Serpent segmenté
    snake.length = 10;

    // Directions possibles (8 directions)
    int dirs[8][2] = {
        {10, 0}, {-10, 0}, {0, 10}, {0, -10},
        {10, 10}, {10, -10}, {-10, 10}, {-10, -10}
    };

    int r = rand() % 8;
    snake.dx = dirs[r][0];
    snake.dy = dirs[r][1];

    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = 400 - i * snake.dx;
        snake.body[i].y = 200 - i * snake.dy;
    }
}

// ===== WINDOWPROC =====

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

        // ===== TOUCHES =====
        case WM_KEYDOWN:

            // LOBBY - Touches pour choisir serveur/client
            if (!gameStarted && !networkMode) {
                if (wParam == '1') {
                    // SERVEUR
                    networkMode = 1;
                    gameState.local_id = 0;  // Serveur = joueur 0
                    init_network(NETWORK_SERVER, NULL, 5555);
                    printf("Mode SERVEUR activé - En attente du client...\n");
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                else if (wParam == '2') {
                    // CLIENT - Utilise l'IP du serveur
                    char serverIP[50] = "192.168.1.10";  // À ADAPTER avec votre IP
                    networkMode = 2;
                    gameState.local_id = 1;  // Client = joueur 1
                    printf("Mode CLIENT - Connexion à %s:5555...\n", serverIP);
                    init_network(NETWORK_CLIENT, serverIP, 5555);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }

            // Lancer le jeu quand connecté
            if (networkMode && !gameStarted && wParam == VK_RETURN) {
                gameStarted = 1;
                initGame();
                InvalidateRect(hwnd, NULL, TRUE);
            }

            // JEU : joueur local (ZQSD)
            if (gameStarted) {
                int pid = gameState.local_id;  // ID du joueur local
                switch (wParam) {
                    case 'Z': gameState.players[pid].y -= 10; break;
                    case 'S': gameState.players[pid].y += 10; break;
                    case 'Q': gameState.players[pid].x -= 10; break;
                    case 'D': gameState.players[pid].x += 10; break;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }

            return 0;

        // ===== TIMER =====
        case WM_TIMER:
            if (gameStarted) {

                // Synchroniser réseau
                if (networkMode) {
                    network_poll(NULL, &gameState, 0, 0, 0, 0, 0);
                }

                // Déplacement du serpent segmenté
                for (int i = snake.length - 1; i > 0; i--) {
                    snake.body[i] = snake.body[i - 1];
                }

                snake.body[0].x += snake.dx;
                snake.body[0].y += snake.dy;

                // Rebonds + croissance
                if (snake.body[0].x < 50 || snake.body[0].x > 750) {
                    snake.dx = -snake.dx;
                    if (snake.length < MAX_SEGMENTS) snake.length++;
                }
                if (snake.body[0].y < 50 || snake.body[0].y > 550) {
                    snake.dy = -snake.dy;
                    if (snake.length < MAX_SEGMENTS) snake.length++;
                }

                // Changement aléatoire de direction (serpent vivant)
                if (rand() % 40 == 0) {
                    int dirs[8][2] = {
                        {10, 0}, {-10, 0}, {0, 10}, {0, -10},
                        {10, 10}, {10, -10}, {-10, 10}, {-10, -10}
                    };
                    int r = rand() % 8;
                    snake.dx = dirs[r][0];
                    snake.dy = dirs[r][1];
                }

                // Collision serpent → joueurs
                for (int p = 0; p < gameState.player_count; p++) {
                    if (!gameState.players[p].active) continue;

                    int dx = gameState.players[p].x - snake.body[0].x;
                    int dy = gameState.players[p].y - snake.body[0].y;
                    int dist2 = dx*dx + dy*dy;

                    if (dist2 < 20*20) {
                        gameState.players[p].active = false;
                    }
                }

                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        // ===== DESSIN =====
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // ===== ÉCRAN DE JEU =====
            if (gameStarted) {

                // Fond noir
                HBRUSH fondJeu = CreateSolidBrush(RGB(10, 10, 10));
                FillRect(hdc, &ps.rcPaint, fondJeu);
                DeleteObject(fondJeu);

                // MAP
                HPEN pen = CreatePen(PS_SOLID, 4, RGB(255, 255, 255));
                HGDIOBJ oldPen = SelectObject(hdc, pen);
                Rectangle(hdc, 50, 50, 750, 550);
                SelectObject(hdc, oldPen);
                DeleteObject(pen);

                // Joueurs
                HBRUSH pBrush = CreateSolidBrush(RGB(0, 200, 255));
                HGDIOBJ oldBrush = SelectObject(hdc, pBrush);

                for (int i = 0; i < gameState.player_count; i++) {
                    if (gameState.players[i].active) {
                        Rectangle(hdc,
                            gameState.players[i].x - 10, gameState.players[i].y - 10,
                            gameState.players[i].x + 10, gameState.players[i].y + 10
                        );
                    }
                }

                SelectObject(hdc, oldBrush);
                DeleteObject(pBrush);

                // Serpent segmenté
                HBRUSH sBrush = CreateSolidBrush(RGB(0, 255, 0));
                oldBrush = SelectObject(hdc, sBrush);

                for (int i = 0; i < snake.length; i++) {
                    Rectangle(hdc,
                        snake.body[i].x - 5, snake.body[i].y - 5,
                        snake.body[i].x + 5, snake.body[i].y + 5
                    );
                }

                SelectObject(hdc, oldBrush);
                DeleteObject(sBrush);

                TextOut(hdc, 60, 60, "FUIS LE SERPENT QUI GRANDIT !", 30);

                EndPaint(hwnd, &ps);
                return 0;
            }

            // ===== LOBBY =====
            HBRUSH fond = CreateSolidBrush(RGB(30, 144, 255));
            FillRect(hdc, &ps.rcPaint, fond);
            DeleteObject(fond);

            if (networkMode == 0) {
                // Menu de sélection serveur/client
                TextOut(hdc, 20, 20, "=== CONFIGURATION RESEAU ===", 28);
                TextOut(hdc, 20, 70, "[1] - Etre SERVEUR (ce PC attend le client)", 43);
                TextOut(hdc, 20, 110, "[2] - Etre CLIENT (se connecter au serveur)", 43);
            }
            else if (networkMode == 1) {
                // Mode serveur
                TextOut(hdc, 20, 20, "=== MODE SERVEUR ===", 20);
                TextOut(hdc, 20, 70, "En attente du client sur le port 5555...", 39);
                TextOut(hdc, 20, 110, "Donnez votre IP a l'autre joueur", 32);
                TextOut(hdc, 20, 200, "[ENTREE] : Lancer la partie", 26);
            }
            else if (networkMode == 2) {
                // Mode client
                TextOut(hdc, 20, 20, "=== MODE CLIENT ===", 19);
                TextOut(hdc, 20, 70, "Connecte au serveur!", 20);
                TextOut(hdc, 20, 200, "[ENTREE] : Lancer la partie", 26);
            }

            EndPaint(hwnd, &ps);
        }
        return 0;

        case WM_DESTROY:
            if (networkMode) shutdown_network();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ===== WINMAIN =====

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    const char CLASS_NAME[] = "MaFenetreGDI";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    initLobby();

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Jeu Serpent Multidirectionnel",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    SetTimer(hwnd, 1, 60, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
