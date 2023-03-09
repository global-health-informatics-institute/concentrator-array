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
unsigned long currentMillis = millis();// //keeps the current tracked time in the variable currentMills, used if HpressureValue>50
unsigned long currentMillis1 = millis();//keeps the current tracked time in the variable currentMills1, used if HpressureValue>60&<70 

long runTime = 0; // initilizing the time that will be indicated & be used to control the concentrators when HpressureValue>50
long runTime1 = 0; //initilizing the time that will be indicated & be used to control the concentrators when HpressureValue>60&<70

long previousMillis = 0;
long previousMillis1 = 0;

long interval = 60000;
long interval1 = 60000;
long interval2 = 120000; 
long interval21 = 120000;
long interval3 = 180000;
long interval4 = 240000;
long interval5 = 300000;
long interval6 = 360000;

//declare variables for storing meassured pressure values
float LpressureValue = 0;
float HpressureValue = 0;
double Irms=0;
int currentState;

void setup() {
  // put your setup code here, to run once:
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

 
//print warming up on both LCD and Serial monitor to show that the hub is on warm up phase
   lcd.clear();
   lcd.setCursor(0,0);   
   lcd.println("Warming Up      ");
   Serial.println("Warming Up");
   delay(15000);
   conc_current_check();
   currentState = 1; 
}

void loop() {
  // put your main code here, to run repeatedly:
  float Lpressure = get_Lpressure(); //get pressure from low pressure tank
  float Hpressure = get_Hpressure(); //get pressure from high pressure tank
  pressurePrinting ();

  if (currentState == 1)
    all_on();
  else if (currentState == 2)
    all_c_running_and_mpt_filling();
  else if (currentState == 3)
    six_c_running_and_mpt_filling();
  else if (currentState == 4)
    six_concentrators_running();
  else if (currentState == 5)
    four_c_running_and_mpt_filling();
  else if (currentState == 6)
    four_concentrators_running();
  else if (currentState == 7)
    all_off();      
            
}

 void all_on(){//turns on all concentrators when called
  digitalWrite(SSR4comp,LOW);
  digitalWrite(Inletvalve,LOW);
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
  if(LpressureValue > 4)
     currentState = 2;   
  }

void all_c_running_and_mpt_filling(){
  
  //pressurePrinting ();
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
mpt_filling();
 
if(HpressureValue > 20){ 
digitalWrite(Outletvalve,HIGH);
}

if(HpressureValue < 20){
digitalWrite(Outletvalve,LOW);  
}

if(HpressureValue > 51){
currentState = 3; 
}

if(LpressureValue < 1 )
currentState = 1;  
}

void six_c_running_and_mpt_filling(){
{runTime=currentMillis - previousMillis;//time taken by left and right side concentrators when HpressureValue is greater than 50 but < 60  
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
       previousMillis = currentMillis;//stores the current time in previousMills to be used to reset the runTime 
     }
 mpt_filling();
if(HpressureValue < 50)
  currentState = 2;
  
if(HpressureValue >61) 
  currentState = 5; 
if(LpressureValue < 1 )
  currentState = 4; 
}

void six_concentrators_running(){
//digitalWrite(Reliefvalve,LOW);
digitalWrite(SSR4comp,LOW);
digitalWrite(Inletvalve,LOW);
{runTime=currentMillis - previousMillis;//time taken by left and right side concentrators when HpressureValue is greater than 50 but < 60  
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
       previousMillis = currentMillis;//stores the current time in previousMills to be used to reset the runTime 
     }
 if(LpressureValue > 4 )
  currentState = 3;     
}

void mpt_filling(){
  int value= digitalRead(SSR4comp);

  if(value ==1){
    digitalWrite(SSR4comp,HIGH);
  
    } else{
        digitalWrite(Reliefvalve,HIGH);
        delay(100);
        
   
        digitalWrite(SSR4comp,HIGH);
        delay(52200);
        digitalWrite(Reliefvalve,LOW);
        
        digitalWrite(Inletvalve,HIGH);
        
      }
}

void four_c_running_and_mpt_filling(){
 runTime1=currentMillis1 - previousMillis1;// timing for turning two sockets when prssure is between 60 and 70
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
      previousMillis1 = currentMillis; //stores the current time in previousMills1 to be used to reset the runTime1 
    }
   mpt_filling(); 
  if(HpressureValue < 60)
      currentState = 4; 
  if(HpressureValue > 70 )
      currentState = 7;    
  if(LpressureValue < 1)
      currentState = 6;    
     
}
 
void four_concentrators_running(){
 digitalWrite(SSR4comp,LOW);
 digitalWrite(Inletvalve,LOW);
 runTime1=currentMillis1 - previousMillis1;// timing for turning two sockets when prssure is between 60 and 70
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
      previousMillis1 = currentMillis; //stores the current time in previousMills1 to be used to reset the runTime1 
    }
   if(LpressureValue > 4)
      currentState = 5;      
}

void all_off(){
  
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  digitalWrite(SSR4comp,LOW);
  digitalWrite(Inletvalve,LOW);
  digitalWrite(Reliefvalve,LOW);
  digitalWrite(Outletvalve,HIGH);
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("off: ");
  lcd.setCursor(5,1);   
  lcd.println("All");
  if(HpressureValue < 70)
      currentState = 6; 
   
}

float get_Lpressure(){
  LpressureValue = analogRead(LpressureInput);//read analog pressure value in low pressure tank
  Serial.print("read Lvalue =");
  Serial.print(LpressureValue);
  Serial.print(",");
  LpressureValue = ((LpressureValue - LpressureZero)*LpressuretransducerMaxPSI)/(LpressureMax - LpressureZero);//convert pressure value to PSi
  if(LpressureValue < 0)          //set all negative pressures to zero
   LpressureValue = 0;
  Serial.print("Low output pressure =");
  Serial.print (LpressureValue,1);  
  Serial.print(",");  
  return LpressureValue;
}

float get_Hpressure(){
  HpressureValue = analogRead(HpressureInput);//read analog pressure value in high pressure
  Serial.print("read Hvalue =");
  Serial.print(HpressureValue);
  Serial.print(",");
  HpressureValue = ((HpressureValue - HpressureZero)*HpressuretransducerMaxPSI)/(HpressureMax - HpressureZero);//convert pressure value to PSI
  if(HpressureValue < 0)       //set all negative pressures to zero
    HpressureValue = 0;
  Serial.print(" High output pressure1 =");
  Serial.println(HpressureValue,1);
  return HpressureValue;
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
  lcd.print("L");
  lcd.print(LpressureValue,1);  
  lcd.print("psi");
  lcd.print(" ");
  lcd.print("H");
  lcd.print(HpressureValue,1);
  lcd.println("psi"); 
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

 







void leftHS_concentrators_on(){//will turn on three concntrators both from left that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 2 3");
}

void rightHS_concentrators_on(){//will turn on three concntrators both from right that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("5 6 7");
  }


           
  

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
     
void half_on1(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 5 6 ");
  }

 void half_on2(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("1 5 7");
  }

 void half_on3(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("1 6 7");
  }

 void half_on4(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();   
  lcd.println("2 5 6 ");
  }

 void half_on5(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();   
  lcd.println("2 5 7");
  }

 void half_on6(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("2 6 7");
  }
 void half_on7(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();  
  lcd.println("3 5 6");
  }

 void half_on8(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("3 5 7");
  }

 void half_on9(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("3 6 7");
  }

 void half_on10(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 2 5");
  }

 void half_on11(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 3 5");
  }

 void half_on12(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("2 3 5");
  }

 void half_on13(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("1 2 6");
  }

 void half_on14(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("1 3 6");
  }

 void half_on15(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting();
  lcd.println("2 3 6");
  }

 void half_on16(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting(); 
  lcd.println("1 2 7");
  }

 void half_on17(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();  
  lcd.println("1 3 7");
  }

 void half_on18(){//will turn on three concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();
  lcd.println("2 3 7");
  }
void running_concPrinting(){// it prints the positions of concentrators that are running together with the pressure at that time on LCD
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("Running: ");
  lcd.setCursor(9,1);   
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
           
  


void alt2ls_on(){// if there is no possible combination of two concentrators on the left side it will check from both sides
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

       // if no any combination of two concentrators is found it has to shift to the side with a concentrator connected 
       if((a+b+c)>0.8){
              leftHS_concentrators_on();
              break;
             } 
       if((d+e+f)>0.8){
              rightHS_concentrators_on();
              break;
             }
  } }

void alt2rs_on(){// if there is no possible combination of two concentrators on the right side it will check from both sides
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

   // if no any combination of two concentrators is found it has to shift to the side with a concentrator connected       
  if((d+e+f)>0.8){
   rightHS_concentrators_on();
   break;
   } 

  if((a+b+c)>0.8){
   leftHS_concentrators_on();
   break;
   } 
                    
}}

void left1 (){ //will turn on two concntrators both from left that are on HIGH if the function is called 
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,HIGH);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("1 2");
   } 

 void left2 (){ //will turn on two concntrators both from left that are on HIGH if the function is called 
  digitalWrite(SSR1,HIGH);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,HIGH);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  running_concPrinting();   
  lcd.println("1 3");
  }
           
void left3 (){  //will turn on two concntrators both from left that are on HIGH if the function is called    
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();   
 lcd.println("2 3");
 }

 void right1 (){//will turn on two concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,HIGH);
  digitalWrite(SSR7,LOW);
  running_concPrinting(); 
  lcd.println("5 6");
   } 

 void right2 (){//will turn on two concntrators that are on HIGH if the function is called
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,HIGH);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,HIGH);
  running_concPrinting();   
  lcd.println("5 7");
  }
           
void right3 (){ //will turn on two concntrators that are on HIGH if the function is called     
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();  
 lcd.println("6 7");
 }

void alt1 () { //will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();  
 lcd.println("1 5");
 } 

void alt2 () {//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,LOW);
 running_concPrinting();
 lcd.println("1 6");
 }

void  alt3 (){//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,HIGH);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();   
 lcd.println("1 7");
 } 

void alt4 (){//will turn on two concntrators that are on HIGH if the function is called 
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();  
 lcd.println("2 5");
  } 

void alt5 (){//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,LOW);
 running_concPrinting(); 
 lcd.println("2 6");
 } 

void  alt6 (){//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,HIGH);
 digitalWrite(SSR3,LOW);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();
 lcd.println("2 7");
 }

void  alt7 (){//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,HIGH);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,LOW);
 running_concPrinting();
 lcd.println("3 5");
 }
 
void  alt8 (){//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,HIGH);
 digitalWrite(SSR7,LOW);
 running_concPrinting();
 lcd.println("3 6");
 } 
void alt9 (){//will turn on two concntrators that are on HIGH if the function is called
 digitalWrite(SSR1,LOW);
 digitalWrite(SSR2,LOW);
 digitalWrite(SSR3,HIGH);
 digitalWrite(SSR5,LOW);
 digitalWrite(SSR6,LOW);
 digitalWrite(SSR7,HIGH);
 running_concPrinting();   
 lcd.println("3 7");
 }           
