#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SEGMENTS 500

// ===== STRUCTURES =====

typedef struct {
    char name[32];
    int ready;
} LobbyPlayer;

typedef struct {
    int x, y;
    int alive;
} Player;

typedef struct {
    int x, y;
} Segment;

typedef struct {
    Segment body[MAX_SEGMENTS];
    int length;
    int dx, dy;
} Snake;

// ===== VARIABLES =====

LobbyPlayer playersLobby[3];
Player players[3];
Snake snake;

int playerCount = 3;
int gameStarted = 0;

// ===== INIT LOBBY =====

void initLobby() {
    strcpy(playersLobby[0].name, "GLITCH");
    playersLobby[0].ready = 1;

    strcpy(playersLobby[1].name, "PLAYER2");
    playersLobby[1].ready = 1;

    strcpy(playersLobby[2].name, "PLAYER3");
    playersLobby[2].ready = 1;
}

// ===== INIT GAME =====

void initGame() {
    srand(time(NULL));

    // Joueurs
    players[0].x = 200; players[0].y = 300; players[0].alive = 1;
    players[1].x = 400; players[1].y = 300; players[1].alive = 1;
    players[2].x = 600; players[2].y = 300; players[2].alive = 1;

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

            // LOBBY
            if (!gameStarted && wParam == VK_SPACE) {
                playersLobby[0].ready = !playersLobby[0].ready;
                InvalidateRect(hwnd, NULL, TRUE);
            }

            if (!gameStarted && wParam == VK_RETURN) {
                int allReady = 1;
                for (int i = 0; i < playerCount; i++)
                    if (!playersLobby[i].ready) allReady = 0;

                if (allReady) {
                    gameStarted = 1;
                    initGame();
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }

            // JEU : joueur 0 (ZQSD)
            if (gameStarted) {
                switch (wParam) {
                    case 'Z': players[0].y -= 10; break;
                    case 'S': players[0].y += 10; break;
                    case 'Q': players[0].x -= 10; break;
                    case 'D': players[0].x += 10; break;
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }

            return 0;

        // ===== TIMER =====
        case WM_TIMER:
            if (gameStarted) {

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
                for (int p = 0; p < 3; p++) {
                    if (!players[p].alive) continue;

                    int dx = players[p].x - snake.body[0].x;
                    int dy = players[p].y - snake.body[0].y;
                    int dist2 = dx*dx + dy*dy;

                    if (dist2 < 20*20) {
                        players[p].alive = 0;
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

                for (int i = 0; i < 3; i++) {
                    if (players[i].alive) {
                        Rectangle(hdc,
                            players[i].x - 10, players[i].y - 10,
                            players[i].x + 10, players[i].y + 10
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

            TextOut(hdc, 20, 20, "=== LOBBY ===", 13);

            for (int i = 0; i < playerCount; i++) {
                char line[64];
                sprintf(line, "%s   [%s]",
                        playersLobby[i].name,
                        playersLobby[i].ready ? "PRÊT" : "PAS PRÊT");
                TextOut(hdc, 20, 60 + i * 30, line, strlen(line));
            }

            TextOut(hdc, 20, 200, "[ESPACE] : Prêt / Pas prêt (J1)", 32);
            TextOut(hdc, 20, 230, "[ENTRÉE] : Lancer la partie", 30);

            EndPaint(hwnd, &ps);
        }
        return 0;

        case WM_DESTROY:
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
