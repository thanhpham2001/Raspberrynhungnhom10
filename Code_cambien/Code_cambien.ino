#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <ArduinoJson.h>
MAX30105 particleSensor;

#define MAX_BRIGHTNESS 255

void ECG(void);
void SPO2_heartRate(void);

uint32_t irBuffer[100]; //du liệu cảm biến hồng ngoại
uint32_t redBuffer[100];  //Dữ liệu của LED đỏ

int32_t bufferLength; 
int32_t spo2; //giá trị SPO2
int8_t validSPO2; //Chỉ báo hiện thị phép tính SPO2 hợp lệ
int32_t heartRate; //Giá trị Nhịp tim
int8_t validHeartRate; //Chỉ báo hiện thị phép tính nhịp tim hợp lệ
int ECG_val;

unsigned long timeSpo2;
unsigned long timeECG;
void setup()
{
  Serial.begin(115200); 
  pinMode(18, INPUT); // Setup for leads off detection LO +
  pinMode(19, INPUT); // Setup for leads off detection LO -
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Sử dụng I2C, tốc độ 400kHz
  {
    Serial.println(F("MAX30102 không tìm thấy."));
    while (1);
  }
  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Cấu hình cho cảm biến với các thông số trên

}

void loop()
{
 // Receive();
  SPO2_heartRate();
  ECG();
}

void ECG(void)
{
  timeECG=millis();
  while(1)
  {
    DynamicJsonDocument doc(1024);
    if((digitalRead(18) == 1)||(digitalRead(19) == 1)){
    Serial.println('!');
    }
    else{
    ECG_val = analogRead(4);
    //Serial.println(ECG_val);
    doc["ECG"]=ECG_val;
    serializeJson(doc, Serial);
    Serial.println();
    delay(10);
    }
    if(millis()-timeECG>60000)
    {
      Serial.println("thoat khoi ECG");
      break;
    }
  }
}

void SPO2_heartRate(void)
{
  timeSpo2=millis();
  DynamicJsonDocument doc(1024);//Tạo một chuỗi JSON
  bufferLength = 100; //Trong 4s đầu lấy 100 mẫu, 1s lấy 25 mẫu  
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) 
      particleSensor.check(); //kiểm tra dữ liệu mới của cảm biến
    redBuffer[i] = particleSensor.getRed();//set giá trị mẫu i
    irBuffer[i] = particleSensor.getIR();//set giá trị mẫu i
    particleSensor.nextSample();//Kết thúc lấy mẫu
    //Serial.print(F("red="));
    //Serial.print(redBuffer[i], DEC);
    //Serial.print(F(", ir="));
    //Serial.println(irBuffer[i], DEC);
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);//Tính toán giá trị của 100 mẫu đầu tiên  
  while (1)
  {
    //Chuyển 25 mẫu vào bộ nhớ và đẩy 75 mẫu lên trên
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
    //Lấy 25 mẫu mới
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) 
        particleSensor.check(); 
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); 
      /*Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);*/
    }
    //Tính toán lại HR và SPO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    doc["SPO2"]=spo2;
    doc["heartRate"]=heartRate;
    serializeJson(doc, Serial);
    Serial.println();
    if(millis()-timeSpo2>60000)
    {
      Serial.println("thoat khoi Sp02");
      break;
    }
  }
}

void Receive(void){
  String Data = "";
  StaticJsonDocument<200> JSON_Doc;
  if (Serial.available()) // kiem tra xem co du lieu truyen den hay khong
  {
    //Serial.readStringUntil(<kí tự dừng>) // cho phếp đọc toàn bộ chuỗi được gửi đến cho đến khi gặp kí tự dừng.
    Data = Serial.readStringUntil('\n');
    Serial.print("Data nhan: ");
    Serial.print(Data);
    
    //Xu ly JSon
    DeserializationError error = deserializeJson(JSON_Doc, Data);
    if(error)
    {
      Serial.print(F("bi loi roi() Loi la: "));
      Serial.println(error.c_str());
      return;  
    }
    else
    {
      Serial.println("JSON OK!!!");
      
      String ECG_val = JSON_Doc["ECG"];
      String SPO2_val = JSON_Doc["SPO2"];
      if(ECG_val == "1")
      {
        ECG();
      }
      if(SPO2_val == "1")
      {
        SPO2_heartRate();
      }
    
    //Clear du lieu trong mang de nhan moi-----------------------
     Data = "";  
  } 
  }
}
