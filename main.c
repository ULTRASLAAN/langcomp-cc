#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "game.h"
#include "network.h"

// ===== VARIABLES =====

GameState gameState;
int gameStarted = 0;
int networkMode = 0;  // 0=none, 1=server, 2=client1, 3=client2
int totalPlayers = 2;  // 2 ou 3 joueurs
// ===== INIT LOBBY =====

void initLobby() {
    init_game(&gameState, 3, 0);  // 3 joueurs, défaut = joueur 0
}

// ===== INIT GAME =====

void initGame() {
    srand(time(NULL));

    // Joueurs positionnés selon le nombre
    gameState.players[0].x = (gameState.player_count == 2) ? 250 : 150; 
    gameState.players[0].y = 300;
    gameState.players[0].hp = 100;
    gameState.players[0].active = true;

    gameState.players[1].x = 400; 
    gameState.players[1].y = 300;
    gameState.players[1].hp = 100;
    gameState.players[1].active = true;

    // Serpent segmenté
    gameState.snake.length = 10;

    // Directions possibles (8 directions)
    int dirs[8][2] = {
        {10, 0}, {-10, 0}, {0, 10}, {0, -10},
        {10, 10}, {10, -10}, {-10, 10}, {-10, -10}
    };

    int r = rand() % 8;
    gameState.snake.dx = dirs[r][0];
    gameState.snake.dy = dirs[r][1];

    for (int i = 0; i < gameState.snake.length; i++) {
        gameState.snake.body[i].x = 400 - i * gameState.snake.dx;
        gameState.snake.body[i].y = 200 - i * gameState.snake.dy;
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
                    // SERVEUR - Jouer à 2 joueurs
                    networkMode = 1;
                    totalPlayers = 2;
                    gameState.player_count = 2;
                    gameState.local_id = 0;
                    init_network(NETWORK_SERVER, NULL, 5555);
                    printf("✓ Mode SERVEUR 2 JOUEURS - En attente d'1 client...\n");
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                else if (wParam == '2') {
                    // CLIENT 1 (192.168.1.30)
                    char serverIP[50] = "192.168.1.10";
                    networkMode = 2;
                    gameState.local_id = 1;
                    gameState.player_count = 2;
                    printf("✓ Mode CLIENT - Connexion à %s:5555...\n", serverIP);
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
            }

            return 0;

        // ===== TIMER =====
        case WM_TIMER:
            if (gameStarted) {

                // Synchroniser réseau
                if (networkMode) {
                    network_poll(NULL, &gameState, 0, 0, 0, 0, 0);
                }

                // SEUL LE SERVEUR met à jour le serpent
                if (networkMode == 1 || networkMode == 0) {
                    // Déplacement du serpent segmenté
                    for (int i = gameState.snake.length - 1; i > 0; i--) {
                        gameState.snake.body[i] = gameState.snake.body[i - 1];
                    }

                    gameState.snake.body[0].x += gameState.snake.dx;
                    gameState.snake.body[0].y += gameState.snake.dy;

                    // Rebonds + croissance
                    if (gameState.snake.body[0].x < 50 || gameState.snake.body[0].x > 750) {
                        gameState.snake.dx = -gameState.snake.dx;
                        if (gameState.snake.length < MAX_SNAKE_SEGMENTS) gameState.snake.length++;
                    }
                    if (gameState.snake.body[0].y < 50 || gameState.snake.body[0].y > 550) {
                        gameState.snake.dy = -gameState.snake.dy;
                        if (gameState.snake.length < MAX_SNAKE_SEGMENTS) gameState.snake.length++;
                    }

                    // Changement aléatoire de direction (serpent vivant)
                    if (rand() % 40 == 0) {
                        int dirs[8][2] = {
                            {10, 0}, {-10, 0}, {0, 10}, {0, -10},
                            {10, 10}, {10, -10}, {-10, 10}, {-10, -10}
                        };
                        int r = rand() % 8;
                        gameState.snake.dx = dirs[r][0];
                        gameState.snake.dy = dirs[r][1];
                    }
                }

                // Collision serpent → joueurs
                for (int p = 0; p < gameState.player_count; p++) {
                    if (!gameState.players[p].active) continue;

                    int dx = gameState.players[p].x - gameState.snake.body[0].x;
                    int dy = gameState.players[p].y - gameState.snake.body[0].y;
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

                for (int i = 0; i < gameState.snake.length; i++) {
                    Rectangle(hdc,
                        gameState.snake.body[i].x - 5, gameState.snake.body[i].y - 5,
                        gameState.snake.body[i].x + 5, gameState.snake.body[i].y + 5
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
                TextOut(hdc, 20, 70, "[1] - SERVEUR (vous contrôlez le joueur de gauche)", 50);
                TextOut(hdc, 20, 120, "[2] - CLIENT (192.168.1.30 - vous contrôlez le joueur de droite)", 64);
            }
            else if (networkMode == 1) {
                // Mode serveur
                TextOut(hdc, 20, 20, "=== SERVEUR (192.168.1.10) ===", 31);
                TextOut(hdc, 20, 70, "En attente du CLIENT (192.168.1.30)...", 38);
                TextOut(hdc, 20, 230, "[ENTREE] : Lancer la partie", 26);
            }
            else if (networkMode == 2) {
                // Mode client
                TextOut(hdc, 20, 20, "=== CLIENT (192.168.1.30) ===", 29);
                TextOut(hdc, 20, 70, "Connecté au serveur 192.168.1.10!", 34);
                TextOut(hdc, 20, 230, "[ENTREE] : Lancer la partie", 26);
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
