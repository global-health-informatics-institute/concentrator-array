#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h>
#include "SoftwareI2C.h"
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
 
//declare variables used in the code
LiquidCrystal_I2C lcd= LiquidCrystal_I2C(0x27, 16, 2);
const int LpressureInput =A3; //pin on arduino nano
const int HpressureInput =A6; //pin on arduino nano
const int LpressureZero =90; //values for calibrating the sensor
const int LpressureMax =780; //values for calibrating the sensor
const int HpressureZero =90; //values for calibrating the sensor
const int HpressureMax =793; //values for calibrating the sensor
const int LpressuretransducerMaxPSI = 30; //pressure sensor on the low pressure tank
const int HpressuretransducerMaxPSI = 100;//pressure sensor on the High pressure tank
const int sensorreadDelay = 250;
const int SSR4comp=5 ,SSR1 =2,SSR2=3,SSR3=4,SSR5=6,SSR6=7,SSR7 =8; //Solid State Relay pins, SSR control power in the sockets
const int  Reliefvalve=11,Outletvalve=10,Inletvalve=12; //relief solenoid valve for the compressor,inletvalve from L.P to H.P and outletvalve from H.p to outlet

float a,b,c,d,e,f,g;//declaring variable in which current measured values will be stored

//declare variables for storing meassured pressure values
float LpressureValue = 0;
float HpressureValue = 0;
double Irms=0;

void setup() {// put your setup code here, to run once:
  
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  //setting SSR, sol,LpressureInput,HpressureInput and pins as output pins
  pinMode(SSR1,OUTPUT); 
  pinMode(SSR2,OUTPUT);
  pinMode(SSR3,OUTPUT);
  pinMode(SSR5,OUTPUT);
  pinMode(SSR6,OUTPUT);
  pinMode(SSR7,OUTPUT);
  pinMode(SSR4comp,OUTPUT);
  pinMode(LpressureInput,INPUT);
  pinMode(HpressureInput,INPUT);
  pinMode(Inletvalve,OUTPUT);
  pinMode(Reliefvalve,OUTPUT);
  pinMode(Outletvalve,OUTPUT);
  digitalWrite(SSR4comp,LOW);
  digitalWrite(Inletvalve,LOW);
  digitalWrite(Reliefvalve,LOW);
  digitalWrite(Outletvalve,LOW);

//print warming up on bot LCD and Serial monitor to show that the hub is on warm up phase
   lcd.clear();
   lcd.setCursor(0,0);   
   lcd.println("Warming Up      ");
   Serial.println("Warming Up");
  delay(15000);
  conc_current_check();// current is measured soon after the warm up and the values are used in the void loop when HpressureInput < 70
  
}

  


void loop() {
  LpressureValue = analogRead(LpressureInput);//read analog pressure value in low pressure tank
  Serial.print("read Lvalue =");
  Serial.print(LpressureValue);
  Serial.print(",");
  HpressureValue = analogRead(HpressureInput);//read analog pressure value in high pressure thak
  Serial.print("read Hvalue =");
  Serial.print(HpressureValue);
  Serial.print(",");
  LpressureValue = ((LpressureValue - LpressureZero)*LpressuretransducerMaxPSI)/(LpressureMax - LpressureZero);//convert pressure value to PSi
  if(LpressureValue < 0)          //set all negative pressures to zero
    LpressureValue = 0;
  Serial.print("Low output pressure =");
  Serial.print (LpressureValue,1);  
  Serial.print(",");
  HpressureValue = ((HpressureValue - HpressureZero)*HpressuretransducerMaxPSI)/(HpressureMax - HpressureZero);//convert pressure value to PSI
  if(HpressureValue < 0)       //set all negative pressures to zero
    HpressureValue = 0;
  Serial.print(" High output pressure1 =");
  Serial.println(HpressureValue,1);
  if(HpressureValue < 20){ 
    all_on();
    if(LpressureValue > 4){
     digitalWrite(Reliefvalve,HIGH); 
     delay(500);  
     digitalWrite(SSR4comp,HIGH);
        
     digitalWrite(Reliefvalve,LOW);
     digitalWrite(Inletvalve,HIGH); 
    
    digitalWrite(Outletvalve,LOW);//turn off the output valve 
   }
    else if(LpressureValue <1){
    digitalWrite(SSR4comp,LOW);
    digitalWrite(Inletvalve,LOW); 
   }
  }
  if(HpressureValue > 20 && HpressureValue < 50){
  digitalWrite(Outletvalve,HIGH);
  all_on();
   
   if(LpressureValue > 4){
     digitalWrite(Reliefvalve,HIGH);
     delay(500);   
     digitalWrite(SSR4comp,HIGH);
        
     digitalWrite(Reliefvalve,LOW);
     digitalWrite(Inletvalve,HIGH); 
    
    digitalWrite(Outletvalve,LOW);//turn off the output valve 
   }
    else if(LpressureValue <1){
    digitalWrite(SSR4comp,LOW);
    digitalWrite(Inletvalve,LOW); 
   }
  }
}
  void all_on(){//turns on all concentrators when called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5, HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Running: ");
  lcd.setCursor(9,1);   
  lcd.println("All");
  }
void all_conc_off(){// turns off all concentrators
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW); 
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Off: ");
  lcd.setCursor(5,1);   
  lcd.println("All");
 }

void conc_current_check(){// measures current drawn by a concentrator and stores the value in a variable
 
  all_conc_off(); 
  emon1.current(2, 111.1);             // Current: input pin, calibration.
  digitalWrite(SSR1,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
  digitalWrite(SSR1,LOW);
//   if (Irms>0.8){
//    g=Irms; 
//    }
//   else{
//     g=0;} 

  digitalWrite(SSR1,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480);  // Calculate Irms only on socket 1 and keeps the value in variable a
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Current on 1:");
  lcd.setCursor(13,1);   
  lcd.println(Irms);
  digitalWrite(SSR1,LOW);
   if (Irms>0.8){
    a=Irms; 
    }
   else{
     a=0;}
     
  digitalWrite(SSR2,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480); // Calculate Irms only on socket 2 and keeps the value in variable b
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Current on 2:");
  lcd.setCursor(13,1);   
  lcd.println(Irms);
  digitalWrite(SSR2,LOW);
   if (Irms>0.8){
    b=Irms; 
   }
   else{
    b=0;}

  digitalWrite(SSR3,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480);  // Calculate Irms only on socket 3 and keeps the value in variable c
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Current on 3:");
  lcd.setCursor(13,1);   
  lcd.println(Irms);
  digitalWrite(SSR3,LOW);
   if (Irms>0.8){
    c=Irms; 
   }
   else{
    c=0;}

  digitalWrite(SSR5,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480);  // Calculate Irms only on socket 5 and keeps the value in variable d
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Current on 5:");
  lcd.setCursor(13,1);   
  lcd.println(Irms);
  digitalWrite(SSR5,LOW);
   if (Irms>0.8){
    d=Irms; 
   }
   else{
    d=0;}

  digitalWrite(SSR6,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480); // Calculate Irms only on socket 6 and keeps the value in variable e
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Current on 6:");
  lcd.setCursor(13,1);   
  lcd.println(Irms);
  digitalWrite(SSR6,LOW);
   if (Irms>0.8){
    e=Irms; 
   }
   else{
    e=0;}
  digitalWrite(SSR7,HIGH);
  delay(700);
  Irms = emon1.calcIrms(1480);  // Calculate Irms only on socket 7 and keeps the value in variable f
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Current on 7:");
  lcd.setCursor(13,1);   
  lcd.println(Irms);
  digitalWrite(SSR7,LOW);
   if (Irms>0.8){
    f=Irms; 
   }
   else{
    f=0;}
// print on serial monitor the stored current values
  Serial.print(a);
  Serial.print(", ");
  Serial.print(b);
  Serial.print(", ");
  Serial.print(c);
  Serial.print(", ");
  Serial.print(d);
  Serial.print(", ");
  Serial.print(e);
  Serial.print(", ");
  Serial.print(f);
  Serial.println(", ");
  //Serial.println(g); 

 }
 void pressurePrinting (){//it prints tanks pressure at a particular time on LCD
  lcd.setCursor(0,0);   
  lcd.print("LP");
  lcd.print(LpressureValue,1);  
  lcd.print("psi");
  lcd.print("H");
  lcd.print(HpressureValue,1);
  lcd.println("psi"); 
  } 
void running_concPrinting(){// it prints the positions of concentrators that are running together with the pressure at that time on LCD
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Running: ");
  lcd.setCursor(9,1);   
  }
