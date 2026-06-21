🚀 STGT - Sistema de Telemetria e Gerenciamento Térmico de Baixo Custo
📌 Sobre o Projeto
O STGT (Sistema de Telemetria e Gerenciamento Térmico) é uma solução de engenharia desenvolvida pela Equipe Rocket para resolver o problema do "Voo Cego" em foguetemodelismo. Durante o voo, o atrito aerodinâmico e o calor da propulsão (motor KNSB) podem causar falhas catastróficas na eletrônica embarcada (Thermal Throttling e derretimento de trilhas).

Utilizando a filosofia COTS (Commercial Off-The-Shelf), este projeto substitui altímetros e sistemas de telemetria comerciais de alto custo (que ultrapassam R$ 4.500) por uma arquitetura acessível e de alta performance baseada no microcontrolador ESP32.

🏗️ Arquitetura do Sistema
O projeto é dividido em três camadas principais:

Camada Embarcada (Foguete): Lê a temperatura ambiente e superficial via I2C (Sensor GY-906) e transmite os dados estruturados via rádio frequência (2.4 GHz) em tempo real.

Camada de Solo (Receptor): Capta os pacotes via rádio, exibe alertas visuais críticos numa tela OLED e atua como um Gateway, roteando os dados via Wi-Fi para a nuvem.

Camada de Nuvem (API Backend): Um servidor RESTful em Java (Spring Boot) que recebe, valida e persiste os logs de voo em um banco de dados relacional para futura plotagem gráfica.

⚙️ Como Funciona (Snippets de Código)
1. Comunicação Ponto a Ponto (ESP-NOW)
Para evitar a latência de redes Wi-Fi tradicionais, utilizamos o protocolo ESP-NOW. Os dados trafegam em pacotes binários crus através de uma struct C++, garantindo máxima velocidade de amostragem.

C++
// Molde do pacote que trafega pelo ar (C++)
typedef struct data_log {
  double temperaturaAmbiente;
  double temperaturaSuperficial;
  unsigned long timestamp; // Tempo de voo em milissegundos
} data_log;
2. Tratamento Assíncrono na Base de Solo
A base recebe o sinal via interrupção de hardware e delega o desenho da tela OLED de forma assíncrona, evitando que o Gateway trave (Kernel Panic) ao receber muitos dados.

C++
// Interrupção ativada instantaneamente quando o pacote de rádio chega
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&payloadRecebido, incomingData, sizeof(payloadRecebido));
  novaLeitura = true; // Levanta a flag segura de forma não-bloqueante
}
3. Persistência na API Spring Boot
Os dados do Gateway são recebidos no Backend através de um Controlador REST, que converte o JSON recebido num Data Transfer Object (DTO) e o envia para o banco de dados via Spring Data JPA.

Java
// Porta de entrada do Servidor (Java)
@PostMapping
public ResponseEntity<TelemetryLog> receberDados(@RequestBody TelemetryRequestDTO dto) {
    TelemetryLog logSalvo = service.salvarTelemetria(dto);
    System.out.println("Pacote salvo: Temp Superficial = " + logSalvo.getTemperaturaSuperficial() + "°C");
    return ResponseEntity.ok(logSalvo);
}
🛠️ Como Testar e Rodar a Aplicação
Requisitos
Java 17+ instalado.

Arduino IDE (com pacotes do ESP32 e bibliotecas Adafruit instaladas).

Postman ou Insomnia (para testar a API sem o hardware).

Passo 1: Subindo a API (Backend)
Navegue até a pasta backend/.

Abra o terminal e rode o comando do Maven para iniciar o servidor:

Bash
./mvnw spring-boot:run
O servidor iniciará na porta 8080 com o banco de dados H2 rodando em memória.

Passo 2: Simulando o Foguete no Postman
Se você não estiver com as placas físicas, pode simular o envio dos dados de telemetria enviando um HTTP POST.

URL: http://localhost:8080/api/telemetry

Método: POST

Headers: Content-Type: application/json

Body (JSON):

JSON
{
  "temperaturaAmbiente": 28.5,
  "temperaturaSuperficial": 85.2,
  "timestampVoo": 12500
}
Nota: Ao enviar este POST, o servidor responderá com o Status 200 OK e o registro será salvo automaticamente com a data e hora do recebimento.

Passo 3: Consultando o Histórico de Voo
Para ver o histórico completo de pacotes (ideal para o frontend desenhar gráficos):

URL: http://localhost:8080/api/telemetry

Método: GET

Passo 4: Rodando o Hardware (ESP32)
Substitua o Endereço MAC (no código do Emissor) pelo MAC da sua placa Receptora.

No código do Receptor, atualize as credenciais do Wi-Fi (ssid e password) e substitua 192.168.X.X pelo IP local da máquina que está rodando o Spring Boot.

Conecte os sensores (Pinos I2C) e faça o upload.

Assim que o foguete for energizado, os dados aparecerão na tela OLED e no console do backend.
