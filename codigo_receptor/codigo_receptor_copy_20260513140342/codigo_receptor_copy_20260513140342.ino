#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h> // Biblioteca necessária para requisições HTTP

// ==========================================
// CONFIGURAÇÕES DA TELA OLED
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==========================================
// CONFIGURAÇÕES DE REDE E SERVIDOR
// ==========================================
// INSIRA OS DADOS DO SEU WI-FI (Pode ser o roteador do celular)
const char* ssid = "Wifi_teste_2026";
const char* password = "12345678910";

// IMPORTANTE: Troque "192.168.X.X" pelo IP local da sua máquina (IPv4)
const String serverName = "http://192.168.3.17:8080/api/telemetry";

// ==========================================
// 1. ESTRUTURA DE DADOS
// ==========================================
typedef struct data_log {
  double temperaturaAmbiente;
  double temperaturaSuperficial;
  unsigned long timestamp;
} data_log;

data_log payloadRecebido;

// Flag de segurança para o loop
volatile bool novaLeitura = false; 

// ==========================================
// 2. CALLBACK DE RECEPÇÃO ESP-NOW
// ==========================================
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&payloadRecebido, incomingData, sizeof(payloadRecebido));
  
  Serial.println("=== Novo Pacote Recebido ===");
  Serial.print("Temp. Superficial: "); Serial.print(payloadRecebido.temperaturaSuperficial); Serial.println(" °C");
  Serial.print("Tempo de Voo:      "); Serial.print(payloadRecebido.timestamp); Serial.println(" ms");
  Serial.println("============================\n");

  novaLeitura = true; 
}

// ==========================================
// 3. FUNÇÃO PARA DISPARAR O HTTP POST
// ==========================================
void enviarParaNuvem() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    char jsonBuffer[128];
    snprintf(jsonBuffer, sizeof(jsonBuffer), 
             "{\"temperaturaAmbiente\":%.2f,\"temperaturaSuperficial\":%.2f,\"timestampVoo\":%lu}", 
             payloadRecebido.temperaturaAmbiente, 
             payloadRecebido.temperaturaSuperficial, 
             payloadRecebido.timestamp);

    // Dispara a requisição
    int httpResponseCode = http.POST(jsonBuffer);

    if (httpResponseCode > 0) {
      Serial.print("HTTP POST enviado! Código de Resposta: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Erro Crítico no HTTP POST. Código: ");
      Serial.println(httpResponseCode);
    }
    
    http.end(); // Libera os recursos
  } else {
    Serial.println("Aviso: Wi-Fi desconectado. Pacote salvo apenas localmente.");
  }
}

// ==========================================
// 4. FUNÇÃO PARA DESENHAR NO OLED
// ==========================================
void atualizarDisplayOLED() {
  display.clearDisplay(); 
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("EQUIPE ROCKET - STGT");
  
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE); 

  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("T. Superficial:");
  
  display.setTextSize(2); 
  display.setCursor(0, 25);
  display.print(payloadRecebido.temperaturaSuperficial, 1); 
  display.print(" C");

  display.setTextSize(1); 
  display.setCursor(0, 45);
  display.print("T. Amb: ");
  display.print(payloadRecebido.temperaturaAmbiente, 1);
  display.print(" C");

  display.setCursor(0, 55);
  display.print("Status: ");
  
  bool alertaAtivo = payloadRecebido.temperaturaSuperficial >= 80.0;

  if (alertaAtivo) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); 
    display.print(" CRITICO ");
    display.setTextColor(SSD1306_WHITE); 
  } else {
    display.print("NOMINAL");
  }
  display.display(); 
}

// ==========================================
// SETUP E LOOP PRINCIPAL
// ==========================================
void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha ao inicializar o OLED SSD1306"));
    for(;;); 
  }
  
  // Animação de Boot
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println("STGT - Base Solo");
  display.setCursor(25, 35);
  display.println("Conectando Rede...");
  display.display();

  // 1. Conexão Wi-Fi (Necessária para o HTTP)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
  Serial.print("IP da Base: ");
  Serial.println(WiFi.localIP());

  Serial.println("\n--- Iniciando Receptor de Telemetria ---");
  Serial.print("Endereço MAC da Base: ");
  Serial.println(WiFi.macAddress()); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro Crítico: Falha ao inicializar o ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
  
  display.clearDisplay();
  display.setCursor(0, 25);
  display.print("Aguardando Voo...");
  display.display();
}

void loop() {
  if (novaLeitura == true) {
    atualizarDisplayOLED();
    enviarParaNuvem();      
    novaLeitura = false;    
  }
  delay(10); 
}
