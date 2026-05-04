// Codigo transmissora

#include <esp_wifi.h> 
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include "WiFi.h"
#include <esp_now.h>

// Cria o objeto do sensor
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// O MAC Address da placa receptora
uint8_t slaveAddress[] = {0x84, 0x1F, 0xE8, 0x1A, 0xCE, 0x00};

typedef struct data_log{
  double temperaturaAmbiente;
  double temperaturaSuperficial;
  unsigned long timestamp; // Tempo em milissegundos desde o boot
} data_log;

data_log payloadData;
esp_now_peer_info_t peerInfo;

// Callback de envio
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Status do Envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falhou");
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar o ESP-NOW");
    return;
  }

  if (!mlx.begin()) {
    Serial.println("Erro ao conectar com o sensor MLX90614. Verifique as conexões!");
    while (1); 
  }

  // FIX: Adicionado o "cast" explícito para compatibilidade com o núcleo ESP32 mais recente
  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);
  
  // Configura a placa "Amiga" (O Receptor)
  memcpy(peerInfo.peer_addr, slaveAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  
  // Adiciona o Receptor à lista de contatos
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Falha ao adicionar o receptor");
    return;
  } // FIX: Essa chave estava faltando no seu código original!

  Serial.println("Sensor conectado com sucesso!");
  Serial.println("-----------------------------------");
}

void loop() {
  // Lê a temperatura (do próprio sensor)
  payloadData.temperaturaAmbiente = mlx.readAmbientTempC();
  payloadData.temperaturaSuperficial = mlx.readObjectTempC(); // FIX: Ponto e vírgula adicionado
  payloadData.timestamp = millis(); 
  
  // FIX: Consertados os erros de digitação (broadcastAddress -> slaveAddress)
  esp_err_t result = esp_now_send(slaveAddress, (uint8_t *) &payloadData, sizeof(payloadData));
   
  if (result == ESP_OK) {
    Serial.println("Mensagem enviada com sucesso pela antena!");
  } else {
    Serial.println("Erro ao tentar enviar a mensagem.");
  }
  
  delay(2000);
}