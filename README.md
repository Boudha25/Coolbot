# Projet CoolBot avec ESP8266

Ce projet utilise un ESP8266 pour contrôler un système de refroidissement en fonction de la température ambiante et de la température de l'évaporateur. L'ESP8266 se connecte à un réseau WiFi ou fonctionne en mode point d'accès pour la configuration. Il utilise un écran OLED pour afficher les informations et un serveur web pour gérer les paramètres.

## Fonctionnalités

- **Affichage OLED** : Montre la température ambiante, la température de l'évaporateur, le point de consigne, et l'adresse IP.
- **Mode de démarrage** : Choisissez entre le mode WiFi et le mode point d'accès (AP) en appuyant sur un bouton.
- **Serveur Web Asynchrone** : Permet de configurer les paramètres du réseau via une page HTML hébergée.
- **Gestion des températures** : Contrôle un relais pour activer ou désactiver un chauffage basé sur les températures mesurées.

## Matériel Nécessaire

- ESP8266 (NodeMCU ou similaire)
- Écran OLED SSD1306
- Capteurs de température Dallas DS18B20
- Relais
- Boutons

## Dépendances

Le code utilise les bibliothèques suivantes :

- `ESPAsyncWebServer` pour le serveur web asynchrone.
- `LittleFS` pour le stockage des fichiers.
- `Adafruit_GFX` et `Adafruit_SSD1306` pour l'affichage sur l'écran OLED.
- `OneWire` et `DallasTemperature` pour la gestion des capteurs de température.

## Installation

1. Installez les bibliothèques nécessaires via le gestionnaire de bibliothèques Arduino IDE ou ajoutez-les manuellement à votre répertoire de bibliothèques.
2. Téléversez le code sur votre ESP8266.
3. Connectez l'ESP8266 à votre réseau WiFi ou configurez-le en mode point d'accès pour initialiser les paramètres du réseau.
4. Installer LittleFS dans votre IDE Arduino pour enregistrer les fichiers du dossier /data dans le Nodemcu.  Suivre cette procédure: https://randomnerdtutorials.com/arduino-ide-2-install-esp8266-littlefs/

## Utilisation

- **Mode WiFi** : Une fois que l'ESP8266 est connecté à votre réseau WiFi, accédez à l'adresse IP de l'ESP8266 via un navigateur web pour configurer les paramètres.
- **Mode Point d'accès** : Connectez-vous au réseau WiFi créé par l'ESP8266 ("Ouellet-Cool") pour accéder à la page de configuration.

## Fonctionnalités du Serveur Web

- `/` : Affiche la page HTML principale.
- `/setpointPlus` : Augmente le point de consigne de 1°C.
- `/setpointMoins` : Diminue le point de consigne de 1°C.

## Notes

- Assurez-vous que l'écran OLED est correctement connecté et que les adresses des capteurs de température sont correctement configurées.
- Le code est conçu pour une utilisation avec des capteurs Dallas DS18B20. Assurez-vous que les capteurs sont correctement connectés au bus OneWire.

## Exemples de Sortie

- Lorsque le bouton de mode est pressé, l'affichage sur l'écran OLED changera pour montrer la température ambiante, la température de l'évaporateur, le point de consigne, ou l'adresse IP.
- Le relais sera activé ou désactivé en fonction de la température ambiante par rapport au point de consigne.
