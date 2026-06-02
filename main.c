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
int gameEnded = 0;  // 0=en cours, 1=terminé
int gameEndTimer = 0;  // Pour animations
int networkMode = 0;  // 0=none, 1=server, 2=client
int totalPlayers = 3;
char serverIP[50] = "192.168.1.10";  // Adresse IP du serveur maître
// ===== INIT LOBBY =====

void initLobby() {
    init_game(&gameState, 3, 0);  // 3 joueurs, défaut = joueur 0
    gameState.player_count = 3;
    gameState.local_id = 0;
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

    gameState.players[2].x = 300;
    gameState.players[2].y = 150;
    gameState.players[2].hp = 100;
    gameState.players[2].active = true;

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

            // Relancer le jeu (F5)
            if (wParam == VK_F5) {
                if (networkMode) shutdown_network();
                gameEnded = 0;
                gameStarted = 0;
                networkMode = 0;
                initLobby();
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }

            // LOBBY - Touches pour choisir serveur/client
            if (!gameStarted && !networkMode) {
                if (wParam == '1') {
                    // SERVEUR - Jouer à 3 joueurs
                    totalPlayers = 3;
                    gameState.player_count = 3;
                    gameState.local_id = 0;
                    if (init_network(NETWORK_SERVER, NULL, 5555, 0)) {
                        networkMode = 1;
                        printf("✓ Mode SERVEUR 3 JOUEURS - En attente de 2 clients...\n");
                    } else {
                        printf("❌ Échec de lancement du serveur\n");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                else if (wParam == '2' || wParam == '3') {
                    int selectedId = (wParam == '2') ? 1 : 2;
                    printf("✓ Tentative connexion au serveur %s:5555 en tant que Joueur %d...\n", serverIP, selectedId + 1);
                    if (init_network(NETWORK_CLIENT, serverIP, 5555, selectedId)) {
                        networkMode = 2;
                        gameState.local_id = selectedId;
                        gameState.player_count = 3;
                        printf("✓ Mode CLIENT - Connecté comme Joueur %d\n", selectedId + 1);
                    } else {
                        printf("❌ Échec de connexion au serveur\n");
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }

            // Lancer le jeu quand connecté
            if (networkMode && !gameStarted && wParam == VK_RETURN) {
                gameStarted = 1;
                gameEnded = 0;
                initGame();
                InvalidateRect(hwnd, NULL, TRUE);
            }

            // JEU : joueur local (ZQSD) - bloquer les contrôles après THE END
            if (gameStarted && !gameEnded) {
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

                // Vérifier si le jeu est terminé (tous les joueurs morts)
                int activePlayers = 0;
                for (int p = 0; p < gameState.player_count; p++) {
                    if (gameState.players[p].active) activePlayers++;
                }
                if (activePlayers == 0 && !gameEnded) {
                    gameEnded = 1;
                    gameEndTimer = 0;
                }
                if (gameEnded) {
                    gameEndTimer++;
                }

                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        // ===== DESSIN =====
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // DOUBLE BUFFERING
            HDC hdcBuffer = CreateCompatibleDC(hdc);
            HBITMAP hbmBuffer = CreateCompatibleBitmap(hdc, 800, 600);
            HBITMAP hbmOld = SelectObject(hdcBuffer, hbmBuffer);

            // ===== ÉCRAN DE FIN DE JEU =====
            if (gameEnded) {
                // Fond noir avec effet
                HBRUSH fondEnd = CreateSolidBrush(RGB(5, 5, 15));
                RECT rcFull = {0, 0, 800, 600};
                FillRect(hdcBuffer, &rcFull, fondEnd);
                DeleteObject(fondEnd);

                // Cercles de pulsation (animation)
                int pulse = (gameEndTimer % 60) * 255 / 60;
                HPEN penPulse = CreatePen(PS_SOLID, 3, RGB(pulse, pulse/2, 255));
                SelectObject(hdcBuffer, penPulse);
                Ellipse(hdcBuffer, 150 + pulse/5, 150 + pulse/5, 650 - pulse/5, 500 - pulse/5);
                DeleteObject(penPulse);

                // "THE END" titre énorme
                HFONT hFont = CreateFont(120, 0, 0, 0, FW_BOLD, 0, 0, 0,
                                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                        CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, "Arial");
                SelectObject(hdcBuffer, hFont);
                SetTextColor(hdcBuffer, RGB(255, 50, 100));
                SetBkMode(hdcBuffer, TRANSPARENT);
                
                // Ombre du texte
                SetTextColor(hdcBuffer, RGB(50, 10, 20));
                TextOut(hdcBuffer, 212, 212, "THE END", 7);
                
                // Texte principal
                SetTextColor(hdcBuffer, RGB(255, 100, 150));
                TextOut(hdcBuffer, 210, 210, "THE END", 7);

                DeleteObject(hFont);

                // Message sous
                HFONT hFont2 = CreateFont(24, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                         CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                                         DEFAULT_PITCH | FF_DONTCARE, "Arial");
                SelectObject(hdcBuffer, hFont2);
                SetTextColor(hdcBuffer, RGB(150, 255, 200));
                TextOut(hdcBuffer, 250, 380, "Le serpent a gagné!", 19);
                TextOut(hdcBuffer, 200, 430, "Appuyez sur F5 pour relancer", 29);
                
                DeleteObject(hFont2);

                // Étoiles décoratives
                for (int i = 0; i < 20; i++) {
                    int sx = 100 + (i * 37) % 600;
                    int sy = 100 + (i * 53) % 400;
                    SetPixel(hdcBuffer, sx, sy, RGB(255, 255, 100));
                    SetPixel(hdcBuffer, sx+1, sy, RGB(255, 255, 100));
                    SetPixel(hdcBuffer, sx, sy+1, RGB(255, 255, 100));
                }
            }
            // ===== ÉCRAN DE JEU =====
            else if (gameStarted) {
                // Fond dégradé moderne bleu-noir
                HBRUSH fondJeu = CreateSolidBrush(RGB(10, 20, 45));
                RECT rcFull = {0, 0, 800, 600};
                FillRect(hdcBuffer, &rcFull, fondJeu);
                DeleteObject(fondJeu);

                // MAP avec bordure néon
                HPEN penOmbre = CreatePen(PS_SOLID, 5, RGB(0, 80, 150));
                SelectObject(hdcBuffer, penOmbre);
                Rectangle(hdcBuffer, 51, 51, 751, 551);
                DeleteObject(penOmbre);

                HPEN penBord = CreatePen(PS_SOLID, 3, RGB(100, 200, 255));
                SelectObject(hdcBuffer, penBord);
                Rectangle(hdcBuffer, 50, 50, 750, 550);
                DeleteObject(penBord);

                // Grille de fond subtile
                HPEN penGrille = CreatePen(PS_SOLID, 1, RGB(30, 60, 100));
                SelectObject(hdcBuffer, penGrille);
                for (int x = 50; x < 750; x += 50) {
                    MoveToEx(hdcBuffer, x, 50, NULL);
                    LineTo(hdcBuffer, x, 550);
                }
                for (int y = 50; y < 550; y += 50) {
                    MoveToEx(hdcBuffer, 50, y, NULL);
                    LineTo(hdcBuffer, 750, y);
                }
                DeleteObject(penGrille);

                // Joueurs avec effets modernes
                for (int i = 0; i < gameState.player_count; i++) {
                    if (gameState.players[i].active) {
                        // Aura de couleur
                        HBRUSH auraColor = CreateSolidBrush(RGB(0, 150, 200));
                        SelectObject(hdcBuffer, auraColor);
                        Ellipse(hdcBuffer,
                            gameState.players[i].x - 18, gameState.players[i].y - 18,
                            gameState.players[i].x + 18, gameState.players[i].y + 18
                        );

                        // Corps du joueur
                        int r = (i == 0) ? 50 : 255;
                        int g = 220;
                        int b = (i == 0) ? 255 : 50;
                        HBRUSH pBrush = CreateSolidBrush(RGB(r, g, b));
                        SelectObject(hdcBuffer, pBrush);
                        Ellipse(hdcBuffer,
                            gameState.players[i].x - 12, gameState.players[i].y - 12,
                            gameState.players[i].x + 12, gameState.players[i].y + 12
                        );

                        // Bordure brillante
                        HPEN pPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
                        SelectObject(hdcBuffer, pPen);
                        Ellipse(hdcBuffer,
                            gameState.players[i].x - 12, gameState.players[i].y - 12,
                            gameState.players[i].x + 12, gameState.players[i].y + 12
                        );

                        DeleteObject(auraColor);
                        DeleteObject(pBrush);
                        DeleteObject(pPen);
                    }
                }

                // Serpent avec dégradé moderne
                for (int i = 0; i < gameState.snake.length; i++) {
                    // Gradient: tête blanche, queue verte foncée
                    int greenVal = 80 + (i * 140 / gameState.snake.length);
                    int redVal = 200 - (i * 180 / gameState.snake.length);
                    if (greenVal > 255) greenVal = 255;
                    if (redVal < 0) redVal = 0;
                    
                    // Aura
                    HBRUSH aura = CreateSolidBrush(RGB(redVal/2, greenVal/2, 50));
                    SelectObject(hdcBuffer, aura);
                    Ellipse(hdcBuffer,
                        gameState.snake.body[i].x - 8, gameState.snake.body[i].y - 8,
                        gameState.snake.body[i].x + 8, gameState.snake.body[i].y + 8
                    );

                    // Segment
                    HBRUSH sBrush = CreateSolidBrush(RGB(redVal, greenVal, 100));
                    SelectObject(hdcBuffer, sBrush);
                    Ellipse(hdcBuffer,
                        gameState.snake.body[i].x - 6, gameState.snake.body[i].y - 6,
                        gameState.snake.body[i].x + 6, gameState.snake.body[i].y + 6
                    );

                    // Bordure
                    HPEN sPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 150));
                    SelectObject(hdcBuffer, sPen);
                    Ellipse(hdcBuffer,
                        gameState.snake.body[i].x - 6, gameState.snake.body[i].y - 6,
                        gameState.snake.body[i].x + 6, gameState.snake.body[i].y + 6
                    );

                    DeleteObject(aura);
                    DeleteObject(sBrush);
                    DeleteObject(sPen);
                }

                // HUD avec infos
                HFONT hFont = CreateFont(16, 0, 0, 0, FW_BOLD, 0, 0, 0,
                                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                        CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, "Arial");
                SelectObject(hdcBuffer, hFont);
                SetTextColor(hdcBuffer, RGB(200, 255, 100));
                SetBkMode(hdcBuffer, TRANSPARENT);
                
                char hud[100];
                sprintf(hud, "Serpent: %d segments", gameState.snake.length);
                TextOut(hdcBuffer, 60, 565, hud, strlen(hud));
                
                DeleteObject(hFont);
            }
            else {
                // ===== LOBBY =====
                // Fond gradient bleu moderne
                HBRUSH fond = CreateSolidBrush(RGB(15, 50, 120));
                RECT rcFull = {0, 0, 800, 600};
                FillRect(hdcBuffer, &rcFull, fond);
                DeleteObject(fond);

                // Titre
                HFONT hFont = CreateFont(60, 0, 0, 0, FW_BOLD, 0, 0, 0,
                                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                        CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE, "Arial");
                SelectObject(hdcBuffer, hFont);
                SetTextColor(hdcBuffer, RGB(100, 255, 200));
                SetBkMode(hdcBuffer, TRANSPARENT);
                TextOut(hdcBuffer, 150, 80, "SERPENT", 7);
                DeleteObject(hFont);

                // Menu
                HFONT hFont2 = CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                         CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
                                         DEFAULT_PITCH | FF_DONTCARE, "Arial");
                SelectObject(hdcBuffer, hFont2);
                SetTextColor(hdcBuffer, RGB(255, 255, 255));

                if (networkMode == 0) {
                    TextOut(hdcBuffer, 200, 200, "[1] SERVEUR", 11);
                    TextOut(hdcBuffer, 200, 260, "[2] CLIENT 2", 12);
                    TextOut(hdcBuffer, 200, 320, "[3] CLIENT 3", 12);
                }
                else if (networkMode == 1) {
                    TextOut(hdcBuffer, 150, 200, "SERVEUR (192.168.1.10)", 22);
                    TextOut(hdcBuffer, 200, 300, "En attente des CLIENTS...", 24);
                    SetTextColor(hdcBuffer, RGB(100, 255, 200));
                    TextOut(hdcBuffer, 200, 450, "[ENTREE] pour jouer", 18);
                }
                else if (networkMode == 2) {
                    char clientMsg[64];
                    sprintf(clientMsg, "CLIENT (Joueur %d) - %s", gameState.local_id + 1, serverIP);
                    TextOut(hdcBuffer, 150, 200, clientMsg, strlen(clientMsg));
                    TextOut(hdcBuffer, 150, 300, "Connecté au serveur!", 20);
                    SetTextColor(hdcBuffer, RGB(100, 255, 200));
                    TextOut(hdcBuffer, 200, 450, "[ENTREE] pour jouer", 18);
                }

                DeleteObject(hFont2);
            }

            // Copier le buffer vers l'écran réel
            BitBlt(hdc, 0, 0, 800, 600, hdcBuffer, 0, 0, SRCCOPY);

            // Nettoyer le buffer
            SelectObject(hdcBuffer, hbmOld);
            DeleteObject(hbmBuffer);
            DeleteDC(hdcBuffer);

            EndPaint(hwnd, &ps);

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
