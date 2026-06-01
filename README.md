# 🎮 Jeu Serpent Multijoueur en Réseau

## 📋 Vue d'ensemble

C'est un jeu simple en C avec interface Windows GDI où **2 joueurs jouent ensemble via Ethernet** :
- **Joueur 1** : Exécute le **serveur** (`jeu_serveur.exe`)
- **Joueur 2** : Exécute le **client** (`jeu_client.exe`)

Les deux joueurs voient le même serpent qui grandit et doivent éviter d'entrer en collision avec lui.

---

## 🚀 Démarrage rapide

### PC Serveur (Ce PC - 192.168.X.X)
```bash
# Afficher l'adresse IP du serveur
afficher_ip.bat

# Lancer le serveur
lancer_serveur.bat
```

### PC Client (Autre PC)
```bash
# D'abord, modifiez main_client.c avec l'IP du serveur
# Ligne 24 : char serverIP[50] = "192.168.X.X";

# Recompiler le client
gcc -o jeu_client.exe main_client.c game.c network.c -lws2_32 -lgdi32 -I.

# Lancer le client
lancer_client.bat
```

---

## 🎯 Commandes de Jeu

| Action | Touche |
|--------|--------|
| Haut | Z |
| Bas | S |
| Gauche | Q |
| Droite | D |

---

## 📁 Fichiers du Projet

```
.
├── main.c                  # Serveur principal
├── main_client.c           # Client principal
├── game.h / game.c         # Logique de jeu
├── network.h / network.c   # Couche réseau (Winsock2)
├── jeu_serveur.exe         # Exécutable serveur compilé
├── jeu_client.exe          # Exécutable client compilé
├── lancer_serveur.bat      # Script pour lancer le serveur
├── lancer_client.bat       # Script pour lancer le client
├── afficher_ip.bat         # Affiche l'IP du serveur
├── compile.bat             # Recompile les deux exécutables
└── GUIDE_RESEAU.txt        # Guide détaillé
```

---

## 🔧 Architecture Technique

### Structs de Base
- **Player** : Position (x, y), vie (hp), état (active)
- **GameState** : 2 joueurs, numéro du joueur local (local_id)
- **Snake** : Corps segmenté, direction, longueur

### Communication Réseau
- **Port** : 5555
- **Protocole** : TCP/IP (Winsock2)
- **Synchronisation** : Tous les ~16ms (60 FPS)

```
Serveur (écoute)  :  listen() → accept() → socket active
Client (connecte) :  connect() → socket etablie

Chaque frame :
  - Joueur local envoie sa position
  - Reçoit la position du joueur distant
  - Synchronized snake movement
```

---

## ⚙️ Compilation Manuelle

```bash
# Serveur
gcc -o jeu_serveur.exe main.c game.c network.c -lws2_32 -lgdi32 -I.

# Client
gcc -o jeu_client.exe main_client.c game.c network.c -lws2_32 -lgdi32 -I.
```

**Librairies nécessaires** :
- `-lws2_32` : Winsock2 (réseau)
- `-lgdi32` : GDI (graphics)

---

## 🐛 Dépannage

### La connexion ne fonctionne pas
1. ✓ Vérifiez que les deux PCs sont **connectés par Ethernet**
2. ✓ Vérifiez l'**IP correcte** dans `main_client.c`
3. ✓ Testez : `ping 192.168.X.X` (depuis le client)
4. ✓ Vérifiez le **pare-feu** (port 5555 doit être ouvert)
5. ✓ Le **serveur doit être lancé EN PREMIER**

### Le serveur n'accepte pas le client
- Assurez-vous que `jeu_serveur.exe` affiche "Client connecté"
- Relancez les deux exécutables

### Le serpent n'est pas au même endroit
- Relancez complètement les deux jeux
- Vérifiez la latence réseau

---

## 🎮 Gameplay

**Objectif** :
- Fuyez le serpent vert qui se déplace aléatoirement
- Le serpent grandit quand il rebondit sur les murs
- Restez dans la zone délimitée (rectangle blanc)
- Évitez d'être touché par le serpent

**Fin de partie** :
- Quand les deux joueurs sont touchés → Game Over

---

## 🔐 Notes de Sécurité

- Le port 5555 doit être accessible entre les PCs
- Assurez-vous que le pare-feu Windows autorise la connexion
- Le protocole est en TCP non sécurisé (OK pour LAN local)

---

## 📝 Modifications Possibles

Pour adapter le code :

1. **Changer le port** : Modifiez `5555` dans `network.h` et les deux main.c
2. **Changer le nombre de joueurs** : Modifiez `MAX_PLAYERS` dans `game.h`
3. **Ajouter le chat** : Étendez la structure `Player` avec un message
4. **Augmenter la résolution** : Modifiez la taille de la fenêtre dans WinMain

---

## 📚 Ressources

- **Winsock2** : https://docs.microsoft.com/en-us/windows/win32/winsock/
- **GDI** : https://docs.microsoft.com/en-us/windows/win32/gdi/
- **MinGW** : Compilateur C pour Windows

---

Bon jeu ! 🐍🎮
