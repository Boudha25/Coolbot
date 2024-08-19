#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Définition des dimensions de l'écran OLED.
// Screen dimensions definition for the OLED display.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Initialisation de l'écran OLED.
// OLED screen initialization.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Le fil de données est connecté à la broche D0 (GPIO16) de l'Arduino.
// Data wire is connected to pin D0 (GPIO16) on the Arduino.
#define ONE_WIRE_BUS 16

// Configurer une instance OneWire pour communiquer avec tous les dispositifs OneWire.
// Setup a oneWire instance to communicate with any OneWire devices.
OneWire oneWire(ONE_WIRE_BUS);

// Passer la référence OneWire à Dallas Temperature.
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Créer un objet AsyncWebServer sur le port 80.
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Créer des variables globales pour chaque champ de saisie.
// create global variables for each input field.
const char* input_parameter1 = "ssid";
const char* input_parameter2 = "pass";
const char* input_parameter3 = "ip";
const char* input_parameter4 = "gateway";

// Variables pour enregistrer les valeurs du formulaire HTML.
//Variables to save values from HTML form.
String ssid;
String pass;
String ip;
String gateway;

// Chemins des fichiers pour enregistrer les valeurs des entrées de manière permanente.
// File paths to save input values permanently.
const char* SSID_path = "/ssid.txt";
const char* Password_path = "/pass.txt";
const char* IP_path = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

boolean restart = false;

// Définir l'adresse IP, la passerelle et le sous-réseau.
// Set the IP address, gateway, and subnet.
IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

// Définir les variables et les broches.
// Define the variable and Pin.
const int relayPin = 15;
const int buttonUp = 12;
const int buttonDown = 13;
int setPoint = 4;      // Default set point temperature in Celsius. // Température de consigne par défaut en degrés Celsius.

// Variable pour le mode de démarrage.
// Variable for the startup mode.
int buttonState = HIGH;
int lastButtonState = HIGH;
bool modeSelected = false;
bool startInWiFiMode = true;

// Variables pour le switch case pour l'affichage.
// Variables for the switch case for display.
const int buttonMode = 14;  // Variable qui sert pour 2 fonctions. // Variable used for 2 functions.
int buttonModeState = 0; // Variable pour stocker l'état du bouton. // Variable to store the button state.
int lastButtonModeState = 0; // Pour détecter les changements d'état. // To detect state changes.
int switchDisplayMode = 0; // Mode d'affichage actuel. // Current display mode.

// Lire un fichier depuis LittleFS.
// Read File from LittleFS.
String readFile(fs::FS& fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Écrire un fichier depuis LittleFS.
// Write file to LittleFS.
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}

// Initialise le WiFi.
// Initialize WiFi.
bool initialize_Wifi() {
  if (ssid == "" || ip == "") {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)) {
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");
  delay(20000);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect.");
    return false;
  }
  Serial.println(WiFi.localIP());
  return true;
}

// Remplacer le modèle par les valeurs réelles de la page index.html.
// Replace the placeholder with the actual values from the index.html page.
String processor(const String& var) {
  if (var == "TEMP_AMBIANTE") {
    sensors.requestTemperatures();
    float tempAmbiante = sensors.getTempCByIndex(0);  
    return String(tempAmbiante);
  } else if (var == "TEMP_EVAPORATEUR") {
    sensors.requestTemperatures();
    float tempEvaporateur = sensors.getTempCByIndex(1);  
    return String(tempEvaporateur);
  } else if (var == "SETPOINT") {
    return String(setPoint);  
  }
  return String();
}

// Affichage du mode de démarrage sélectionné sur l'écran OLED.
// Display the selected startup mode on the OLED screen.
void displayMode(String mode) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Mode:");
  display.setCursor(0, 30);
  display.println(mode);
  display.display();
}

// Affiche l'adresse IP sur l'écran OLED.
// Display the IP address on the OLED screen.
void displayIP() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("IP Address");
  
  // Vérifiez si le NodeMCU est en mode AP.
  // Check if the NodeMCU is in AP mode.
  if (WiFi.getMode() & WIFI_AP) {
    IPAddress ip = WiFi.softAPIP();
    display.setCursor(0, 30);
    display.println(ip.toString());
  }
  // Sinon, il est en mode station (STA) connecté au réseau WiFi.
  // Otherwise, it is in station (STA) mode connected to the WiFi network.
  else if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    display.setCursor(0, 30);
    display.println(ip.toString());
  }
  // Si aucune connexion n'est active, affiche "No IP".
  // If no connection is active, display 'No IP'.
  else {
    display.setCursor(0, 30);
    display.println("No IP");
  }
  display.display();
}

// Fonction qui permet l'affichage des temperatures sur l'écran OLED.
// Function that allows displaying temperatures on the OLED screen.
void displayTemperature(const char* label, float temperature, int yPosition) {
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(label);
  display.setTextSize(3);
  display.setCursor(0, yPosition);
  display.print(temperature);
  display.print(F(" C"));
  display.display();
}

void setup() {
  Serial.begin(115200);

// Mode de fonctionnement des broches.
// Pin mode configuration.
  pinMode(relayPin, OUTPUT);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonMode, INPUT_PULLUP); 

  sensors.begin();

// Initialisation de LittleFS.
// Initialization of LittleFS.
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();
  delay(2000);  // Pause pour voir le démarrage. // Pause to observe the startup.

  // Sélection du mode de démarrage.
  // Select the startup mode.
  while (!modeSelected) {
    buttonState = digitalRead(buttonMode);

    if (buttonState == LOW && lastButtonState == HIGH) {
      // Changement de mode lors de la pression du bouton.
      // Change mode when the button is pressed.
      startInWiFiMode = !startInWiFiMode;
      Serial.print("Mode changed to: ");
      Serial.println(startInWiFiMode ? "WiFi Mode" : "AP Mode");

      // Affichage du mode sur l'OLED.
      // Display the mode on the OLED.
      if (startInWiFiMode) {
        displayMode("WiFi Mode");
      } else {
        displayMode("AP Mode");
      }
      delay(250);  // Anti-rebond pour éviter de multiples changements rapides. // Debouncing to avoid multiple rapid changes.
    }
    lastButtonState = buttonState;

    // Condition de sortie de la boucle.
    // While exit condition.
    if (buttonState == HIGH) {
      modeSelected = true;
      Serial.println("Mode selected, exiting loop.");
    }
  }

// Lire le SSID, le mot de passe et l'adresse IP depuis les fichiers LittleFS, les enregistrer dans des variables et les afficher dans le moniteur série.
// Read SSID, password, and IP address from LittleFS files, save them in variables, and print them to the serial monitor.
  ssid = readFile(LittleFS, SSID_path);
  pass = readFile(LittleFS, Password_path);
  ip = readFile(LittleFS, IP_path);
  gateway = readFile(LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

// Si la fonction initialize_Wifi() retourne true, gérer les requêtes HTTP reçues en mode station avec le serveur web asynchrone.
// If the initialize_Wifi() function returns true, handle HTTP requests received in station mode with the asynchronous web server.
  if (startInWiFiMode && initialize_Wifi()) {
    Serial.println("Server start wifi mode");

// Route pour servir index.html.
// Route to serve index.html.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Server ON");
    request->send(LittleFS, "/index.html", "text/html");
    Serial.println("Server ON:  Send index.html");
  });
// Route pour servir style.css.
// Route to serve style.css.
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
});

// Ajouter des routes pour répondre aux requêtes AJAX.
// Add routes to handle AJAX requests.
server.on("/getTempAmbiante", HTTP_GET, [](AsyncWebServerRequest *request){
  sensors.requestTemperatures();
  delay(10); 
  float tempAmbiante = sensors.getTempCByIndex(0);
  if (isnan(tempAmbiante)) {
    Serial.println("Error reading ambient temperature");
    request->send(500, "text/plain", "Error reading ambient temperature");
  } else {
    request->send(200, "text/plain", String(tempAmbiante));
  }
  Serial.println("Server on request->send(200");
});

server.on("/getTempEvaporateur", HTTP_GET, [](AsyncWebServerRequest *request){
  sensors.requestTemperatures();
  delay(10); 
  float tempEvaporateur = sensors.getTempCByIndex(1);
  if (isnan(tempEvaporateur)) {
    Serial.println("Error reading evaporator temperature");
    request->send(500, "text/plain", "Error reading evaporator temperaturer");
  } else {
    request->send(200, "text/plain", String(tempEvaporateur));
  }
});

server.on("/getSetPoint", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(setPoint));
});

// Route AJAX pour augmenter le setpoint.
// AJAX Route to Increase Setpoint.
server.on("/setpointPlus", HTTP_GET, [](AsyncWebServerRequest *request) {
    setPoint++;
    Serial.println("Setpoint increased: " + String(setPoint));
    request->send(200, "text/plain", "OK");
});

// Route AJAX pour diminuer le setpoint.
// AJAX Route to Decrease Setpoint.
server.on("/setpointMoins", HTTP_GET, [](AsyncWebServerRequest *request) {
    setPoint--;
    Serial.println("Setpoint decreased: " + String(setPoint));
    request->send(200, "text/plain", "OK");
});

    server.begin();
    Serial.println("Server started successfully.");
  } else {
    Serial.println("Setting Access Point");
    WiFi.softAP("Ouellet-Cool", NULL);

// Affichage de l'adresse Ip sur le port série.
// Display the IP address on the serial port.
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // URL racine du serveur Web.
    // Web Server Root URL.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });

    server.serveStatic("/", LittleFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          // Valeur SSID de la requête HTTP POST.
          // HTTP POST ssid value.
          if (p->name() == input_parameter1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Écrire un fichier pour enregistrer la valeur.
            // Write file to save value.
            writeFile(LittleFS, SSID_path, ssid.c_str());
          }
          // Valeur pass de la requête HTTP POST.
          // HTTP POST pass value.
          if (p->name() == input_parameter2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Écrire un fichier pour enregistrer la valeur.
            // Write file to save value
            writeFile(LittleFS, Password_path, pass.c_str());
          }
          // Valeur ip de la requête HTTP POST.
          // HTTP POST ip value
          if (p->name() == input_parameter3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Écrire un fichier pour enregistrer la valeur.
            // Write file to save value
            writeFile(LittleFS, IP_path, ip.c_str());
          }
          if (p->name() == input_parameter4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Écrire un fichier pour enregistrer la valeur.
            // Write file to save value
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
        }
      }
      restart = true;
      request->send(200, "text/plain", "Success. Ouellet-Cool will now restart. Connect to your router and go to IP address: " + ip);
      delay(10);
    });
    server.begin();
    Serial.println("Server started successfully in AP mode.");
  }
}

void loop() {
  if (restart) {
    delay(5000);
    Serial.print("Restart");
    ESP.restart();
  }

    // Lire l'état du bouton.
    // Read the button state.
  buttonModeState = digitalRead(buttonMode);
    // Vérifier si le bouton a été pressé.
    // Check if the button has been pressed.
  if (buttonModeState == LOW && lastButtonModeState == HIGH) {
    // Changer de mode d'affichage.
    // Change the display mode.
    switchDisplayMode = (switchDisplayMode + 1) % 4; // Alterner entre 0, 1, 2 et 3. // Cycle between 0, 1, 2, and 3.
  }
  lastButtonModeState = buttonModeState;
  // Afficher en fonction du mode d'affichage.
  // Display based on the display mode.
  display.clearDisplay();

  // Demander les températures aux capteurs.
  // Request temperatures from the sensors.
  sensors.requestTemperatures();
  float roomTemp = sensors.getTempCByIndex(0);  // Température ambiante. // Room temperature.
  float evapTemp = sensors.getTempCByIndex(1);  // Température de l'évaporateur. // Evap temperature.

  switch (switchDisplayMode) {
    case 0:
      displayTemperature("Room temp: ", roomTemp, 30);
      break;
    case 1:
      displayTemperature("Evap temp: ", evapTemp, 30);
      break;
    case 2:
      displayTemperature("SetPoint: ", setPoint, 30);
      // Ajuster le point de consigne en fonction des entrées des boutons.
      // Adjust set point based on button inputs.
      if (digitalRead(buttonUp) == LOW) {
        setPoint++;
        delay(250);  // Delais anti-rebond. // Debounce delay.
        }
      if (digitalRead(buttonDown) == LOW) {
        setPoint--;
        delay(250);  // Delais anti-rebond. // Debounce delay.
        }
      break;
    case 3:
      // Affiche l'adresse IP sur l'OLED.
      // Display the IP address on the OLED.
      displayIP();
      break;
  }
   
// Variables pour stocker le dernier temps d'action pour la fonction controlRelay.
// Variables to store the last action time for the controlRelay function.
unsigned long previousMillisControlRelay = 0;
const long intervalControlRelay = 30000; // Intervalle de 30 secondes. // 30-second interval.

// Contrôler le relais en fonction de la température de l'évaporateur et du point de consigne.
// Control the relay based on the evaporator temperature and the setpoint.
unsigned long currentMillisControlRelay = millis();

if (roomTemp > setPoint && evapTemp > 0.75) {
  // Si le relais est éteint et que les conditions sont réunies.
  // If the relay is off and the conditions are met.
  if (currentMillisControlRelay - previousMillisControlRelay >= intervalControlRelay) {
    previousMillisControlRelay = currentMillisControlRelay;
    digitalWrite(relayPin, HIGH);  // Allumer le chauffage. // Turn on the heating.
  }
} else {
  // Si le relais est allumé et que les conditions ne sont plus réunies.
  // If the relay is on and the conditions are no longer met.
  if (digitalRead(relayPin) == HIGH) {
    if (currentMillisControlRelay - previousMillisControlRelay >= intervalControlRelay) {
      previousMillisControlRelay = currentMillisControlRelay;
      digitalWrite(relayPin, LOW);  // Éteindre le chauffage. // Turn off the heating.
    }
  }
}
}

