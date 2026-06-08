#include "DHT.h"
#include <ESP32Servo.h>
#include <Fuzzy.h>





#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

static const int servoPin = 13;
Servo servo1;

Fuzzy *fuzzy = new Fuzzy();
void setupFuzzy() {
  // Khai báo biến đầu vào: Nhiệt độ (Temp)
  FuzzyInput *temp = new FuzzyInput(1);
  FuzzySet *cool = new FuzzySet(0, 0, 20, 25);
  FuzzySet *warm = new FuzzySet(22, 26, 28, 32);
  FuzzySet *hot = new FuzzySet(30, 35, 50, 50);
  temp->addFuzzySet(cool); temp->addFuzzySet(warm); temp->addFuzzySet(hot);
  fuzzy->addFuzzyInput(temp);

  // Khai báo biến đầu ra: Thời gian phun (Duration)
  FuzzyOutput *duration = new FuzzyOutput(1);
  FuzzySet *short_mist = new FuzzySet(0, 0, 30, 60);     // 0 - 1 phút
  FuzzySet *medium_mist = new FuzzySet(45, 90, 120, 150); // 1.5 - 2.5 phút
  FuzzySet *long_mist = new FuzzySet(120, 180, 300, 300); // 3 - 5 phút
  duration->addFuzzySet(short_mist); duration->addFuzzySet(medium_mist); duration->addFuzzySet(long_mist);
  fuzzy->addFuzzyOutput(duration);

  // Định nghĩa Luật mờ (Rules)
  // RULE 1: Nếu Temp là HOT -> Phun LONG
  FuzzyRuleAntagonist *ifTempHot = new FuzzyRuleAntagonist(); 
  ifTempHot->withFuzzySet(hot);
  FuzzyRuleConsequent *thenLongMist = new FuzzyRuleConsequent(); 
  thenLongMist->withFuzzySet(long_mist);
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifTempHot, thenLongMist);
  fuzzy->addFuzzyRule(fuzzyRule1);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  servo1.attach(servoPin);

  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  bool State = false;
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  if(h >=50){
    State = true;
  }
  if (State == true){
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
    servo1.write(posDegrees);
    Serial.println(posDegrees);
    delay(2000);
   }

  for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
    servo1.write(posDegrees);
    Serial.println(posDegrees);
    delay(2000);
    }
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.println();
}