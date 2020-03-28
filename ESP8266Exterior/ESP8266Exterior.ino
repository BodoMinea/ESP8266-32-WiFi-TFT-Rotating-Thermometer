#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DallasTemperature.h>
#include <ESP8266HTTPClient.h>

#define ONE_WIRE_BUS D2

const char* ssid = "<< WIFI SSID (NAME) >>";
const char* password = "<< WIFI PASS >>";

String deviceName = "<< DESIRED DEVICE HOSTNAME >>";
String grafanaServer = " << GRAFANA SERVER IP >> ";

String db = "<< GRAFANA DATABASE >>";
const int port = << GRAFANA TCP PORT >>;
IPAddress staticIP(192, 168, << SUBNET >>, << IP >>);
IPAddress gateway(192, 168, << SUBNET >>, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

ESP8266WebServer server(80);

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

HTTPClient http;

float temp1;bool first = true;
unsigned long previousMillis = 0; 
const long interval = 60000;

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
  server.send(200, "text/plain", String(temp1));
}

void setup(void){
  WiFi.disconnect();
  WiFi.setAutoConnect(false);
  Serial.begin(9600);
  WiFi.hostname(deviceName);
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, subnet, gateway);
  delay(100);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");

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
  
  Serial.println("Dallas Temperature IC Control");
  sensors.begin();

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){ 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval || first) {
    if(first) first=false;
    previousMillis = currentMillis;
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();
    Serial.println("DONE");
    Serial.print("Temperature for the device 1 (index 0) is: ");
    temp1=sensors.getTempCByIndex(0);
    Serial.println(temp1);
    send_value(temp1);
  }
  server.handleClient();
}

void send_value(float value) {
   String content = "EnvironmentData,deviceType=ESP8266,uid="+ String(ESP.getChipId()) +" temp1=" + String(temp1);

   http.begin("http://"+grafanaServer+":"+String(port)+"/write?db="+db);      //Specify request destination
   http.addHeader("Content-Type", "text/plain");  //Specify content-type header
 
   int httpCode = http.POST(content);   //Send the request
   String payload = http.getString();                  //Get the response payload
 
   Serial.println(httpCode);   //Print HTTP return code
 
   http.end();  //Close connection
}
