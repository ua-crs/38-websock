/*
 *  Project 38-websock - main.cpp
 *      Mostrar que el uso de WebSockets tiene ventajas
 *      con respect a usar ervidores web en el manejo de sistemas
 *      tendientes a tiempo real
 */

#include <Arduino.h>

#ifdef ESP32

#include <WiFi.h>
#include <WebServer.h>

#define ServerObject    WebServer
// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 12;


#else

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define ServerObject    ESP8266WebServer
#define HelloMsg        "Hello from ESP8266 !"
#endif

#include <WebSocketsServer.h>

#include "wifi_ruts.h"
#include "webpage.h"

/*
 *  Constantes establecidas por platformio.ini
 *      MY_SSID     SSID del WiFi
 *      MY_PASS     Password del WiFi
 *      LED         IOPort del LED
 *      SERIAL_BAUD Baud Rate del port seria
 */

//  Creación de objetos

ServerObject server;
WebSocketsServer webSocket = WebSocketsServer(81);

/*
 *  Funciones privadas
 */

/*
 *  webSocketEvent:
 *      Callback para recepción de eventos
 */

void
webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    uint16_t brightness;
    size_t i;

    if (type == WStype_TEXT)
    {
        if (payload[0] == '#')
        {
            brightness = (uint16_t)strtol((const char *)&payload[1], NULL, 10);
            brightness = 1024 - brightness;
#ifdef ESP32
            ledcWrite(ledChannel, brightness);
#else
            analogWrite(LED, brightness);
#endif
            Serial.print("brightness= ");
            Serial.println(brightness);
        }

        else
        {
            for (i = 0; i < length; i++)
                Serial.print((char)payload[i]);
            Serial.println();
        }
    }
}

/*
 *  Funciones públicas
 */

void
setup(void)
{
    Serial.begin(SERIAL_BAUD);

#ifdef ESP32
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(LED, ledChannel);
#endif

    connect_wifi(MY_SSID, MY_PASS);

    server.on("/", []() { server.send_P(200, "text/html", webpage); });
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void
loop(void)
{
    webSocket.loop();
    server.handleClient();
    if (Serial.available() > 0)
    {
        char c[] = {(char)Serial.read()};
        webSocket.broadcastTXT(c, sizeof(c));
    }
}


