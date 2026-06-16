// ==========================================
// CÓDIGO PARA EL SLAVE (ESP32 con 3 relés)
// Versión con mejoras anti-reinicio
// ==========================================
#include <WiFi.h>
#include <WebServer.h>

// Configuración WiFi
const char *ssid = "MEGACABLE-2.4G-9CAA";
const char *password = "bSYmEa7NHa";

// Pines donde tienes conectados los relés
#define RELE_PIN_1  5    // Sala
#define RELE_PIN_2  15   // Cocina
#define RELE_PIN_3  16   // Patio
#define SENSOR_PIN  34   // Sensor analogico o digital

WebServer server(80);

// Función segura para activar relés (con delay para evitar picos)
void activar_rele(int pin, const char* nombre) {
  digitalWrite(pin, LOW);
  delay(10);  // Pequeña pausa para estabilizar
  Serial.printf("Rele %s: ON (pin %d)\n", nombre, pin);
}

void desactivar_rele(int pin, const char* nombre) {
  digitalWrite(pin, HIGH);
  delay(10);
  Serial.printf("Rele %s: OFF (pin %d)\n", nombre, pin);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== INICIANDO SLAVE ===\n");
  
  // Configurar pines de relés como salida
  pinMode(RELE_PIN_1, OUTPUT);
  pinMode(RELE_PIN_2, OUTPUT);
  pinMode(RELE_PIN_3, OUTPUT);
  
  // Estado inicial: HIGH = rele desactivado
  digitalWrite(RELE_PIN_1, HIGH);
  digitalWrite(RELE_PIN_2, HIGH);
  digitalWrite(RELE_PIN_3, HIGH);
  
  // Esperar a que los pines se estabilicen
  delay(100);
  
  // Conectar a WiFi
  Serial.print("Conectando WiFi");
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 40) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Conectado!");
    Serial.print("IP del Slave: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nError: No se pudo conectar WiFi");
  }
  
  // ========== ENDPOINTS ==========
  
  // Rele 1 - Sala (pin 5)
  server.on("/rele/5/on", []() {
    activar_rele(RELE_PIN_1, "SALA");
    server.send(200, "application/json", "{\"estado\":\"ON\"}");
  });
  
  server.on("/rele/5/off", []() {
    desactivar_rele(RELE_PIN_1, "SALA");
    server.send(200, "application/json", "{\"estado\":\"OFF\"}");
  });
  
  // Rele 2 - Cocina (pin 15)
  server.on("/rele/15/on", []() {
    activar_rele(RELE_PIN_2, "COCINA");
    server.send(200, "application/json", "{\"estado\":\"ON\"}");
  });
  
  server.on("/rele/15/off", []() {
    desactivar_rele(RELE_PIN_2, "COCINA");
    server.send(200, "application/json", "{\"estado\":\"OFF\"}");
  });
  
  // Rele 3 - Patio (pin 16)
  server.on("/rele/16/on", []() {
    activar_rele(RELE_PIN_3, "PATIO");
    server.send(200, "application/json", "{\"estado\":\"ON\"}");
  });
  
  server.on("/rele/16/off", []() {
    desactivar_rele(RELE_PIN_3, "PATIO");
    server.send(200, "application/json", "{\"estado\":\"OFF\"}");
  });
  
  // Endpoint para leer sensor
  server.on("/sensor", []() {
    int valor = analogRead(SENSOR_PIN);
    server.send(200, "application/json", "{\"sensor\":" + String(valor) + "}");
  });
  
  // Endpoint raíz
  server.on("/", []() {
    server.send(200, "application/json", "{\"status\":\"ok\",\"ip\":\"" + WiFi.localIP().toString() + "\"}");
  });
  
  server.begin();
  Serial.println("Servidor HTTP iniciado");
  Serial.println("\n=== SLAVE LISTO ===");
}

void loop() {
  server.handleClient();
  delay(10);  // Delay pequeño pero suficiente
}