#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <dht.h>
#include <HTTPClient.h>
#define DHT11_PIN 13
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "bitmaps.h"
#include <WiFiClientSecure.h>

dht DHT;

const char* ssid = "<< WIFI SSID (NAME) >>";
const char* password = "<< WIFI PASSWORD >>";

String deviceName = "<< DESIRED DEVICE HOSTNAME >>";

String db = "<< GRAFANA DATABASE >>";
const int port = << GRAFANA TCP PORT >>;

String outdoorThermometer = "http:// << EXTERIOR DEVICE IP >> /";
String grafanaServer = "http:// << GRAFANA SERVER IP >>";

String darkSkyAPIKey = "<< DARKSKY API KEY >>";
String coords = "<< LAT, LONG LOCATION TO GET WEATHER CONDITIONS FOR >>";
String weatherLang = "<< LANGUAGE SHORTCODE >>";

IPAddress staticIP(192, 168, << SUBNET >>, 251);
IPAddress gateway(192, 168, << SUBNET >>, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

WebServer server(80);

TFT_eSPI tft = TFT_eSPI();
WiFiClient client;

const char* host = "52.3.137.27"; // darksky API IP
const int httpsPort = 443;

#define TFT_GREY 0x5AEB

float temp1,humid1;bool first = true;String icon = "", summary = "";
unsigned long previousMillis = 0, previousMillis2 = 0, previousMillis3 = 0; 
const long interval = 60000,interval2 = 1500, interval3 = 900000;
String payload;bool last14,last19,last21,last22;

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleRoot() {
  server.send(200, "text/plain", String(temp1)+","+String(humid1));
}

void setup(void){
  WiFi.disconnect();
  WiFi.setAutoConnect(false);
  Serial.begin(9600);
  tft.init();
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet);
  delay(100);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(3);
  tft.print("Connecting...");
  pinMode(14,INPUT_PULLUP);pinMode(19,INPUT_PULLUP);pinMode(21,INPUT_PULLUP);pinMode(22,INPUT_PULLUP);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void drawData(){
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE);tft.setTextFont(1);
  
  if(digitalRead(14)&&digitalRead(19)&&!digitalRead(21)&&digitalRead(22)){
    tft.setRotation(1);
    tft.println("         Inside");
    tft.println("      Temperature");
    tft.setTextFont(4);
    tft.setTextColor(TFT_YELLOW);
    tft.print("    ");tft.print(temp1);tft.println(" \xB0""C");
    tft.setTextFont(1);tft.setTextColor(TFT_WHITE);
    tft.println("        Humidity");
    tft.setTextColor(TFT_BLUE);
    tft.setTextFont(4);
    tft.print("    ");tft.print(humid1);tft.println(" %");
    tft.setTextFont(1);tft.setTextColor(TFT_WHITE);
    tft.println("        Outside");
    tft.println("      Temperature");tft.println();
    tft.setTextColor(TFT_GREY);tft.setTextFont(4);
    tft.print("    ");tft.print(payload);tft.println(" \xB0""C");
  }else if(digitalRead(14)&&!digitalRead(19)&&digitalRead(21)&&!digitalRead(22)){
    tft.setRotation(0);
    tft.println(summary);

    if(icon=="clear-night"){
        tft.pushImage(2,28,100,100,image_data_clearnight);
    }else if(icon=="cloudy"){
        tft.pushImage(2,28,100,100,image_data_cloudy);
    }else if(icon=="fog"){
        tft.pushImage(2,28,100,100,image_data_fog);
    }else if(icon=="hail"){
        tft.pushImage(2,28,100,100,image_data_hail);
    }else if(icon=="partly-cloudy-day"){
        tft.pushImage(2,28,100,100,image_data_partlycloudyday);
    }else if(icon=="partly-cloudy-night"){
        tft.pushImage(2,28,100,100,image_data_partlycloudynight);
    }else if(icon=="rain"){
        tft.pushImage(2,28,100,100,image_data_rain);
    }else if(icon=="sleet"){
        tft.pushImage(2,28,100,100,image_data_sleet);
    }else if(icon=="snow"){
        tft.pushImage(2,28,100,100,image_data_snow);
    }else if(icon=="thunderstorm"){
        tft.pushImage(2,28,100,100,image_data_thunderstorm);
    }else if(icon=="tornado"){
        tft.pushImage(2,28,100,100,image_data_tornado);
    }else if(icon=="wind"){
        tft.pushImage(2,28,100,100,image_data_wind);
    }else if(icon=="clear-day"){
        tft.pushImage(2,28,100,100,image_data_clearday);
    }

    
  }
}

void loop(void){ 
  // get current time
  unsigned long currentMillis = millis();

  // update display mode
  if(currentMillis - previousMillis2 >= interval2 || first ){
      previousMillis2 = currentMillis;

      Serial.print(digitalRead(14));
      Serial.print(digitalRead(19));
      Serial.print(digitalRead(21));
      Serial.println(digitalRead(22));

      if(( last14!=digitalRead(14) || last19!=digitalRead(19) || last21!=digitalRead(21) || last22!=digitalRead(22)) || first){
        Serial.println("Orientation changed");
        drawData();
      }

      last14 = digitalRead(14);
      last19 = digitalRead(19);
      last21 = digitalRead(21);
      last22 = digitalRead(22);
  }

  // update and send indoor/outdoor temp
  if (currentMillis - previousMillis >= interval || first ) {
    HTTPClient http2;
    previousMillis = currentMillis;
    int chk = DHT.read11(DHT11_PIN);
    Serial.print("Temperature = ");
    Serial.println(DHT.temperature);
    Serial.print("Humidity = ");
    Serial.println(DHT.humidity);
    temp1 = DHT.temperature-3;
    humid1=DHT.humidity;
    http2.begin(client,outdoorThermometer);
    http2.GET();
    payload = http2.getString();
    http2.end();
    drawData();
    if (temp1!=0&&humid1!=0) send_value(temp1,humid1);
  }

  // update darksky

  if(currentMillis - previousMillis3 >= interval3 || first ){
      previousMillis3 = currentMillis;

      WiFiClientSecure client;DynamicJsonBuffer jsonBuffer;
      Serial.print("DEBUG: connecting to ");
      Serial.println(host);
    
      if (!client.connect(host, httpsPort)) {
        Serial.println("ERROR: connection failed");
        return;
      }
    
      String url = "/forecast/"+darkSkyAPIKey+"/"+coords+"?lang="+weatherLang+"&exclude=minutely,daily,alerts,flags&units=auto";
    
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: api.darksky.net\r\n" +
                   "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.113 Safari/537.36\r\n" +
                   "Connection: close\r\n\r\n");
    
        String datax = "";
        while (client.connected()) {
          if (client.readStringUntil('\n') == "\r") {
            break;
          }
        }
        while (client.available()) {
          datax += char(client.read());
        }
        client.stop();
        datax.trim();
        datax.replace("null", "-1");
        JsonObject& root = jsonBuffer.parseObject(datax);
        JsonObject& current = root["currently"];
        JsonObject& hourly = root["hourly"];
        icon = current["icon"].asString();
        summary = String(current["summary"].asString())+", "+String(hourly["summary"].asString());
        drawData();
  }

  if(first) first=false;
  server.handleClient();
}

void send_value(float value1,float value2) {
   HTTPClient http;
   String content = "EnvironmentData,deviceType=ESP32,uid="+ String((uint32_t)ESP.getEfuseMac()) +" temp1=" + String(temp1)+",humid1="+String(humid1);

   http.begin(client,grafanaServer+":"+String(port)+"/write?db="+db);  //Specify destination for HTTP request
   http.addHeader("Content-Type", "text/plain");             //Specify content-type header
 
   int httpResponseCode = http.POST(content);   //Send the actual POST request
 
   http.end();
}
