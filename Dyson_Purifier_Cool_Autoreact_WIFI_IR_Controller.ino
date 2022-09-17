#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <IRremote.hpp>
#include <ArduinoJson.h>

#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

// Minified HTML
const char UI[] PROGMEM = "<!DOCTYPE html><html><head> <title>Dyson Purifier Cool AutoReact Controller</title> <meta charset='UTF-8'> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <link href='https://fonts.googleapis.com/css?family=Roboto:100,300,400,500,700,900' rel='stylesheet'> <link href='https://cdn.jsdelivr.net/npm/@mdi/font@6.x/css/materialdesignicons.min.css' rel='stylesheet'> <link href='https://cdn.jsdelivr.net/npm/vuetify@2.x/dist/vuetify.min.css' rel='stylesheet'> <meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no, minimal-ui'> <script src='https://cdn.jsdelivr.net/npm/vue@2.7.10'></script> <script src='https://cdn.jsdelivr.net/npm/vuetify@2.x/dist/vuetify.js'></script></head><body> <div id='app'> <v-app> <v-main> <v-container fluid><template> <v-row> <v-col cols='12' sm='2' xs='4' > <v-row> <v-col cols='6' v-for='item in items' align='center'> <v-btn fab x-large :color=item.color||'primary' @click='sendCommand(item.value)'> <v-icon x-large dark>{{item.icon}}</v-icon> </v-btn> </v-col> </v-row> </v-col> </v-row> </template></v-container> </v-main> </v-app> </div><script>new Vue({el: '#app', vuetify: new Vuetify({theme:{dark: true}}), data: ()=> ({items: [{text: 'On / Off', value: 'onOff', icon: 'mdi-power-standby', color: 'red'},{text: 'Information Menu', value: 'informationMenu', icon: 'mdi-information-outline'},{text: 'Airflow Speed (+)', value: 'airflowSpeedUp', icon: 'mdi-fan-plus'},{text: 'Auto Mode', value: 'autoMode', icon: 'mdi-refresh-auto'},{text: 'Airflow Speed (-)', value: 'airflowSpeedDown', icon: 'mdi-fan-minus'},{text: 'Airflow Direction', value: 'airflowDirection', icon: 'mdi-horizontal-rotate-clockwise'},{text: 'Oscillation On / Of', value: 'oscillationOnOf', icon: 'mdi-axis-z-rotate-counterclockwise'},{text: 'Night Mode', value: 'nightMode', icon: 'mdi-weather-night', color:'blue darken-4'}]}), methods:{async sendCommand(command){try{const response=await (await fetch(`/set?command=${command}`)).text(); console.log(response);}catch (error){window.alert(error.message)}}}}) </script></body></html>";

#define IR_SEND_PIN 4

DynamicJsonDocument commands(1024);

ESP8266WebServer server(80);

void handleRoot()
{
  server.send(200, "text/html", UI);
}

void sendCommand(uint32_t command)
{
  IrSender.sendPulseDistanceWidthFromArray(38, 2250, 700, 800, 1400, 800, 700, &command, 21, PROTOCOL_IS_LSB_FIRST);
}

void requestHandler()
{
  String message = "OK";
  int command = commands[server.arg("command")];
  if (command)
  {
    sendCommand(command);
  }
  else
  {
    message = "ERROR";
  }

  server.send(200, "text/html", message);
}

void setup(void)
{
  commands["onOff"] = 0x1f0120;
  commands["airflowSpeedUp"] = 0x8d520;
  commands["airflowSpeedDown"] = 0xc3520;
  commands["oscillationOnOf"] = 0x9920;
  commands["informationMenu"] = 0x11db20;
  commands["autoMode"] = 0x182b20;
  commands["airflowDirection"] = 0xe4320;
  commands["nightMode"] = 0x1c1320;

  pinMode(LED_BUILTIN, OUTPUT);
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/set", requestHandler);

  server.begin(); // Start server
  Serial.println("HTTP server started");
}

void loop(void)
{
  server.handleClient();
}
