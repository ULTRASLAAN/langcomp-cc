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

            // DOUBLE BUFFERING - Créer un buffer pour éviter le scintillement
            HDC hdcBuffer = CreateCompatibleDC(hdc);
            HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, 800, 600);
            HBITMAP hbmOld = SelectObject(hdcBuffer, hbmBuffer);

            // ===== ÉCRAN DE JEU =====
            if (gameStarted) {
                // Fond dégradé bleu foncé
                HBRUSH fondJeu = CreateSolidBrush(RGB(15, 25, 50));
                RECT rcFull = {0, 0, 800, 600};
                FillRect(hdcBuffer, &rcFull, fondJeu);
                DeleteObject(fondJeu);

                // MAP avec bordure épaisse et ombre
                HPEN penOmbre = CreatePen(PS_SOLID, 4, RGB(0, 0, 0));
                SelectObject(hdcBuffer, penOmbre);
                Rectangle(hdcBuffer, 52, 52, 752, 552);
                DeleteObject(penOmbre);

                HPEN penBord = CreatePen(PS_SOLID, 4, RGB(100, 200, 255));
                SelectObject(hdcBuffer, penBord);
                Rectangle(hdcBuffer, 50, 50, 750, 550);
                DeleteObject(penBord);

                // Joueurs avec gradient de couleur
                for (int i = 0; i < gameState.player_count; i++) {
                    if (gameState.players[i].active) {
                        // Couleurs différentes par joueur
                        int r = (i == 0) ? 50 : 255;
                        int g = 200;
                        int b = (i == 0) ? 255 : 50;
                        
                        HBRUSH pBrush = CreateSolidBrush(RGB(r, g, b));
                        SelectObject(hdcBuffer, pBrush);
                        
                        // Cercle approximé avec Rectangle arrondi
                        Ellipse(hdcBuffer,
                            gameState.players[i].x - 12, gameState.players[i].y - 12,
                            gameState.players[i].x + 12, gameState.players[i].y + 12
                        );
                        
                        // Bordure plus claire
                        HPEN pPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                        SelectObject(hdcBuffer, pPen);
                        Ellipse(hdcBuffer,
                            gameState.players[i].x - 12, gameState.players[i].y - 12,
                            gameState.players[i].x + 12, gameState.players[i].y + 12
                        );
                        
                        DeleteObject(pBrush);
                        DeleteObject(pPen);
                    }
                }

                // Serpent vert éclatant avec effet
                for (int i = 0; i < gameState.snake.length; i++) {
                    // Gradient de couleur : tête plus claire, queue plus foncée
                    int greenVal = 180 + (i * 75 / gameState.snake.length);
                    if (greenVal > 255) greenVal = 255;
                    
                    HBRUSH sBrush = CreateSolidBrush(RGB(0, greenVal, 50));
                    SelectObject(hdcBuffer, sBrush);
                    
                    Ellipse(hdcBuffer,
                        gameState.snake.body[i].x - 6, gameState.snake.body[i].y - 6,
                        gameState.snake.body[i].x + 6, gameState.snake.body[i].y + 6
                    );
                    
                    // Bordure pour effet 3D
                    HPEN sPen = CreatePen(PS_SOLID, 1, RGB(100, 255, 100));
                    SelectObject(hdcBuffer, sPen);
                    Ellipse(hdcBuffer,
                        gameState.snake.body[i].x - 6, gameState.snake.body[i].y - 6,
                        gameState.snake.body[i].x + 6, gameState.snake.body[i].y + 6
                    );
                    
                    DeleteObject(sBrush);
                    DeleteObject(sPen);
                }

                // Texte avec fond semi-transparent
                SetTextColor(hdcBuffer, RGB(255, 200, 0));
                SetBkMode(hdcBuffer, TRANSPARENT);
                TextOut(hdcBuffer, 65, 65, ">> FUIS LE SERPENT ! <<", 23);

                EndPaint(hwnd, &ps);
            }
            else {
                // ===== LOBBY =====
                HBRUSH fond = CreateSolidBrush(RGB(20, 100, 180));
                RECT rcFull = {0, 0, 800, 600};
                FillRect(hdcBuffer, &rcFull, fond);
                DeleteObject(fond);

                SetTextColor(hdcBuffer, RGB(255, 255, 255));
                SetBkMode(hdcBuffer, TRANSPARENT);

                if (networkMode == 0) {
                    // Menu de sélection
                    TextOut(hdcBuffer, 30, 30, "=== CONFIGURATION RESEAU ===", 28);
                    TextOut(hdcBuffer, 30, 100, "[1] - SERVEUR (192.168.1.10)", 28);
                    TextOut(hdcBuffer, 30, 150, "[2] - CLIENT (192.168.1.30)", 27);
                }
                else if (networkMode == 1) {
                    TextOut(hdcBuffer, 30, 30, "=== SERVEUR (192.168.1.10) ===", 31);
                    TextOut(hdcBuffer, 30, 100, "En attente du CLIENT...", 22);
                    TextOut(hdcBuffer, 30, 300, "[ENTREE] : Lancer la partie", 26);
                }
                else if (networkMode == 2) {
                    TextOut(hdcBuffer, 30, 30, "=== CLIENT (192.168.1.30) ===", 29);
                    TextOut(hdcBuffer, 30, 100, "Connecté au serveur!", 20);
                    TextOut(hdcBuffer, 30, 300, "[ENTREE] : Lancer la partie", 26);
                }

                EndPaint(hwnd, &ps);
            }

            // Copier le buffer vers l'écran réel
            BitBlt(hdc, 0, 0, 800, 600, hdcBuffer, 0, 0, SRCCOPY);

            // Nettoyer le buffer
            SelectObject(hdcBuffer, hbmOld);
            DeleteObject(hbmBuffer);
            DeleteDC(hdcBuffer);

            return 0;
        }

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
