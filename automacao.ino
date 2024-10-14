#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHTesp.h>

const char *ssid = "WI-FI_CASA"; // Nome da sua rede Wi-Fi
const char *password = "12345678"; // Senha da sua rede Wi-Fi

ESP8266WebServer server(80);

#define DHTPIN D1         // Pino digital conectado ao sensor DHT
#define LED_PIN D4       // Pino digital conectado ao LED

DHTesp dht;

// Definindo um limite máximo de temperatura
const float TEMPERATURE_LIMIT = 80.0;

// Lista para armazenar as temperaturas
const int MAX_DATA_POINTS = 10; // Número máximo de pontos de dados a serem armazenados
float temperatures[MAX_DATA_POINTS];
int currentIndex = 0;

void setup() {
    dht.setup(14, DHTesp::DHT11);
    Serial.begin(115200);
    delay(10);

    // Configura o pino do LED como saída
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Certifica-se de que o LED esteja desligado inicialmente

    // Conecta à rede Wi-Fi
    Serial.println();
    Serial.print("Conectando à rede ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Conectado à rede Wi-Fi");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());

    // Iniciar o servidor
    server.on("/", HTTP_GET, handleRoot);
    server.begin();
}

void loop() {
    server.handleClient();
}

// Função para lidar com a requisição da página HTML
void handleRoot() {
    float temperatura = dht.getTemperature();
    float umidade = dht.getHumidity();

    // Adiciona a temperatura atual à lista
    temperatures[currentIndex] = temperatura;
    currentIndex = (currentIndex + 1) % MAX_DATA_POINTS;

    String paginaHTML = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><style>";
    paginaHTML += "body {background-color: #f0f0f0; font-family: Arial, sans-serif;}";
    paginaHTML += ".container {max-width: 600px; margin: 0 auto;}";
    paginaHTML += ".card {background-color: white; padding: 20px; margin-top: 20px; border-radius: 10px; box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2);}";
    paginaHTML += "button {background-color: #4CAF50; color: white; padding: 15px 30px; border: none; cursor: pointer; border-radius: 5px; font-size: 18px; margin-top: 10px;}";
    paginaHTML += "button:hover {background-color: #45a049;}";
    paginaHTML += ".titulo {background-color: #4CAF50; color: white; padding: 20px; text-align: center; margin-bottom: 20px;}";
    paginaHTML += ".botao-desligar {margin-top: 20px;}";
    paginaHTML += "</style></head><body>";
    paginaHTML += "<div class=\"container\">";
    paginaHTML += "<h1 class=\"titulo\">Automação Van</h1>";
    paginaHTML += "<div class=\"card\"><h2>Temperatura</h2><p>" + String(temperatura) + "°C</p></div>";
    paginaHTML += "<div class=\"card\"><h2>Umidade</h2><p>" + String(umidade) + "%</p></div>";
    paginaHTML += "<canvas id=\"grafico-temperatura\" width=\"400\" height=\"200\"></canvas>";
    paginaHTML += "<span id=\"nome-grafico\" style=\"position: absolute; top: 420px; left: 400px;\">Temperatura °C</span>";
    paginaHTML += "<button onclick=\"ligarLED()\">Ligar LED</button>";
    paginaHTML += "<button class=\"botao-desligar\" onclick=\"desligarLED()\">Desligar LED</button>";
    paginaHTML += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.min.js\"></script>";
    paginaHTML += "<script>";
    paginaHTML += "var ctx = document.getElementById('grafico-temperatura').getContext('2d');";
    paginaHTML += "var temperaturaChart = new Chart(ctx, {type: 'line',data: {labels: [],datasets: [{label: 'Temperatura',data: [],backgroundColor: '#FF6384',borderColor: '#FF6384',fill: false}]},options: {legend: {display: false},tooltips: {enabled: false},scales: {yAxes: [{ticks: {beginAtZero:true, max: 80}}]}}});";
    paginaHTML += "function atualizarGrafico() {";
    paginaHTML += "   temperaturaChart.data.labels = Array.from(Array(" + String(MAX_DATA_POINTS) + ").keys());";
    paginaHTML += "   temperaturaChart.data.datasets[0].data = [" + joinData() + "];";
    paginaHTML += "   temperaturaChart.update();";
    paginaHTML += "}";
    paginaHTML += "atualizarGrafico();";
    paginaHTML += "setInterval(function() { location.reload(); }, 10000);"; // Recarregar a página a cada 10 segundos
    paginaHTML += "function ligarLED(){fetch('/ligar-led');}";
    paginaHTML += "function desligarLED(){fetch('/desligar-led');}";
    paginaHTML += "</script>";
    paginaHTML += "</div></body></html>";
    server.send(200, "text/html", paginaHTML);
}

// Função para formatar os dados da lista para uso no JavaScript
String joinData() {
    String result = "";
    for (int i = currentIndex; i < MAX_DATA_POINTS + currentIndex; i++) {
        int index = i % MAX_DATA_POINTS;
        if (i != currentIndex) {
            result += ",";
        }
        result += String(temperatures[index]);
    }
    return result;
}
