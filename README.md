# EPI Inteligente

## Resumo do Projeto

O EPI Inteligente é um sistema embarcado de segurança ocupacional que transforma um capacete de proteção convencional em um equipamento capaz de detectar quedas em tempo real. O problema que o projeto resolve é a demora no atendimento a trabalhadores que sofrem acidentes em ambientes industriais, de construção civil ou similares. Quando um trabalhador cai e fica inconsciente, pode levar minutos ou até horas para que alguém perceba a situação.

O sistema utiliza um microcontrolador ESP32 conectado a um sensor inercial (acelerômetro e giroscópio) que monitora continuamente os movimentos do trabalhador. Quando detecta um impacto seguido de imobilidade prolongada, o dispositivo aciona automaticamente um alarme sonoro e exibe no display OLED as informações de emergência do trabalhador (nome, tipo sanguíneo, alergias, contato de emergência). Além disso, o ESP32 serve um dashboard web acessível por qualquer dispositivo na mesma rede Wi-Fi, permitindo o acompanhamento remoto dos dados dos sensores e do estado do sistema.

---

## Integrantes da Equipe

| Nome Completo | RA |
|---|---|
| Natan Henrique Ramos Silva | Desenvolvedor Firmware/Software |
| Ingrid Barbosa Silva | Modelagem 3D & Design Estrutural |
| João Emanuel Cardoso da Rosa | Gestão de Qualidade & Documentação |
| Pedro Henrique Heerdt Sehnem Wicheral | Líder de Hardware e Prototipação |

---

## Arquitetura da Solucao

O sistema e composto por tres camadas principais: sensoriamento, processamento e interface.

O sensor inercial ICM-20689 coleta dados de aceleracao e velocidade angular nos tres eixos a cada 20 milissegundos e os envia ao ESP32 por meio do barramento I2C (bus 1, pinos SDA=21 e SCL=22).

O ESP32 e o nucleo central do sistema. Ele recebe os dados do sensor e os processa por meio de uma maquina de estados implementada na classe FallDetector. Essa maquina possui tres estados: NORMAL, WAITING_IMMOBILITY e ALERT. Quando a magnitude da aceleracao ultrapassa 20 m/s2 (aproximadamente 2g), o sistema entende que houve um impacto e transita para WAITING_IMMOBILITY. Se apos o impacto o trabalhador permanece imovel por 5 segundos (aceleracao estavel proximo a 1g e giroscopio praticamente zerado), o sistema confirma a queda e transita para ALERT, acionando o buzzer por meio do AlertManager.

O display OLED SSD1306 esta conectado ao ESP32 por um segundo barramento I2C (bus 0, pinos SDA=19 e SCL=18). Ele exibe o perfil de emergencia do trabalhador em 4 paginas rotativas: nome, tipo sanguineo, contato de emergencia e alergias/observacoes.

O ESP32 tambem opera como servidor HTTP na porta 80, conectando-se a uma rede Wi-Fi no modo station (STA). Ele serve arquivos estaticos (HTML, CSS, JavaScript) armazenados no sistema de arquivos LittleFS da memoria flash e expoe uma API REST que retorna os dados do sistema em formato JSON. O cliente web (navegador) acessa o dashboard pelo IP do ESP32 e faz requisicoes GET a cada 1 segundo para atualizar os dados em tempo real.

Resumindo o fluxo: o sensor envia dados ao ESP32 via I2C, o ESP32 processa a deteccao de queda e atualiza o display OLED e o buzzer localmente, e ao mesmo tempo serve o dashboard web para qualquer navegador conectado na mesma rede Wi-Fi.

---

## Hardware Utilizado

### Lista de Componentes

| Componente | Modelo / Especificacao | Quantidade |
|---|---|---|
| Microcontrolador | ESP32 DevKit V1 | 1 |
| Sensor inercial (acelerometro + giroscopio) | ICM-20689 (compativel com GY-521 / MPU6050) | 1 |
| Display OLED | SSD1306 128x64 pixels, I2C | 1 |
| Buzzer | Piezoeletrico passivo | 1 |
| Bateria | 18650 Li-Ion, 3.7V, 2200mAh (1S1P com BMS) | 1 |
| Protoboard e jumpers | - | conforme necessario |
| Cabo USB | Micro-USB (para programacao e alimentacao) | 1 |

### Diagrama de Conexoes (Pinagem)

**Sensor ICM-20689 ao ESP32 (barramento I2C 1):**

| Pino do Sensor | Pino do ESP32 |
|---|---|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| VCC | 3.3V |
| GND | GND |

**Display OLED SSD1306 ao ESP32 (barramento I2C 0):**

| Pino do Display | Pino do ESP32 |
|---|---|
| SDA | GPIO 19 |
| SCL | GPIO 18 |
| VCC | 3.3V |
| GND | GND |

**Buzzer ao ESP32:**

| Pino do Buzzer | Pino do ESP32 |
|---|---|
| Sinal (+) | GPIO 23 |
| GND (-) | GND |

O buzzer e controlado por PWM com frequencia de 2000 Hz e resolucao de 8 bits.

**Bateria:** atualmente simulada por software (valor fixo decrementando). Futuramente sera conectada a um pino ADC do ESP32 por meio de um divisor de tensao para leitura real da carga.

---

## Documentacao da API / Comunicacao

O ESP32 opera como servidor HTTP na porta 80 e expoe os seguintes endpoints:

### GET /api/status

Retorna o estado completo do sistema em formato JSON.

Exemplo de resposta:

```json
{
  "battery": 100,
  "accelerometer": {
    "x": 0.12,
    "y": -0.34,
    "z": 9.81
  },
  "gyroscope": {
    "x": 0.01,
    "y": -0.02,
    "z": 0.00
  },
  "buzzer": false,
  "fallState": "NORMAL",
  "emergency": false,
  "temperature": 25.50
}
```

Descricao dos campos:

| Campo | Tipo | Descricao |
|---|---|---|
| battery | inteiro | Nivel de bateria em porcentagem (simulado) |
| accelerometer | objeto | Aceleracao nos eixos X, Y e Z em m/s2 |
| gyroscope | objeto | Velocidade angular nos eixos X, Y e Z em rad/s |
| buzzer | booleano | Indica se o buzzer esta ativo |
| fallState | string | Estado da deteccao de queda: NORMAL, WAITING_IMMOBILITY ou ALERT |
| emergency | booleano | Indica se o modo de emergencia esta ativo |
| temperature | decimal | Temperatura interna do sensor em graus Celsius |

### POST /api/buzzer/on

Aciona o buzzer remotamente. Nao requer corpo na requisicao. Retorna confirmacao em JSON.

### POST /api/buzzer/off

Desliga o buzzer remotamente. Nao requer corpo na requisicao. Retorna confirmacao em JSON.

### POST /api/emergency/reset

Rearma o detector de queda e desliga o buzzer de emergencia. O cadastro do funcionario é mantido. O loop do ESP32 atualiza o estado do sistema e volta a exibir a bateria no OLED. Nao requer corpo na requisicao.

### POST /api/funcionario/refresh-display

Recarrega na memoria do ESP32 o perfil armazenado para uso no OLED. O perfil so e exibido enquanto o modo de emergencia estiver ativo. Nao altera nem exclui o cadastro.

### GET /

Serve a pagina principal do dashboard (index.html) armazenada no LittleFS.

### GET /cadastro.html

Serve a pagina de cadastro do funcionario.

### Arquivos estaticos

Todos os demais arquivos (style.css, script.js, funcionario.js) sao servidos automaticamente pelo LittleFS a partir do diretorio data/.

### Protocolo de comunicacao

A comunicacao entre o cliente (navegador) e o ESP32 e feita exclusivamente por HTTP sobre Wi-Fi. O dashboard faz polling a cada 1 segundo com requisicoes GET para /api/status e atualiza a interface com os dados recebidos. Os comandos do buzzer sao enviados por requisicoes POST.

---

## Instrucoes de Instalacao e Execucao

### Pre-requisitos

- PlatformIO instalado (como extensao do VS Code ou via CLI). Pode ser instalado em https://platformio.org/install
- ESP32 DevKit conectado ao computador via cabo USB
- Sensor ICM-20689, display OLED SSD1306 e buzzer conectados conforme a pinagem descrita acima
- Rede Wi-Fi 2.4 GHz disponivel

### Passo 1: Clonar o repositorio

```
git clone https://github.com/natanhnrqe/epi-inteligente.git
cd epi-inteligente/firmware/esp32
```

### Passo 2: Configurar as credenciais do Wi-Fi

Abra o arquivo src/main.cpp e altere as seguintes linhas com o nome e a senha da sua rede Wi-Fi:

```cpp
const char* WIFI_SSID = "NOME_DA_SUA_REDE";
const char* WIFI_PASSWORD = "SENHA_DA_SUA_REDE";
```

### Passo 3: Fazer upload dos arquivos do dashboard (LittleFS)

Este comando envia os arquivos HTML, CSS e JavaScript da pasta data/ para a memoria flash do ESP32:

```
pio run --target uploadfs
```

### Passo 4: Compilar e enviar o firmware

```
pio run --target upload
```

### Passo 5: Verificar a conexao

Abra o monitor serial para acompanhar a inicializacao do sistema e obter o endereco IP do ESP32:

```
pio device monitor
```

O monitor exibira mensagens indicando a inicializacao de cada componente (OLED, sensor, buzzer) e, ao final, mostrara o IP atribuido ao ESP32 na rede, por exemplo:

```
Servidor HTTP iniciado!
Acesse: http://192.168.1.100
```

### Passo 6: Acessar o dashboard

Abra um navegador em qualquer dispositivo conectado a mesma rede Wi-Fi e acesse o IP exibido no monitor serial. O dashboard mostrara em tempo real os dados do acelerometro, giroscopio, nivel de bateria, estado de queda e controle do buzzer.

A pagina de cadastro do funcionario pode ser acessada pelo menu de navegacao do dashboard ou diretamente em http://<IP_DO_ESP32>/cadastro.html.

---

## Video Demonstrativo / Screenshots

**Teste 1: Esp32 Ligado
https://github.com/user-attachments/assets/a5c501eb-ed74-4942-8a20-35a1782fc362

**Teste 2: Esp32 Conectado à Rede
https://github.com/user-attachments/assets/700dbf5b-6c1f-46f5-b41f-3e41fd52da96

**Teste 3: Desligamento do Modo de Emergência
https://github.com/user-attachments/assets/08d41dc1-6129-414a-aeb6-ee199cde7824

**Teste 4: Reset dos Dados do Display
https://github.com/user-attachments/assets/e50feb9b-506d-487c-abab-8c7d61bc4b92

**Teste 5: Testes Automatizados de Conexão e Funcionamento de Componentes 

https://github.com/user-attachments/assets/99f6b883-14fd-4520-a7be-73198f82ecaa




---

## Testes Automatizados

Os testes em `firmware/esp32/test/` usam o ambiente `esp32test` do PlatformIO. Eles incluem verificacoes que dependem do hardware real, como sensor ICM-20689, display OLED e buzzer.

Com o ESP32 e os componentes conectados conforme a pinagem deste documento, execute a partir de `firmware/esp32/`:

```sh
pio test -e esp32test
```

Para apenas compilar o firmware de teste, sem enviar ou executar os testes no dispositivo:

```sh
pio test -e esp32test --without-uploading --without-testing
```

O teste do detector usa esperas reais para validar a janela de imobilidade; por isso, a execucao completa leva alguns segundos. Os testes do OLED verificam a inicializacao e exercitam o desenho das telas, mas nao validam visualmente os pixels exibidos.

---

## Estrutura do Codigo-Fonte

O firmware esta organizado em modulos dentro de firmware/esp32/src/:

- **main.cpp**: ponto de entrada do firmware. Inicializa todos os componentes e executa o loop principal.
- **sensors/Mpu6050Sensor**: encapsula a comunicacao I2C com o sensor ICM-20689 e fornece leituras de aceleracao, giroscopio e temperatura.
- **safety/FallDetector**: implementa a maquina de estados de deteccao de queda em tres fases (NORMAL, WAITING_IMMOBILITY, ALERT).
- **safety/AlertManager**: gerencia os padroes de alerta sonoro do buzzer (curto, atencao, critico).
- **actuators/Buzzer**: controla o buzzer via PWM (frequencia de 2 kHz, duty cycle de 50%).
- **displays/OledDisplay**: controla o display OLED e exibe as paginas do perfil de emergencia.
- **network/WebServerManager**: gerencia a conexao Wi-Fi, o servidor HTTP e as rotas da API REST.
- **models/EmergencyProfile**: estrutura de dados do perfil de emergencia do trabalhador (nome, tipo sanguineo, alergias, contato, observacoes).
- **models/SystemStatus**: estrutura de dados que agrega o estado completo do sistema para envio pela API.

O projeto tambem contem uma versao anterior do firmware na pasta firmware/arduino/main/main.ino, que era um sketch monolitico usado como prototipo inicial. A versao atual em firmware/esp32/ e a versao modularizada e atualizada.
