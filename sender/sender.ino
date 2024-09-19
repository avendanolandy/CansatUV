#include <LoRa.h>
#include <SPI.h>
#include "DHT.h"
#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>


 
#define ss 8//5
#define rst 9//14
#define dio0 7//2
#define DHTTYPE DHT11
void dthsensor();
void gpssensor();

int counter = 0;
String LoRaMessage = "";
int readingID = 0;
const int DHTPin = 4;     
DHT dht(DHTPin, DHTTYPE);
TinyGPS gps;
SoftwareSerial serialgps(1,0);

//int year;
//byte month, day, hour, minute, second, hundredths;
unsigned long chars;
unsigned short sentences, failed_cheksum;
char dato=' ';
int year = 0;
float month = 0;
float day = 0;
float hour = 0;
float minute = 0; 
float second = 0;
float hundredths = 0;
float humidity = 0;
float temperature = 0;


void setup()    
{
  Serial.begin(115200);
  serialgps.begin(9600); 
  while (!Serial);
  Serial.println("LoRa Sender");
  Serial.println("DHTxx test!");
  Serial.println("--Buscando señal--");
  dht.begin();
 
  LoRa.setPins(ss, rst, dio0);    //setup LoRa transceiver module
  
 while (!LoRa.begin(433E6))     
  {
    Serial.println(".");
    counter++;
    delay(500);
  }
  if (counter == 10){
    readingID++;
    Serial.println("Starting LoRa failed");
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Sending packet");
}
 
void loop() 
{
  // Wait a few seconds between measurements.
  
  //Serial.println("Sending packet: ");
    dthsensor();
    delay(500);
    gpssensor();
    //delay(500);
    delay(1000);
}

void dthsensor(){
  //Serial.println(counter);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    //Serial.print(", ");
    Serial.print(humidity);
    //Serial.print(", ");
    Serial.print(temperature);
    return;
  }

  
  Serial.print(F("Temperature:"));
  Serial.print(temperature);
  Serial.println(F("*C"));
  Serial.print(F("Humidity:"));
  Serial.print(humidity);
  Serial.println(F("%\t"));

   LoRaMessage = String(readingID) + "#" + String(temperature) + "&" + String(humidity) ;
  LoRa.beginPacket();   //Send LoRa packet to receiver
  LoRa.print(LoRaMessage);
   /*LoRa.print("Temperature: ");
   LoRa.print(t);
   LoRa.print("*C");
   LoRa.println(",");
   LoRa.print("Humidity: ");
   LoRa.print(h);
   LoRa.print("%\t");
   LoRa.print(",");*/
   LoRa.endPacket();

   Serial.print("Sending packet:");
   Serial.println(readingID);
   readingID++;
   Serial.println(LoRaMessage);
   delay(1000);
}

void gpssensor(){
  /*if(serialgps.available())
  {
    dato=serialgps.read();
    LoRa.print(dato);
    Serial.print(dato);
  }*/
 while(serialgps.available())
 {
  int c = serialgps.read();
  if(gps.encode(c))
  {
    float latitud, longitud;
     gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
    gps.f_get_position(&latitud, &longitud);
    LoRa.beginPacket();
    LoRa.print("Latitud/Longitud");
    LoRa.print(latitud, 5);
    LoRa.print(longitud, 5);

    //gps.crack_datetime(&año, &mes, &dia, &hora, &minutos, &segundos, &centesimas);
    LoRa.print("Fecha: "); LoRa.print(day, DEC); LoRa.print("/");
    LoRa.print(month, DEC); LoRa.print("/"); LoRa.print(year);
    LoRa.print(minute, DEC); LoRa.print(":"); LoRa.print(second, DEC);
    LoRa.print("."); LoRa.println(hundredths, DEC);
    LoRa.print(",");
    LoRa.print("Altitud(metros): ");
    LoRa.println(gps.f_altitude());
    LoRa.print(",");
    LoRa.print("Rumbo(grados): "); LoRa.println(gps.f_course());
    LoRa.print(",");
    LoRa.print("Velocidad (kmph): ");
    LoRa.println(gps.f_speed_kmph());
    LoRa.print(",");  
    LoRa.print("Satelites: "); LoRa.println(gps.satellites());
    LoRa.print(",");
    LoRa.endPacket();
    gps.stats(&chars, &sentences, &failed_cheksum);

    Serial.print("Latitud/Longitud");
    Serial.print(latitud, 5);
    Serial.print(longitud, 5);

    //gps.crack_datetime(&año, &mes, &dia, &hora, &minutos, &segundos, &centesimas);
    Serial.print("Fecha: "); Serial.print(day, DEC); Serial.print("/");
    Serial.print(month, DEC); Serial.print("/"); Serial.print(year);
    Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC);
    Serial.print("."); Serial.println(hundredths, DEC);
    Serial.print("Altitud(metros): ");
    Serial.println(gps.f_altitude());
    Serial.print("Rumbo(grados): "); Serial.println(gps.f_course());
    Serial.print("Velocidad (kmph): ");
    Serial.println(gps.f_speed_kmph());  
    Serial.print("Satelites: "); Serial.println(gps.satellites());
    gps.stats(&chars, &sentences, &failed_cheksum);

    delay(2000);
  }
  
 }
 
}