//BLYNK credentials
#define BLYNK_TEMPLATE_ID "YOUR TEMPLATE ID"
#define BLYNK_DEVICE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "YOUR AUTH TOKEN"
#define BLYNK_PRINT Serial

//WiFi credentials
char ssid[] = "wifiName";
char pass[] = "wifiPassword";

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

//defining pin names. Pins like V4, V6 represent the virtual pins for using with Blynk

#define DHTPIN 4 //GPIO 4 means D2 , other allowable GPIOs 3, 4, 5, 12, 13 or 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define moistPin A0
#define pumpVirtualPin V4
#define pumpPhysPin D5  //d5
#define pumpVirtualPin2 V8
#define pumpPhysPin2 D7 //d7
#define refillPumpVirtualPin V6

#define manualVirtualPin V9
#define echoPin D6
#define trigPin D1
#define waterLevelVirtualPin V2
#define minMoistureVirtualPin V11
#define maxMoistureVirtualPin V10
float hum = 0; //humidity (%)
float temp = 0; // temperature (Celsius)
int moisture1 = 0;
int moisture2 = 0;
int manualMode = 0;
float waterLevel = 0;
int refill = 1;
unsigned long previousMillis = 0;
unsigned long interval = 1000;
int minMoisture = 30;
int maxMoisture = 40;


//function to get the humidity and temperature data from DHT11 sensor
void runDHT()
{
  hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temp = dht.readTemperature();
  Serial.print("Humidity: "); Serial.println(hum);
  Serial.print("Temperature (C): "); Serial.println(temp);
  Blynk.virtualWrite(V0, temp); //update the value to blynk
  Blynk.virtualWrite(V3, hum);
}

//sync current system state to blynk user interface
void syncLoop()
{
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V8);
  Blynk.syncVirtual(V9);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
}

//function for getting soil moisture data from moisture-sensor
void runMoisture()
{
    moisture1 = analogRead(A0);
    moisture1 = map(moisture1, 1023, 0, 0, 100);
    Serial.println("Moisture");
    Serial.println(moisture1);
    Blynk.virtualWrite(V1, moisture1);
}

// function for running the pump that directly waters the plants
void runPump1()
{
  if (!manualMode)
  {
     if(moisture1 < minMoisture  )
    {
      digitalWrite(pumpPhysPin, HIGH);
      Blynk.virtualWrite(V4, 1);
      delay(200);
    }
    else if(moisture1 >=maxMoisture)
    {
      digitalWrite(pumpPhysPin, LOW);
      Blynk.virtualWrite(V4, 0);
    } 
  }
}

//function for running the pump that refills the water storage tank
void runPump2()
{
  unsigned long currentMillis = millis();
  if (refill)
  {
      if (currentMillis - previousMillis > interval) 
      {
        previousMillis = currentMillis;
        if(waterLevel < 3  )
      {
        digitalWrite(pumpPhysPin2, HIGH);
        Blynk.virtualWrite(V8, 1);
      }
      else if(waterLevel >=6)
      {
        digitalWrite(pumpPhysPin2, LOW);
        Blynk.virtualWrite(V8, 0);
      }
      } 
  }

}

//function for running the sonar sensor which measures the water level. the water container used was 13 cm high. edit it according to your system
void runSonar()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  int duration = pulseIn(echoPin, HIGH);
  float  distance_cm = duration * 0.034/2;  //sound velocity = 0.034 cm/us
  distance_cm = constrain(distance_cm, 0, 13);
  waterLevel = 13.0 - distance_cm;
  Blynk.virtualWrite(V2, waterLevel);
}


//blynk functions to communicate with node mcu and blynk user interface

BLYNK_WRITE(minMoistureVirtualPin)
{
  minMoisture = param.asInt();
  minMoisture = min(minMoisture, maxMoisture);
  maxMoisture = max(minMoisture, maxMoisture);
}
BLYNK_WRITE(maxMoistureVirtualPin)
{
  maxMoisture = param.asInt();
  minMoisture = min(minMoisture, maxMoisture);
  maxMoisture = max(minMoisture, maxMoisture);
}
BLYNK_WRITE(pumpVirtualPin)
{
  if (manualMode)
  {
    if(param.asInt() == 1)
    {
      digitalWrite(pumpPhysPin, HIGH);
    }
    else
    {
      digitalWrite(pumpPhysPin, LOW);
    }
  }
}
BLYNK_WRITE(pumpVirtualPin2)
{
  if(manualMode)
  {
    if(param.asInt() == 1)
    {
      digitalWrite(pumpPhysPin2, HIGH);
    }
    else
    {
      digitalWrite(pumpPhysPin2, LOW);
    }
  }
}
BLYNK_WRITE(manualVirtualPin)
{
  if(param.asInt() == 1)
  {
    manualMode = 1;
  }
  else
  {
    manualMode = 0;
  }  
}
BLYNK_WRITE(refillPumpVirtualPin)
{
  if(param.asInt() == 1)
  {
    refill = 1;
  }
  else
  {
    refill = 0;
  }  
}

//sync system state with blynk user interface each time it is connected

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V8);
  Blynk.syncVirtual(V9);
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V10);
  Blynk.syncVirtual(V11);
}
/////////////////////////////// REMEMBER TO KEEP THIS TIMER EVENT FUNCTION JUST OVER void setup()
BlynkTimer timer;
void myTimerEvent()
{
  syncLoop();
  runDHT();
  runSonar();  
  runMoisture();
  runPump1();
  runPump2();
}


void setup() {
  pinMode(moistPin, INPUT);
  pinMode(pumpPhysPin, OUTPUT);
  pinMode(pumpPhysPin2, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  // put your setup code here, to run once:
  dht.begin();
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, myTimerEvent);
}
void loop() {
  Blynk.run();
  timer.run();
  Serial.println(manualMode);
}
