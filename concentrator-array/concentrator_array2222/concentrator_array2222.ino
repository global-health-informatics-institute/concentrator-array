#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h>
#include "SoftwareI2C.h"
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
 

LiquidCrystal_I2C lcd= LiquidCrystal_I2C(0x27, 16, 2);
//SoftwareI2C softwarei2c;

const int LpressureInput =A3;
const int HpressureInput =A6; 
const int LpressureZero =90;
const int LpressureMax =780;
const int HpressureZero =90;
const int HpressureMax =793;
const int LpressuretransducerMaxPSI = 30;
const int HpressuretransducerMaxPSI = 100;
const int sensorreadDelay = 250;
const int SSR4comp=5 ,SSR1 =2,SSR2=3,SSR3=4,SSR5=6,SSR6=7,SSR7 =8, sol=12,sol1=11; //Solid State Relay pins

long runTime = 0;
long previousMillis = 0;
long interval = 60000; 
long interval2 = 120000;


long runTime1 = 0;
long previousMillis1 = 0;
long interval1 = 60000; 
long interval21 = 120000;
long interval3 = 180000;
long interval4 = 240000;
long interval5 = 300000;
long interval6 = 360000;
float a,b,c,d,e,f,g;
//PIN D9 and D10

float LpressureValue = 0;
float HpressureValue = 0;
double Irms;


void setup() {// put your setup code here, to run once:
  
  Serial.begin(9600);
  
  //softwarei2c.begin(4, 5);       // sda, scl
  lcd.init();
  lcd.backlight();
  pinMode(SSR1,OUTPUT); //set SSR pins as output pins
  pinMode(SSR2,OUTPUT);
  pinMode(SSR3,OUTPUT);
  pinMode(SSR5,OUTPUT);
  pinMode(SSR6,OUTPUT);
  pinMode(SSR7,OUTPUT);
  pinMode(SSR4comp,OUTPUT);
  pinMode(LpressureInput,INPUT);
  pinMode(HpressureInput,INPUT);
  pinMode(sol,OUTPUT);
  pinMode(sol1,OUTPUT);
  digitalWrite(SSR4comp,LOW);
  digitalWrite(sol,LOW);
  digitalWrite(sol1,LOW);

   lcd.clear();
   lcd.setCursor(0,0);   
   lcd.println("Warming Up      ");
   Serial.println("Warming Up");
  delay(15000);
  conc_current_check();
  
}

void loop() {
  
  unsigned long currentMillis = millis();
  unsigned long currentMillis1 = millis();
  LpressureValue = analogRead(LpressureInput);//read pressure value
  Serial.print("read Lvalue =");
  Serial.print(LpressureValue);
  Serial.print(",");
  HpressureValue = analogRead(HpressureInput);//read pressure value
  Serial.print("read Hvalue =");
  Serial.print(HpressureValue);
  Serial.print(",");
  LpressureValue = ((LpressureValue - LpressureZero)*LpressuretransducerMaxPSI)/(LpressureMax - LpressureZero);//convert pressure value to PSi
  Serial.print("Low output pressure =");
  Serial.print (LpressureValue,1);  
  Serial.print(",");
  HpressureValue = ((HpressureValue - HpressureZero)*HpressuretransducerMaxPSI)/(HpressureMax - HpressureZero);//convert pressure value to PSI
  Serial.print(" High output pressure1 =");
  Serial.println(HpressureValue,1);
  if(LpressureValue < 0)          //set all negative pressures to zero
    LpressureValue = 0;
  if(HpressureValue < 0)       //set all negative pressures to zero
    HpressureValue = 0;
  if (LpressureValue > 8 && HpressureValue < 50){  
    compressorSol_on();}         // turn on compressor and valve
  if(LpressureValue <4)
   compressorSol_off();         // turn off compressor and valve and turns on all concentrators and relief solenoid valve
  if (LpressureValue > 10 && HpressureValue <71){
    all_conc_off();
    compressorSol_on();
    delay(1000);
  }
  if (LpressureValue > 10 && HpressureValue >70){
    all_off();
  }
  if ((HpressureValue < 50) &&(LpressureValue > 4)&& (LpressureValue < 8)){
   all_on();}
   
  
 //if high pressure is greater tahn 50 but less than 61 and lowpressure is less than 8 do the following
  if((HpressureValue >50) && (HpressureValue < 61) && (LpressureValue < 8)){ 
   runTime=currentMillis - previousMillis;//time taken by left and right side concentrators when HpressureValue is greater than 50 but < 70  
   Serial.print("RunTime ="); 
   Serial.println(runTime,1); 
   if(runTime < interval) { 
      if(a>0.8 && b>0.8 && c>0.8){// if all concentrators on the left side are connected
       leftHS_concentrators_on();
      }
      else{
        alt_left_on ();
      }}
   else{
   if((runTime > interval) && (runTime < interval2)) {
      if(d>0.8 && e>0.8 && f>0.8){ //if all concentrators on the right side are connected
       rightHS_concentrators_on();
      }
      else{
        alt_right_on ();
      }}}}
 if(runTime > interval2){
       previousMillis = currentMillis; 
     }

 
 if((HpressureValue >61) && (HpressureValue < 71) && (LpressureValue < 8)){ 
 runTime1=currentMillis1 - previousMillis1;// timing for turing two sockets whwn prssure is between 60 and 70
 Serial.print("RunTime1 ="); 
 Serial.println(runTime1,1);    
    if(runTime1 < interval1){
      if (a>0.8 && b>0.8){
          left1 ();
         }
      else{
         alt2ls_on();
          }}

     if(runTime1 > interval1 && runTime1 < interval21 ){
       if (a>0.8 && c>0.8){
          left2 ();
          }
      
      else{
        alt2ls_on();
      }}
    
    if(runTime1 > interval21 && runTime1 < interval3 ){
    
       if (b>0.8 && c>0.8){
          left3 ();
          }
      
      else{
        alt2ls_on();
      }}

    if(runTime1 > interval3 && runTime1 < interval4 ){
      if (d>0.8 && e>0.8){
          right1 ();
         }
      else{
         alt2rs_on();
          }}

     if(runTime1 > interval4 && runTime1 < interval5 ){
       if (d>0.8 && f>0.8){
          right2 ();
          }
      
      else{
        alt2rs_on();
      }}
    if(runTime1 > interval5 && runTime1 < interval6 ){
    
       if (e>0.8 && f>0.8){
          right3 ();
          }
      
      else{
        alt2rs_on();
      }}
   if(runTime1 > interval6 ){
      previousMillis1 = currentMillis;  
    }
      
    }
        
 if((HpressureValue > 50) && (HpressureValue < 70) && (LpressureValue > 8)){
      compressorSol_on();  
  }

 if(HpressureValue > 70){
  
  all_off();// turm off all concentrators, compressor and valve
  conc_current_check();
  }
delay(sensorreadDelay);
}
void compressorSol_off(){ //this function switches on all concentrators and switches off the outlet compressor
  
 digitalWrite(SSR4comp,LOW);
 digitalWrite(sol,LOW);
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,HIGH);
 lcd.clear();
 pressurePrinting ();
 lcd.setCursor(0,1);   
 lcd.print("Running: ");
 lcd.setCursor(9,1);   
 lcd.println("All");
 //delay(2000);
 //digitalWrite(sol1,HIGH);
 //delay(5000);
 //digitalWrite(sol1,LOW);

 

}
void compressorSol_on(){//this function switches off all concentrators and switches on the outlet compressor

 digitalWrite(sol1,HIGH);
 delay(1000);
 digitalWrite(sol1,LOW);
 digitalWrite(SSR4comp,HIGH);
 digitalWrite(sol,HIGH);

 
 
}
void all_on(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Running: ");
  lcd.setCursor(9,1);   
  lcd.println("All");
  }
void leftHS_concentrators_on(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 2 3");
}

 void rightHS_concentrators_on(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("5 6 7");
  }

 void half_on1(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 5 6 ");
  }

 void half_on2(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("1 5 7");
  }

 void half_on3(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("1 6 7");
  }

 void half_on4(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();   
  lcd.println("2 5 6 ");
  }

 void half_on5(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();   
  lcd.println("2 5 7");
  }

 void half_on6(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("2 6 7");
  }
 void half_on7(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();  
  lcd.println("3 5 6");
  }

 void half_on8(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("3 5 7");
  }

 void half_on9(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("3 6 7");
  }

 void half_on10(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 2 5");
  }

 void half_on11(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 3 5");
  }

 void half_on12(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("2 3 5");
  }

 void half_on13(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("1 2 6");
  }

 void half_on14(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 3 6");
  }

 void half_on15(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("2 3 6");
  }

 void half_on16(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("1 2 7");
  }

 void half_on17(){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();  
  lcd.println("1 3 7");
  }

 void half_on18(){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("2 3 7");
  }

  
  
 void all_off(){// all concentrators,solenoid and compressor off
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  digitalWrite(SSR4comp,LOW);
  digitalWrite(sol,LOW);
  digitalWrite(sol1,LOW);
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("off: ");
  lcd.setCursor(5,1);   
  lcd.println("All");
  
  }
 void all_conc_off(){
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

 void left1 (){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("1 2");
   } 

 void left2 (){
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();   
  lcd.println("1 3");
  }
           
void left3 (){      
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();   
 lcd.println("2 3");
 }

 void right1 (){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("5 6");
   } 

 void right2 (){
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();   
  lcd.println("5 7");
  }
           
void right3 (){      
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();  
 lcd.println("6 7");
 }

void alt1 () { 
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();  
 lcd.println("1 5");
 } 

void alt2 () {
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,LOW);
 running_concPrinting();
 lcd.println("1 6");
 }

void  alt3 (){
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();   
 lcd.println("1 7");
 } 

void alt4 (){ 
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();  
 lcd.println("2 5");
  } 

void alt5 (){
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,LOW);
 running_concPrinting(); 
 lcd.println("2 6");
 } 

void  alt6 (){
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();
 lcd.println("2 7");
 }

void  alt7 (){
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();
 lcd.println("3 5");
 }
 
void  alt8 (){
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,LOW);
 running_concPrinting();
 lcd.println("3 6");
 } 
void alt9 (){
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();   
 lcd.println("3 7");
 }  
  
 
void conc_current_check(){// measres current drawn by a concentrator and stores the value in a variable
 
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
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
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
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
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
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
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
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
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
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
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
  Irms = emon1.calcIrms(1480);  // Calculate Irms only
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

void alt_left_on (){ // select other alternative to turn on if no concentrator is connected on some socckets on the left side
  for(int i=0;i<10;i++){// if some concentrators on the left side are not connected, select the connected from both sides
          if(a>0.8 && d>0.8 && e>0.8){
             half_on1 ();
             break;
          }
          if(a>0.8 && d>0.8 && f>0.8){
             half_on2 ();
             break;
           }
          if(a>0.8 && e>0.8 && f>0.8){
              half_on3 ();
              break;
           }
          if(b>0.8 && d>0.8 && e>0.8){
              half_on4 ();
              break;
             }
          if(b>0.8 && d>0.8 && f>0.8){
              half_on5 ();
              break;
             }
          if(b>0.8 && e>0.8 && f>0.8){
              half_on6 ();
              break;
             }
          if(c>0.8 && d>0.8 && e>0.8){
              half_on7 ();
              break;
             }
          if(c>0.8 && d>0.8 && f>0.8){
             half_on8 ();
             break;
           }
          if(c>0.8 && e>0.8 && f>0.8){
             half_on9 ();
             break;
           }
          if(d>0.8 && a>0.8 && b>0.8){
              half_on10 ();
              break;
             }

          if(d>0.8 && a>0.8 && c>0.8){
              half_on11 ();
              break;
             } 

          if(d>0.8 && b>0.8 && c>0.8){
              half_on12 ();
              break;
             }
          if(e>0.8 && a>0.8 && b>0.8){
             half_on13 ();
             break;
              }

          if(e>0.8 && a>0.8 && c>0.8){
             half_on14 ();
             break;
             }
          if(e>0.8 && b>0.8 && c>0.8){
             half_on15 ();
             break;
            }

          if(f>0.8 && a>0.8 && b>0.8){
              half_on16 ();
              break;
             }
          if(f>0.8 && a>0.8 && c>0.8){
              half_on17 ();
              break;
              }
           if(f>0.8 && b>0.8 && c>0.8){
              half_on18 ();
              break;
             }
           if((a+b+c)>0.8){
              leftHS_concentrators_on();
              break;
             }
           if((d+e+f)>0.8){
              rightHS_concentrators_on();
              break;
             }
             
        }}
           
  

void alt_right_on (){ // select other alternative to turn on if no concentrator is connected on some socckets on the right side
   for(int i=0;i<10;i++){// if some concentrators on the right side are not connected, select the connected from both sides
          if(d>0.8 && a>0.8 && b>0.8){
              half_on10 ();
              break;
             }

          if(d>0.8 && a>0.8 && c>0.8){
              half_on11 ();
              break;
             } 

          if(d>0.8 && b>0.8 && c>0.8){
              half_on12 ();
              break;
             }
          if(e>0.8 && a>0.8 && b>0.8){
             half_on13 ();
             break;
              }

          if(e>0.8 && a>0.8 && c>0.8){
             half_on14 ();
             break;
             }
          if(e>0.8 && b>0.8 && c>0.8){
             half_on15 ();
             break;
            }

          if(f>0.8 && a>0.8 && b>0.8){
              half_on16 ();
              break;
             }
          if(f>0.8 && a>0.8 && c>0.8){
              half_on17 ();
              break;
              }
          if(f>0.8 && b>0.8 && c>0.8){
              half_on18 ();
              break;
             }
          if(a>0.8 && d>0.8 && e>0.8){
             half_on1 ();
             break;
          }
          if(a>0.8 && d>0.8 && f>0.8){
             half_on2 ();
             break;
           }
          if(a>0.8 && e>0.8 && f>0.8){
              half_on3 ();
              break;
           }
          if(b>0.8 && d>0.8 && e>0.8){
              half_on4 ();
              break;
             }
          if(b>0.8 && d>0.8 && f>0.8){
              half_on5 ();
              break;
             }
          if(b>0.8 && e>0.8 && f>0.8){
              half_on6 ();
              break;
             }
          if(c>0.8 && d>0.8 && e>0.8){
              half_on7 ();
              break;
             }
          if(c>0.8 && d>0.8 && f>0.8){
             half_on8 ();
             break;
           }
          if(c>0.8 && e>0.8 && f>0.8){
             half_on9 ();
             break;
           }
           
         if((d+e+f)>0.8){
              rightHS_concentrators_on();
              break;
             }
         if((a+b+c)>0.8){
              leftHS_concentrators_on();
              break;
             }
           
        }
      }
     
void alt2ls_on(){
  for (int soc=1;soc<3; soc++){
        if (a>0.8 && d>0.8){
          alt1 ();
          break;
         } 
        if (a>0.8 && e>0.8){
          alt2 ();
          break;
        }
        
        if (a>0.8 && f>0.8){
           alt3 ();
           break;
        }
        if (b>0.8 && d>0.8){
          alt4 ();
          break;
          }
       if (b>0.8 && e>0.8){
        alt5 ();
        break;
       }
       if (b>0.8 && f>0.8){
         alt6 ();
        break;
       }
       if (c>0.8 && d>0.8){
         alt7 ();
        break;
       }
           
       if (c>0.8 && e>0.8){
         alt8 ();
        break; 
       }
       if (c>0.8 && f>0.8){
         alt9 ();
         break;
      }

       if (runTime1< interval1) {
         if (d>0.8 && e>0.8){
           right1 ();
           break;
          }
         if (d>0.8 && f>0.8){
           right2 ();
           break;
          }
         if (e>0.8 && f>0.8){
           right3 ();
           break;
         }      
          }

        if (runTime1> interval1 && runTime1 < interval21) {
           if (d>0.8 && f>0.8){
             right2 ();
             break;
           }
           if (d>0.8 && e>0.8){
             right1 ();
             break;
           }
           if (e>0.8 && f>0.8){
             right3 ();
             break;
           }      
          }
          if (runTime1> interval21 && runTime1 < interval3) {
           if (e>0.8 && f>0.8){
             right3 ();
             break;
           }      
           if (d>0.8 && f>0.8){
             right2 ();
             break;
           }
           if (d>0.8 && e>0.8){
             right1 ();
             break;
           }
          }
       
       if((a+b+c)>0.8){
              leftHS_concentrators_on();
              break;
             } 
       if((d+e+f)>0.8){
              rightHS_concentrators_on();
              break;
             }
  } }

void alt2rs_on(){
 for (int soc=1;soc<3; soc++){
  if (c>0.8 && f>0.8){
         alt9 ();
         break;
      }      

   if (c>0.8 && e>0.8){
         alt8 ();
         break;
         }

   if (c>0.8 && d>0.8){
         alt7 ();
         break;      
       }

   if (b>0.8 && f>0.8){
         alt6 ();
        break;
       }       
   if (b>0.8 && e>0.8){
        alt5 ();
        break;
       }      

  if (b>0.8 && d>0.8){
          alt4 ();
          break;
          }   
  if (a>0.8 && f>0.8){
           alt3 ();
           break;
        }

  if (a>0.8 && e>0.8){
          alt2 ();
          break;
        }        
  if (a>0.8 && d>0.8){
          alt1 ();
          break;
         } 
  if (runTime1> interval3 && runTime1 < interval4){
   if (a>0.8 && b>0.8){
  
          left1 ();
           break;
         }
    if (a>0.8 && c>0.8){
          left2 ();
           break;
          }
    if (b>0.8 && c>0.8){
          left3 ();
           break;
          }}
  if (runTime1> interval4 && runTime1 < interval5) {
    if (a>0.8 && c>0.8){
          left2 ();
           break;}
    if (a>0.8 && b>0.8){
          left1 ();
           break;
         }
    if (a>0.8 && d>0.8){
          alt1 ();
          break;
         }}
          
   if (runTime1> interval5 && runTime1 < interval6) {
    if (b>0.8 && c>0.8){
          left3 ();
           break;
          }
    if (a>0.8 && c>0.8){
          left2 ();
           break;
          }
    if (a>0.8 && b>0.8){
         left1 ();
           break;
         }      
          }

          
  if((d+e+f)>0.8){
   rightHS_concentrators_on();
   break;
   } 

  if((a+b+c)>0.8){
   leftHS_concentrators_on();
   break;
   } 
                    
}}       
        
void pressurePrinting (){
  lcd.setCursor(0,0);   
  lcd.print("LP");
  lcd.print(LpressureValue,1);  
  lcd.print("psi");
  lcd.print("H");
  lcd.print(HpressureValue,1);
  lcd.println("psi"); 
  } 
void running_concPrinting(){
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Running: ");
  lcd.setCursor(9,1);   
  }
 
       
      
     
     
           
      
         
    
      
   
