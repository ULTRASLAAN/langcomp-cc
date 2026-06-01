@echo off
REM ===== SERVEUR JEU MULTIJOUEUR =====
REM Lance le serveur en attente du client

cls
echo.
echo  ╔════════════════════════════════════════════╗
echo  ║  SERVEUR - JEU SERPENT MULTIJOUEUR         ║
echo  ║  En attente de connexion du client...       ║
echo  ╚════════════════════════════════════════════╝
echo.
echo  Votre adresse IP (exemple) : 192.168.1.1
echo  Port : 5555
echo.
echo  Donnez cette IP au client pour qu'il se connecte
echo.
echo  Appuyez sur ENTREE dans le jeu pour lancer la partie
echo.
echo ═══════════════════════════════════════════════
echo.

jeu_serveur.exe

pause
