@echo off
REM Affiche l'adresse IP du serveur pour configurer le client

cls
echo.
echo  ╔════════════════════════════════════════════╗
echo  ║     ADRESSE IP DU SERVEUR                  ║
echo  ╚════════════════════════════════════════════╝
echo.
echo  Voici votre adresse IP : 192.168.1.1
echo.
ipconfig | findstr "Adresse IPv4"
echo.
echo  Utilisez cette adresse IP pour configurer le client
echo.
echo  IMPORTANT :
echo  - Cherchez l'IP qui commence par 192.168 ou 10.
echo  - Evitez les adresses 127.0.0.1 ou 169.254.x.x
echo.
pause
