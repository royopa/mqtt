/* 
 *  Projeto: Envio de temperatura e umidade para um broker mqtt no ESP32
 *  Bibliotecas usadas:
 *    Biblioteca MQTT (PubSubClient)
 *    https://github.com/knolleary/pubsubclient
 *    Biblioteca do sensor de temperatura e umidade DHT
 *    https://github.com/adafruit/DHT-sensor-library
 *    Biblioteca Adafruit Unified Sensor Library
 *    https://github.com/adafruit/Adafruit_Sensor
 *    
 */
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
 
/* Definicoes do sensor DHT */
#define DHTPIN 23     //GPIO que está ligado o pino de dados do sensor
 
#define DHTTYPE DHT11
//#define DHTTYPE DHT22   //sensor em utilização: DHT22
//#define DHTTYPE DHT21
 
/* Defines do MQTT */
#define TOPICO_PUBLISH_TEMPERATURA   "cli-mlk-002-pub/temperature" 
#define TOPICO_PUBLISH_UMIDADE       "cli-mlk-002-pub/humidity" 
 
#define ID_MQTT  "cli-mlk-002-pub"  //id cliente mqtt (para identificação de sessão)
                                    //IMPORTANTE: este deve ser único no broker (ou seja, 
                                    //            se um client MQTT tentar entrar com o mesmo 
                                    //            id de outro já conectado ao broker, o broker 
                                    //            irá fechar a conexão de um deles).
 
/* Variaveis, constantes e objetos globais */
DHT dht(DHTPIN, DHTTYPE);
 
const char* SSID = "FernandaRodrigo_2G"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "Ferodrigo2015"; // Senha da rede WI-FI que deseja se conectar
 
const char* BROKER_MQTT = "mqtt.eclipseprojects.io"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT
   
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
 
/* Prototypes */
float faz_leitura_temperatura(void);
float faz_leitura_umidade(void);
void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);
 
/*
 * Implementações
 */
 
/* Função: faz a leitura de temperatura (sensor DHT)
 * Parametros: nenhum
 * Retorno: temperatura (graus Celsius)
 */
float faz_leitura_temperatura(void)
{
    float t = dht.readTemperature();
    float result;
     
    if (! (isnan(t)) )
        result = t;
    else
        result = -99.99;
 
    return result;
}
 
/* Função: faz a leitura de umidade relativa do ar (sensor DHT)
 * Parametros: nenhum
 * Retorno: umidade (0 - 100%)
 */
float faz_leitura_umidade(void)
{
    float h = dht.readHumidity();    
    float result;
     
    if (! (isnan(h)) )
        result = h;
    else
        result = -99.99;
 
    return result;
}
 
/* Função: inicializa e conecta-se na rede WI-FI desejada
*  Parâmetros: nenhum
*  Retorno: nenhum
*/
void initWiFi(void) 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
      
    reconnectWiFi();
}
 
/* Função: inicializa parâmetros de conexão MQTT(endereço do
 *         broker, porta e seta função de callback)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void initMQTT(void) 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
}

/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 *         em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnectMQTT(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.println();
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentativa de conexao em 2s");
            delay(2000);
        }
    }
}
 
/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
      
     reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnectWiFi(void) 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
          
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
      
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.print(" - IP obtido: ");
    Serial.print(WiFi.localIP());
}
  
/* Função de setup */
void setup() 
{
    Serial.begin(115200);  
 
    /* Inicializacao do sensor de temperatura */
    dht.begin();  
 
    /* Inicializa a conexao wi-fi */
    initWiFi();
 
    /* Inicializa a conexao ao broker MQTT */
    initMQTT();
}
 
/* Loop principal */
void loop() 
{
    char temperatura_str[10] = {0};
    char umidade_str[10]     = {0};
     
    /* garante funcionamento das conexões WiFi e ao broker MQTT */
    VerificaConexoesWiFIEMQTT();
 
    /* Compoe as strings a serem enviadas pro dashboard (campos texto) */
    sprintf(temperatura_str,"%.2f", faz_leitura_temperatura());
    sprintf(umidade_str,"%.2f", faz_leitura_umidade());
 
    /*  Envia as strings ao dashboard MQTT */
    MQTT.publish(TOPICO_PUBLISH_TEMPERATURA, temperatura_str);
    MQTT.publish(TOPICO_PUBLISH_UMIDADE, umidade_str);
   
    /* keep-alive da comunicação com broker MQTT */
    MQTT.loop();
 
    /* Refaz o ciclo após 10 segundos */
    delay(10000);
}
