#include <LoRa.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>


#define ss 5    //8
#define rst 14  //9
#define dio0 2  //7
#define NUM_SLAVES 1

String temperature;
String humidity;
String readingID;
String formattedDate;
String hour;
String day;
String timestamp;
String latitud;
String longitud;
String altitude;
String rumbo;
String velocidad;
String satelites;

void WiFiStationEvent(WiFiEvent_t event, WiFiEventInfo_t info);

static const char* ssid = "Estacion terrena";
static const char* password = "cansat2023";

WiFiServer client(80);
AsyncWebServer server(80);

AsyncEventSource events("/events");

unsigned long lastTime = 0;  
unsigned long timerDelay = 30000;
//se carga la pagina en el codigo en formato html y se le agrega css para darle estilo a la pagina
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
  *{
    padding: 0;
    margin: 2px;
  box-sizing: border-box;  
}

.inicio{
  text-align: center;
  padding: 0.5rem 0.5rem 0.5rem 0.5rem;
  margin: 1rem;
  display: flex;
  flex-direction: row;
  align-items: center;
  border: 2px solid #eb9c5d;
  justify-content: space-evenly;
  background-color: #eb9c5b;
  border-radius: 5px;
}

.gps{
    padding: 0.5rem;
    margin: 1rem;
    display: flex;
    flex-direction: row;
    align-items: center;
    border: 2px;justify-content: space-evenly;
   
}
section{
    padding: 1px 1px 1px 1px;
}

.seleccion .titulo{
    padding: 40px;
    border: 2px solid #8080804d;
    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: space-evenly;
    background-color: #eb9c5b;
    border-radius: 5px;
}
main .seleccion input{
    width: 7rem;
    height: 1rem;
   
}

.contenido {
    padding: 1px;
    align-items: center;
    justify-content: space-evenly;
}
main button{
background-color: white;
height: 25px;
width: 10rem;
border: 1px solid #eb9c5b;
border-radius: 6px;
}
main button:hover{
    border: 1px solid #8080804d;
    box-shadow: 0 0.5px 1px 0 #000001;
    cursor: pointer;
}
</Style>
</head>

<body>
<main class="content">
    <h1 class="inicio">Cansat </h1>
    <section class="seleccion"> 
        <div class="titulo">
            <h2 class="gps">DHT11 </h2>
            <div class="contenido">
                <p class="dht">Temperatura: <span id="temperature" class="readings">%TEMPERATURE%</span></p>
                <p class="dht">Humedad: <span id="humidity" class="readings>%HUMIDITY%</span></p>
            </div>
        </div>

    <div class="titulo">  
        <h2 class="gps">GPS </h2>
        <div class="contenido">
            <p><strong>Last received packet: <br/><span id="timestamp">%TIMESTAMP%</span></strong></p>
            <p class="date">Fecha: <input type="text"><span id="day" + /  + id="month" + / + id="year" class="readings></span></p>
            <p class="date">Hora: <input type="text"><span id="minute" + : + id="second" class"readings"></span></p>
            <p class="posicion">Latitud: <input type="text"> <span id="latitud" class="readings"></span></p>
            <p class="posicion">Longitud: <input type="text"><span id="longitud" class="readings"></span></p>
            <p class="posicion">Altitud: <input type="text"><span id="gps.f_altitude class="readings"">mtrs</span></p>
            <p class="posicion">Rumbo: <input type="text"><span id="gps.f_course" class"readings"></span>° </p>
            <p class="posicion">Velocidad: <input type="text"><span id="gps.f_speed:kmph" class="readings"> kmph</span> </p>
            <p class="posicion">Satelites: <input type="text"><span id="gps.satellites" class="readings"></span> </p>
        </div>
    </div>

    <script>

    setInterval(updateValues, 10000, "temperature");
    setInterval(updateValues, 10000, "humidity");
    setInterval(updateValue, 10000, "timestamp");

   function updateValues(value) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById(value).innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/" + value, true);
  xhttp.send();
    </script>
    <div>
        <button>Revisar conexión</button>
        <button>Despegue</button>
    </div>
    </section>
</main>
</body>
</html>
)rawliteral";

String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){

    return temperature;
  }
  else if(var == "HUMIDITY"){
    return humidity;
  }
  else if(var == "TIMESTAMP"){
    return timestamp;
  }
  return String();
}


void WiFiAPEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  // Handling function code
  if (event == ARDUINO_EVENT_WIFI_AP_START) {
    Serial.println("AP Started");
  }
  else if (event == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
    Serial.println("Client connected");
  }
  else if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
    Serial.println("Client disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Receiver");

  LoRa.setPins(ss, rst, dio0);  //setup LoRa transceiver module

  while (!LoRa.begin(433E6))  //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");

  /* WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);
  Serial.print("Iniciando Estación terrana:\t");
  Serial.println(ssid); 
  WiFi.begin(serverssid, serverpassword);
  // Esperar a que nos conectemos
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);
  Serial.print('.');
  }


  // Mostrar mensaje de exito y dirección IP asignada
 */
  Serial.println("Configurando estación terrena");
  WiFi.onEvent(WiFiAPEvent, ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(WiFiAPEvent, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(WiFiAPEvent, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP SSID: ");
  Serial.println(ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  // Handle Web Server
 /* if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  };*/

  //se establecen los parametros de entrada que van a ser mandados a la pagina web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperature.c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", humidity.c_str());
  });
  server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", timestamp.c_str());
  });
   server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", latitud.c_str());
  });
   server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", longitud.c_str());
  });
   server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", altitude.c_str());
  });
   server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", rumbo.c_str());
  });
   server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", velocidad.c_str());
  });
   server.on("/timestamp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", satelites.c_str());
  });

  // Start server
  server.begin();
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second

  
}

void getLoRaData(){
  Serial.print("Lora packet received: ");

  while (LoRa.available()){
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData);
    //se les designa una posicion dependiendo el cuadro en que se desea se introduzcan los valores
    int pos1 = LoRaData.indexOf('#');
    int pos2 = LoRaData.indexOf('&');
    int pos3 = LoRaData.indexOf('%');
    int pos4 = LoRaData.indexOf('!');
    int pos5 = LoRaData.indexOf('?');
    int pos6 = LoRaData.indexOf('¿');
    int pos7 = LoRaData.indexOf('¡');
    int pos8 = LoRaData.indexOf('$');
    readingID = LoRaData.substring(0, pos1);
    temperature = LoRaData.substring(pos1 + 1, pos2);
    humidity = LoRaData.substring(pos2 + 1, pos3);
    latitud = LoRaData.substring(pos3 + 1, pos4);
    longitud = LoRaData.substring(pos4 + 1, pos5);
    altitude = LoRaData.substring(pos5 + 1, pos6);
    rumbo = LoRaData.substring(pos6 + 1, pos7);
    velocidad = LoRaData.substring(pos7 + 1, pos8);
    satelites = LoRaData.substring(pos8 + 1, LoRaData.length())

  }
}
void loop() {
  //esta paqueteria permite la entrada de los datos de un lora al otro
  int packetSize = LoRa.parsePacket();  // try to parse packet
  if (packetSize) {
    getLoRaData();
    // Serial.print("Received packet '");

   /* while (LoRa.available())  // read packet
    {
     // String LoRaData = LoRa.readString();
      //Serial.print(LoRaData);

      String LoRaLabel = LoRa.readStringUntil(':');
      if (LoRaLabel.endsWith("Temperature:"));
      String temperature = LoRa.readStringUntil(',');
      if (LoRaLabel.endsWith("Humidity:"));
      String humidity = LoRa.readStringUntil(',');
      

         Serial.print("Temperature:");
         Serial.println(temperature);
         Serial.print("Humidity:");
         Serial.println(humidity);
       
        
    }*/
    //Serial.print("' with RSSI ");         // print RSSI of packet
    //Serial.println(LoRa.packetRssi());
  }

  /*if ((millis() - lastTime) > timerDelay) {
  }*/
}