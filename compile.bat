@echo off
REM Compilation du jeu en réseau - Serveur et Client

echo ===== COMPILATION SERVEUR =====
gcc -o jeu_serveur.exe main.c game.c network.c -lws2_32 -I.

if %errorlevel% neq 0 (
    echo Erreur lors de la compilation du serveur
    pause
    exit /b 1
)

echo Serveur compilé avec succès!
echo.

echo ===== COMPILATION CLIENT =====
gcc -o jeu_client.exe main_client.c game.c network.c -lws2_32 -I.

if %errorlevel% neq 0 (
    echo Erreur lors de la compilation du client
    pause
    exit /b 1
)

echo Client compilé avec succès!
echo.

echo ===== COMPILATION TERMINÉE =====
echo Exécutables créés:
echo - jeu_serveur.exe (lance sur ce PC)
echo - jeu_client.exe (lance sur l'autre PC)
echo.

echo Avant de lancer le client, modifiez l'adresse IP dans main_client.c (variable serverIP)
echo ou utilisez l'invite de commande: ipconfig /all

pause
