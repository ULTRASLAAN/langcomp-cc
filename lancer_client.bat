@echo off
REM ===== CLIENT JEU MULTIJOUEUR =====
REM Lance le client qui se connecte au serveur

cls
echo.
echo  ╔════════════════════════════════════════════╗
echo  ║  CLIENT - JEU SERPENT MULTIJOUEUR          ║
echo  ║  En attente de connexion au serveur...      ║
echo  ╚════════════════════════════════════════════╝
echo.
echo  Assurez-vous que :
echo  ✓ Le serveur est lance sur l'autre PC
echo  ✓ Vous avez configure l'IP correcte
echo  ✓ Les deux PCs sont connectes par Ethernet
echo.
echo  Pour changer l'IP du serveur :
echo  1. Editez le fichier main_client.c (ligne 24)
echo  2. Mettez a jour serverIP avec l'IP du serveur
echo  3. Recompilez : gcc -o jeu_client.exe main_client.c game.c network.c -lws2_32 -lgdi32 -I.
echo.
echo ═══════════════════════════════════════════════
echo.

jeu_client.exe

pause
