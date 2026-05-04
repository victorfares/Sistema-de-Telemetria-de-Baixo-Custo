#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> // ADICIONE ESTA LINHA

// ==========================================
// 1. ESTRUTURA DE DADOS (Deve ser IDÊNTICA ao Emissor)
// ==========================================
typedef struct data_log {
  double temperaturaAmbiente;
  double temperaturaSuperficial;
  unsigned long timestamp;
} data_log;

data_log payloadRecebido;

// ==========================================
// 2. CALLBACK DE RECEPÇÃO ESP-NOW
// ==========================================
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Copia os dados que chegaram da antena para a nossa struct
  memcpy(&payloadRecebido, incomingData, sizeof(payloadRecebido));
  
  // Imprime os dados organizados no Monitor Serial
  Serial.println("=== Novo Pacote Recebido ===");
  Serial.print("Tamanho do pacote: ");
  Serial.print(len);
  Serial.println(" bytes");
  
  Serial.print("Temp. Superficial: ");
  Serial.print(payloadRecebido.temperaturaSuperficial);
  Serial.println(" °C");
  
  Serial.print("Temp. Ambiente:    ");
  Serial.print(payloadRecebido.temperaturaAmbiente);
  Serial.println(" °C");
  
  Serial.print("Tempo de Voo:      ");
  Serial.print(payloadRecebido.timestamp);
  Serial.println(" ms");
  Serial.println("============================\n");
}

// ==========================================
// SETUP E LOOP PRINCIPAL
// ==========================================
void setup() {
  Serial.begin(115200);

  // Configura a placa apenas como Estação Wi-Fi (WIFI_STA)
  // Isso liga a antena no modo necessário para o ESP-NOW, sem tentar conectar a roteadores
  WiFi.mode(WIFI_STA);
  // FORÇA O ESP32 A FICAR NO CANAL DE RÁDIO 1
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  Serial.println("\n--- Iniciando Receptor de Telemetria ---");
  Serial.print("Endereço MAC desta placa Base: ");
  Serial.println(WiFi.macAddress()); // Imprime o MAC para você confirmar se está igual no código do foguete!

  // Inicializa o protocolo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro Crítico: Falha ao inicializar o ESP-NOW");
    return;
  }
  
  // Registra a função que será chamada automaticamente toda vez que um pacote chegar
  esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
  
  Serial.println("Receptor pronto e escutando a frequência...\n");
}

void loop() {
  // O loop fica praticamente vazio. 
  // O ESP32 usa interrupções de hardware para o rádio: assim que o sinal bate na antena, 
  // ele pausa o loop, roda a função OnDataRecv e volta.
  delay(1000);
}