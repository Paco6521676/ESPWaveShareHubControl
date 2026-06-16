// ==========================================
// CÓDIGO PARA EL SLAVE 2 (ESP32 con 3 relés)
// Pines: 14, 26, 27
// Pin 12: LED indicador de WiFi conectado
// ==========================================
#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "MEGACABLE-2.4G-9CAA";
const char *password = "bSYmEa7NHa";

#define RELE_PIN_1  14   // Sala 2
#define RELE_PIN_2  26   // Cocina 2
#define RELE_PIN_3  27   // Patio 2
#define LED_WIFI_PIN 12  // LED indicador de WiFi conectado

WebServer server(80);

void activar_rele(int pin, const char* nombre) {
  digitalWrite(pin, LOW);
  delay(10);
  Serial.printf("%s: ON (pin %d)\n", nombre, pin);
}

void desactivar_rele(int pin, const char* nombre) {
  digitalWrite(pin, HIGH);
  delay(10);
  Serial.printf("%s: OFF (pin %d)\n", nombre, pin);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== SLAVE 2 ===\n");
  
  // Configurar pines de relés
  pinMode(RELE_PIN_1, OUTPUT);
  pinMode(RELE_PIN_2, OUTPUT);
  pinMode(RELE_PIN_3, OUTPUT);
  
  // Configurar pin del LED WiFi
  pinMode(LED_WIFI_PIN, OUTPUT);
  digitalWrite(LED_WIFI_PIN, LOW);  // Inicia apagado
  
  // Iniciar relés apagados
  digitalWrite(RELE_PIN_1, HIGH);
  digitalWrite(RELE_PIN_2, HIGH);
  digitalWrite(RELE_PIN_3, HIGH);
  
  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // WiFi conectado - ENCENDER pin 12
  digitalWrite(LED_WIFI_PIN, HIGH);
  
  Serial.println("\nWiFi Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // ========== ENDPOINTS ==========
  server.on("/rele/14/on", []() {
    activar_rele(RELE_PIN_1, "SALA 2");
    server.send(200, "application/json", "{\"estado\":\"ON\"}");
  });
  server.on("/rele/14/off", []() {
    desactivar_rele(RELE_PIN_1, "SALA 2");
    server.send(200, "application/json", "{\"estado\":\"OFF\"}");
  });
  
  server.on("/rele/26/on", []() {
    activar_rele(RELE_PIN_2, "COCINA 2");
    server.send(200, "application/json", "{\"estado\":\"ON\"}");
  });
  server.on("/rele/26/off", []() {
    desactivar_rele(RELE_PIN_2, "COCINA 2");
    server.send(200, "application/json", "{\"estado\":\"OFF\"}");
  });
  
  server.on("/rele/27/on", []() {
    activar_rele(RELE_PIN_3, "PATIO 2");
    server.send(200, "application/json", "{\"estado\":\"ON\"}");
  });
  server.on("/rele/27/off", []() {
    desactivar_rele(RELE_PIN_3, "PATIO 2");
    server.send(200, "application/json", "{\"estado\":\"OFF\"}");
  });
  
  server.begin();
  Serial.println("Servidor listo");
  Serial.println("Endpoints: /rele/14/on|off, /rele/26/on|off, /rele/27/on|off");
  Serial.println("LED WiFi en pin 12: ENCENDIDO");
}

void loop() {
  server.handleClient();
}