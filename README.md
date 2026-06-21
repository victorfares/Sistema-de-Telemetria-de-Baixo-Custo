# STGT - Sistema de Telemetria e Gerenciamento Térmico de Baixo Custo

![Badge Status](https://img.shields.io/badge/Status-Em%20Desenvolvimento-green)
![Badge Hardware](https://img.shields.io/badge/Hardware-ESP32%20%7C%20OLED%20%7C%20GY--906-blue)
![Badge Backend](https://img.shields.io/badge/Backend-Java%2017%20%7C%20Spring%20Boot-brightgreen)

## Sobre o Projeto
O **STGT (Sistema de Telemetria e Gerenciamento Térmico)** é uma solução de engenharia desenvolvida pela Equipe Rocket para resolver o problema do "Voo Cego" em foguetemodelismo. Durante o voo, o atrito aerodinâmico e o calor da propulsão (motor KNSB) podem causar falhas catastróficas na eletrônica embarcada (*Thermal Throttling* e derretimento de trilhas).

Utilizando a filosofia **COTS (Commercial Off-The-Shelf)**, este projeto substitui altímetros e sistemas de telemetria comerciais de alto custo por uma arquitetura acessível e de alta performance baseada no microcontrolador ESP32.

---

## Arquitetura do Sistema
O projeto é dividido em três camadas principais:

1. **Camada Embarcada (Foguete):** Lê a temperatura ambiente e superficial via I2C (Sensor GY-906) e transmite os dados estruturados via rádio frequência (2.4 GHz) em tempo real.
2. **Camada de Solo (Receptor):** Capta os pacotes via rádio, exibe alertas visuais críticos numa tela OLED e atua como um *Gateway*, roteando os dados via Wi-Fi para a nuvem.
3. **Camada de Nuvem (API Backend):** Um servidor RESTful em Java (Spring Boot) que recebe, valida e persiste os logs de voo em um banco de dados relacional para futura plotagem gráfica.

---

## Como Funciona

### 1. Comunicação Ponto a Ponto (ESP-NOW)
O envio dos dados utiliza o protocolo ESP-NOW para o tráfego de pacotes binários brutos através de uma estrutura de dados contendo a temperatura ambiente, a temperatura superficial e o timestamp do voo em milissegundos, garantindo velocidade de amostragem sem a latência do Wi-Fi convencional.

### 2. Tratamento Assíncrono na Base de Solo
A recepção dos pacotes de rádio é feita via interrupção de hardware por uma função de callback dedicada. Os dados recebidos são copiados em memória e um sinalizador booleano é ativado para que o loop principal atualize a tela OLED sem interromper a escuta do rádio.

### 3. Persistência na API Spring Boot
O Gateway de solo envia os dados estruturados para o Backend através de uma requisição HTTP POST. Um controlador REST intercepta os dados mapeados em um objeto de transferência de dados (DTO), valida as informações e delega a gravação para o repositório JPA, que persiste o histórico de telemetria no banco de dados.

---

## Como Testar e Rodar a Aplicação

### Requisitos
* Java 17+ instalado.
* Arduino IDE (com pacotes do ESP32 e bibliotecas Adafruit instaladas).
* Postman ou Insomnia (para testar a API).

### Passo 1: Subindo a API (Backend)
1. Navegue até a pasta `backend/`.
2. Abra o terminal e execute o comando do Maven para iniciar o servidor (`./mvnw spring-boot:run`).
3. O servidor iniciará na porta `8080` com o banco de dados H2 rodando em memória.

### Passo 2: Simulando o Foguete no Postman
Configure uma nova requisição do tipo POST para o endpoint da API no endereço local porta 8080 (/api/telemetry). Adicione o cabeçalho de tipo de conteúdo como JSON e envie as propriedades estruturadas com valores numéricos para temperaturaAmbiente, temperaturaSuperficial e timestampVoo.

> **Nota:** Ao realizar o disparo do POST, o servidor retornará o código de status 200 OK e persistirá o registro com o momento exato do recebimento.

### Passo 3: Consultando o Histórico de Voo
Para verificar os registros armazenados e simular a dashboard reativa do frontend, realize uma requisição do tipo GET para o mesmo endpoint de telemetria para obter a listagem completa dos logs.

### Passo 4: Rodando o Hardware (ESP32)
1. Vincule o endereço MAC correto do receptor no código do emissor.
2. Configure as credenciais do Wi-Fi local e informe o IP de rede da máquina servidora no código da base de solo.
3. Realize o upload dos códigos e inicialize o monitoramento.
