
#include "Wire.h"

//==================Definicja pinów==================//
#define joyX A0
#define joyY A1
#define guzik1 7
#define guzik2 9
#define reset_button 16
//===================================================//

//=============Przyciski oraz joystick===============//

float sens = 15.00;
float mouse_sens = 0.1;
//===================================================//

//=============Wyjścia dla MPU6050===================//
char up ='w';// KEY_UP_ARROW;
char down ='s';// KEY_DOWN_ARROW;
char right ='a';// KEY_RIGHT_ARROW;
char left ='d';// KEY_LEFT_ARROW;
//===================================================//

//=====================Zmienne=======================//
const int adres = 0x68;
float gyroX,gyroY,gyroZ;
float osX,osY,osZ;
float accel_osX,accel_osY,accel_osZ;
float sumX,sumY,sumZ;
float accX,accY,accZ;
float offsetX,offsetY,offsetZ;
float acc_offsetX,acc_offsetY,acc_offsetZ;
float stopnieX,stopnieY,stopnieZ;
float acc_stopnieX,acc_stopnieY,acc_stopnieZ;
float accelX,accelY,accelZ;
float liczba;
float time,p_time;
float roznica;
float k_wartoscX,k_wartoscY,k_wartoscZ;
int i=0;
//===================================================//

void gyro_signals(void){
  //==========Ustawienie czułości żyroskopu============//
  Wire.beginTransmission(adres);
  Wire.write(0x1B); 
  Wire.write(0x8); //ustawiamy czułość żyrskopu na +- 500 deg/s
  Wire.endTransmission();
  //===================================================//

  //============Rządanie danych z rejestrów============//
  Wire.beginTransmission(adres);
  Wire.write(0x43); 
  Wire.endTransmission();
  Wire.requestFrom(adres,6); //otwierając komunikacje z pamięcią, żądamy 6 BAJTÓW informacji, czyli odczyty dla każdej osi
  //===================================================//

  //=============Surowe wartość z MPU6050==============//
  int16_t gyroX=Wire.read()<<8 | Wire.read(); //Zczytujemy wartośći, przsuwamy o 8 BITÓW i zapisauemy jako liczbę 16-bitową
  int16_t gyroY=Wire.read()<<8 | Wire.read();
  int16_t gyroZ=Wire.read()<<8 | Wire.read();
  //===================================================//

  //========Prędkość kątowa w stopniach/sek============//
  osX=(gyroX/65.5) + 0.05214 ;//obliczamy obrot żyroskopu w Stopniach/sekunde
  osY=(gyroY/65.5) - 1.924715; //POGGERS//
  osZ=(gyroZ/65.5) + 0.324768; //zostawia
  //===================================================//

  //=============Reset driftu żyroskopu================//
  if(digitalRead(reset_button)==LOW){
    stopnieX=0;
    stopnieY=0;
    stopnieZ=0;    
  }
  else{
    stopnieX+=(osX*roznica);
    stopnieY+=(osY*roznica);
    stopnieZ+=(osZ*roznica);
  }
  //===================================================//
}
void accel_signals(void){
  //=========Ustawienie czułości akcelerometru========//
  Wire.beginTransmission(adres);
  Wire.write(0x1C);
  Wire.write(0x8);
  Wire.endTransmission();
  //===================================================//

  //============Rządanie danych z rejestrów============//
  Wire.beginTransmission(adres);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(adres,6);
  //===================================================//

  //==============Przyśpieszenie w m/s^2===============//  
  accelX=(Wire.read()<<8 | Wire.read() / 8192) - 256;
  accelY=(Wire.read()<<8 | Wire.read() / 8192) + 7;
  accelZ=(Wire.read()<<8 | Wire.read() / 8192);
  //===================================================//

  //==========Aktualna pozycja w stopniach=============//
  acc_stopnieX=(atan(accelY/sqrt(pow(accelX,2)+pow(accelZ,2)))*180/PI);
  acc_stopnieY=(atan(-1 *accelX/sqrt(pow(accelY,2)+pow(accelZ,2)))*180/PI);
  acc_stopnieZ=(atan(sqrt(pow(accelX,2)+pow(accelY,2))/accelZ)*180/PI);
  //===================================================//
}
void gyro_error(void){
  while(i<200){
    //==========Ustawienie czułości żyroskopu============//
    Wire.beginTransmission(adres);
    Wire.write(0x1B);
    Wire.write(0x8); 
    Wire.endTransmission();
    //===================================================//    

    //============Rządanie danych z rejestrów============//
    Wire.beginTransmission(adres);
    Wire.write(0x43); 
    Wire.endTransmission();
    Wire.requestFrom(adres,6); 
    //===================================================//  
    
    //=============Surowe wartość z MPU6050==============//
    int16_t gyroX=Wire.read()<<8 | Wire.read();
    int16_t gyroY=Wire.read()<<8 | Wire.read();
    int16_t gyroZ=Wire.read()<<8 | Wire.read();

    //========Prędkość kątowa w stopniach/sek============//
    osX=(gyroX/65.5);
    osY=(gyroY/65.5); 
    osZ=(gyroZ/65.5); 

    delay(1);
    //=============Sumy odczytów z żyroskopu=============//
    sumX+=osX;
    sumY+=osY;
    sumZ+=osZ;

    i++;
    //===================================================//
  } 
  //============Błędy z pomiarów żyroskopu=============//
  offsetX = sumX/200;
  offsetY = sumY/200;
  offsetZ = sumZ/200;
  //===================================================//
  i=0;
  while(i<1000){
    Wire.beginTransmission(adres);
    Wire.write(0x1C);
    Wire.write(0x8);
    Wire.endTransmission();

    Wire.beginTransmission(adres);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(adres,6);

    accelX=Wire.read()<<8 | Wire.read() / 8192;
    accelY=Wire.read()<<8 | Wire.read() / 8192;
    accelZ=Wire.read()<<8 | Wire.read() / 8192;

    delay(1);

    //Sumy odczytów z akcelerometru//
    accX+=accelX;
    accY+=accelY;
    accZ+=accelZ; 

    i++;
  }
  //============Błędy z pomiarów akcelerometru=============//
  acc_offsetX = accX / 1000;
  acc_offsetY = accY / 1000; 
  //=======================================================//
  i=0;  
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  //===================================================//
  pinMode(3,OUTPUT);
  pinMode(guzik1,INPUT_PULLUP);
  pinMode(guzik2,INPUT_PULLUP);
  pinMode(reset_button,INPUT_PULLUP);
  //===================================================//
  digitalWrite(3,HIGH);
  Wire.setClock(400000); //Czestotliwość podana przez producenta;
  Wire.begin();
  delay(250); //dajemy czas aby mpu6050 wystartowało
  Wire.beginTransmission(adres);
  Wire.write(0x6B); // rozpoczynamy transmisje z rejestrem zarządzania energią
  Wire.write(0x00); // ustawiająć ten rejestr na 0 umożliwia start i kontynuowanie pracy
  Wire.endTransmission();

  //==================*Błąd gyroskopu*=================//  
  gyro_error();
  //===================================================//
  Serial.print("  Gyro error X: ");
  Serial.print(offsetX);
  Serial.print("  Gyro error Y: ");
  Serial.print(offsetY);
  Serial.print("  Gyro error Z: ");
  Serial.print(offsetZ);
  Serial.print("  Accel error X: ");
  Serial.print(acc_offsetX);
  Serial.print("  Accel error Y: ");
  Serial.print(acc_offsetY);
  Serial.println();
  //===================================================//

}
void loop() {
  while(!Serial){
    delay(10);
  }

  //==========Odczyty z przycisków i joysticka=========//
  int xValue = map(analogRead(A0),0,1023,-512,512);
  int yValue = map(analogRead(A1),0,1023,-512,512);
  int stan1 = digitalRead(guzik1); 
  int stan2 = digitalRead(guzik2);  
  //==============*Czas pomiędzy odczytami*============//  
  p_time=time;
  time=millis();
  roznica=(time - p_time)/1000;
  //===================================================//

  //============*Wartości z gyro/accel*================//
  gyro_signals();
  accel_signals();
  //===================================================//

  //============*Complementary filter*=================//  
  k_wartoscX=0.85*stopnieX + 0.15*acc_stopnieX;
  k_wartoscY=0.85*stopnieY + 0.15*acc_stopnieY;  
  k_wartoscZ=0.85*stopnieZ + 0.15*acc_stopnieZ;
  //===================================================//
  delay(10);
  //==========Wyświetlanie wartości====================//
  Serial.print(k_wartoscX);
  Serial.print("  Stopnie y: ");
  Serial.print(k_wartoscY);
  Serial.print("  Stopnie y: ");
  Serial.print(k_wartoscZ);
  Serial.println();
  //==================================================//

  //===========Output odczytów z MPU6050==============/
  
} 
              