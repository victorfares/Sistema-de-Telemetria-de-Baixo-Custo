#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================================
// CONFIGURAÇÕES DA TELA OLED
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C // Endereço padrão I2C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==========================================
// 1. ESTRUTURA DE DADOS (Idêntica ao Emissor)
// ==========================================
typedef struct data_log {
  double temperaturaAmbiente;
  double temperaturaSuperficial;
  unsigned long timestamp;
} data_log;

data_log payloadRecebido;

// Flag (sinalizador) para avisar o loop que chegou dado novo
volatile bool novaLeitura = false; 

// ==========================================
// 2. CALLBACK DE RECEPÇÃO ESP-NOW
// ==========================================
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Copia os dados
  memcpy(&payloadRecebido, incomingData, sizeof(payloadRecebido));
  
  Serial.println("=== Novo Pacote Recebido ===");
  Serial.print("Temp. Superficial: "); Serial.print(payloadRecebido.temperaturaSuperficial); Serial.println(" °C");
  Serial.print("Temp. Ambiente:    "); Serial.print(payloadRecebido.temperaturaAmbiente); Serial.println(" °C");
  Serial.print("Tempo de Voo:      "); Serial.print(payloadRecebido.timestamp); Serial.println(" ms");
  Serial.println("============================\n");

  novaLeitura = true; 
}

// ==========================================
// 3. FUNÇÃO PARA DESENHAR NO OLED
// ==========================================
void atualizarDisplayOLED() {
  display.clearDisplay(); // Limpa o quadro antigo

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("EQUIPE ROCKET - STGT");
  
  // Linha separadora horizontal
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE); 

  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("T. Superficial:");
  
  display.setTextSize(2); // Fonte grande!
  display.setCursor(0, 25);
  display.print(payloadRecebido.temperaturaSuperficial, 1); // 1 casa decimal
  display.print(" C");

  display.setTextSize(1); // Volta para a fonte menor
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
    display.setTextColor(SSD1306_WHITE); // Volta a cor ao normal
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
    for(;;); // Trava o código aqui se o display estiver desconectado
  }
  
  // Animação de Boot da Equipe
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println("STGT - Base Solo");
  display.setCursor(25, 35);
  display.println("Iniciando...");
  display.display();
  delay(2000); 

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  Serial.println("\n--- Iniciando Receptor de Telemetria ---");
  Serial.print("Endereço MAC da Base: ");
  Serial.println(WiFi.macAddress()); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro Crítico: Falha ao inicializar o ESP-NOW");
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print("ERRO DE RADIO!");
    display.display();
    return;
  }
  
  esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
  
  Serial.println("Receptor pronto e escutando a frequência...\n");

  // Tela de espera
  display.clearDisplay();
  display.setCursor(0, 25);
  display.print("Aguardando Foguete...");
  display.display();
}

void loop() {
  if (novaLeitura == true) {
    atualizarDisplayOLED();
    novaLeitura = false;    // Abaixa a flag para esperar o próximo pacote
  }
  
  delay(10); 
}