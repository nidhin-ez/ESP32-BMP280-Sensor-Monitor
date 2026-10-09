#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>

#define screenWidth 128
#define screenHeight 64

unsigned long previousTime = 0;
unsigned long sensorInterval = 1000;
bool bmpReady = false;
const unsigned long retryTime = 1000;
unsigned long lastRetryTime = 0;
byte failureC =0;

Adafruit_SSD1306 display(screenWidth,screenHeight,&Wire,-1);
Adafruit_BMP280 bmp;

typedef struct{
  float temp;
  float pressure;
  float altitude;
}sensorData;

sensorData sensor = {0};

bool readSensor(sensorData* s){
  s->temp =  bmp.readTemperature();
  s->pressure = bmp.readPressure()/100.0F;
  s->altitude = bmp.readAltitude(1013.25F);
  
  if(!isfinite(s->temp)||!isfinite(s->pressure)||!isfinite(s->altitude)){
    return false;
  }
  else{
    if(s->temp < -40 || 
      s->temp > 85 || 
      s->pressure < 300 || 
      s->pressure > 1100){
      return false;
    }
    else{
      return true;
    }
  }
}


bool initializeBmp(){
  if(bmp.begin(0x77)){
    bmpReady = true;
    failureC = 0;
    displayStatus("bmp found");
    Serial.print("bmp found");
    return true;
  }
  else{
    bmpReady = false;
    displayStatus("bmp not found");
    Serial.print("bmp not found");
    return false;
  }
}

void printSensorData(const sensorData* data){
  Serial.print("temp = : ");
  Serial.print(data->temp);
  Serial.println();
  Serial.print("pressure : ");
  Serial.print(data->pressure);
  Serial.println();
  Serial.print("altitude : ");
  Serial.print(data->altitude);
}

void displayData(sensorData *data){
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("BMP280 SENSOR");

  display.setCursor(0, 18);
  display.print("Temp: ");
  display.print(data->temp);

  display.setCursor(0, 32);
  display.print("Press: ");
  display.print(data->pressure);

  display.setCursor(0, 46);
  display.print("Alt: ");
  display.print(data->altitude, 1);

  display.display();
}
void displayStatus(const char* message){
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);

    display.println("ESP32 SENSOR");
    display.println(message);
    display.display();
}

void failureCount(){
  failureC++;
    if(failureC >=3){
      bmpReady = false;
      displayStatus("bmp is unavailble");
    }
}

void setup(){
  Serial.begin(115200);
  Wire.begin(21,22);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.print("oled not found!");
    while(true){
      delay(3000);
    }
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  initializeBmp();
  
  display.display();
  }

void loop(){
  unsigned long currentTime = millis();
  
  if(!bmpReady && currentTime - lastRetryTime >= retryTime){
    lastRetryTime = currentTime;
    initializeBmp();
  }

  if (currentTime - previousTime >= sensorInterval) {
    previousTime = currentTime;
    if(bmpReady){
      if(readSensor(&sensor)){
        failureC = 0;
        printSensorData(&sensor);
        displayData(&sensor);
      }
      else{
        Serial.print("invalid readings!");
        displayStatus("invalid_readings");
        failureCount();
        
      }
    }
  }

}


