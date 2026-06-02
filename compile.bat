@echo off
REM Compilation du jeu réseau dans un seul exécutable

echo ===== COMPILATION JEU =====
gcc -o jeu.exe main.c game.c network.c -lws2_32 -lgdi32 -I.

if %errorlevel% neq 0 (
    echo Erreur lors de la compilation de jeu.exe
    pause
    exit /b 1
)

echo Jeu compilé avec succès!
echo.
echo Executable créé : jeu.exe
echo Lancez jeu.exe sur n'importe quel PC puis choisissez [1] serveur, [2] client 2, [3] client 3.

pause
