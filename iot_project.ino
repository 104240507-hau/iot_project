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
  FuzzySet *short_m = new FuzzySet(5, 10, 20, 30);      
  FuzzySet *medium_m = new FuzzySet(25, 40, 60, 90);    
  FuzzySet *long_m = new FuzzySet(80, 120, 180, 180);   
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
  
  // Cấu hình Servo thực tế cho ESP32-S3
  ESP32PWM::allocateTimer(0); 
  servo1.setPeriodHertz(50);  
  servo1.attach(servoPin, 500, 2400); 
  
  // --- KHỞI ĐỘNG: ÉP SERVO VỀ VỊ TRÍ BAN ĐẦU LÀ 180 ĐỘ ---
  servo1.write(180);          
  delay(1000);              // Chờ 1 giây để MG996R chắc chắn xoay về góc 180 cố định trước khi chạy
  
  dht.begin();    
  setupFuzzy();   
}

void loop() {
  delay(2000); // Chờ 2 giây đọc cảm biến theo tiêu chuẩn DHT11

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Loi phan cung: Khong doc duoc DHT11! Kiem tra lai day nguon/tin hieu."));
    return; 
  }

  Serial.print(F("Do am: ")); Serial.print(h);
  Serial.print(F("% | Nhiet do: ")); Serial.print(t); Serial.println(F("°C"));

  // Tính toán logic mờ
  fuzzy->setInput(1, t); 
  fuzzy->fuzzify();      
  float output_duration = fuzzy->defuzzify(2); 

  if (output_duration < 5.0) {
    output_duration = 5.0; 
  }
  
  Serial.print(F("-> Thoi gian phun thuc te tinh tu Fuzzy: ")); Serial.print(output_duration); Serial.println(F(" giay."));

  int sensorValue = analogRead(A0);
  Serial.print("Sensor Value A0: ");
  Serial.println(sensorValue);
  
  // KIỂM TRA ĐIỀU KIỆN ĐỘ ẨM ĐỂ ĐIỀU KHIỂN VAN (SERVO)
  if (h < 60.0) { 
    Serial.println(F("--> Mo van phun suong (Quay ve goc 0)..."));
    servo1.write(0); // Quay về 0 độ để mở van
    
    // Đếm thời gian mở van theo kết quả tính toán từ Logic Mờ
    unsigned long thoi_gian_cho = output_duration * 1000;
    delay(thoi_gian_cho); 
    
    Serial.println(F("--> Het thoi gian phun. Tra van ve lai vi tri ban dau (Goc 180)."));
    servo1.write(180);  // Quay ngược lại 180 độ để đóng van
    
    Serial.println(F("--> Cho 5 giay de on dinh he thong..."));
    delay(5000); 
  } else {
    Serial.println(F("--> Du am. Giu nguyen vi tri ban dau (Góc 180)."));
    servo1.write(180); // Nếu đủ ẩm thì giữ nguyên ở 180 độ
  }
}
