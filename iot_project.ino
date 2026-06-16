#include "DHT.h"
#include <ESP32Servo.h> 
#include <Fuzzy.h>

#define DHTPIN 4       
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

static const int servoPin = 5; 
Servo servo1;

Fuzzy *fuzzy = new Fuzzy();

void setupFuzzy() {
  FuzzyInput *temp = new FuzzyInput(1);
  FuzzySet *cool = new FuzzySet(0, 0, 20, 25);
  FuzzySet *warm = new FuzzySet(22, 26, 28, 32);
  FuzzySet *hot = new FuzzySet(30, 35, 50, 50);
  temp->addFuzzySet(cool); temp->addFuzzySet(warm); temp->addFuzzySet(hot);
  fuzzy->addFuzzyInput(temp);


  FuzzyOutput *duration = new FuzzyOutput(2);
  FuzzySet *short_m = new FuzzySet(5, 10, 20, 30);      // Thực tế: Phun ngắn từ 5 - 30 giây
  FuzzySet *medium_m = new FuzzySet(25, 40, 60, 90);    // Phun vừa từ 25 - 90 giây
  FuzzySet *long_m = new FuzzySet(80, 120, 180, 180);   // Phun dài từ 80 - 180 giây (3 phút)
  duration->addFuzzySet(short_m); duration->addFuzzySet(medium_m); duration->addFuzzySet(long_m);
  fuzzy->addFuzzyOutput(duration);

 
  FuzzyRuleAntecedent *ifTempHot = new FuzzyRuleAntecedent(); 
  ifTempHot->joinSingle(hot); 
  FuzzyRuleConsequent *thenLongMist = new FuzzyRuleConsequent(); 
  thenLongMist->addOutput(long_m); 
  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, ifTempHot, thenLongMist);
  fuzzy->addFuzzyRule(fuzzyRule1);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("He thong DHT11 + Chuan chan 4 dang khoi dong..."));
  
  // Cấu hình Servo thực tế
  ESP32PWM::allocateTimer(0); 
  servo1.setPeriodHertz(50);  
  servo1.attach(servoPin, 500, 2400); 
  
  servo1.write(0); 5
  
  dht.begin();    
  setupFuzzy();   
}

void loop() {
  delay(2000); // Chờ 2 giây đọc cảm biến

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Loi phan cung: Khong doc duoc DHT22! Kiem tra lai day nguon/tin hieu."));
    return; 
  }

  Serial.print(F("Do am: ")); Serial.print(h);
  Serial.print(F("% | Nhiet do: ")); Serial.print(t); Serial.println(F("°C"));

  fuzzy->setInput(1, t); 
  fuzzy->fuzzify();      
  float output_duration = fuzzy->defuzzify(2); 


  if (output_duration < 5.0) {
    output_duration = 5.0; 
  }
  
  Serial.print(F("-> Thoi gian phun thuc te: ")); Serial.print(output_duration); Serial.println(F(" giay."));


  if (h < 60.0) { 
    Serial.println(F("--> Mo van phun suong..."));
    servo1.write(90); 
    
    delay(output_duration * 1000); 
    
    servo1.write(0);  
    Serial.println(F("--> Dong van."));
    
  
    delay(5000); 
  } else {
    Serial.println(F("--> Du am. Dong van."));
    servo1.write(0);
  }
}
