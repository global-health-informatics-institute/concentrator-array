
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h>
#include "SoftwareI2C.h"
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
 
//declare variables used in the code
LiquidCrystal_I2C lcd= LiquidCrystal_I2C(0x27, 16, 2);
const int LpressureInput =A3;
const int HpressureInput =A6; 
const int LpressureZero =90;
const int LpressureMax =780;
const int HpressureZero =90;
const int HpressureMax =793;
const int LpressuretransducerMaxPSI = 30;
const int HpressuretransducerMaxPSI = 100;
const int sensorreadDelay = 250;
const int SSR4comp=5 ,SSR1 =2,SSR2=3,SSR3=4,SSR5=6,SSR6=7,SSR7 =8; //Solid State Relay pins
const int  sol1=11,sol2=10,sol=12; //sol1 is a relief solenoid valve,sol is the inlet valve for high pressure tant and sol2 is hp tank outlet valve

long runTime = 0; // initilizing the time that will be indicated & be used to control the concentrators when HpressureValue>50&<60
long runTime1 = 0; //initilizing the time that will be indicated & be used to control the concentrators when HpressureValue>60&<70

long previousMillis = 0;
long previousMillis1 = 0;

//declaration of time intervals for runing each set of concentrators selected
long interval = 60000; 
long interval2 = 120000;
long interval1 = 60000; 
long interval21 = 120000;
long interval3 = 180000;
long interval4 = 240000;
long interval5 = 300000;
long interval6 = 360000;
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
  pinMode(sol,OUTPUT);
  pinMode(sol1,OUTPUT);
  pinMode(sol2,OUTPUT);
  digitalWrite(SSR4comp,LOW);
  digitalWrite(sol,LOW);
  digitalWrite(sol1,LOW);
  digitalWrite(sol2,LOW);

//print warming up on bot LCD and Serial monitor to show that the hub is on warm up phase
   lcd.clear();
   lcd.setCursor(0,0);   
   lcd.println("Warming Up      ");
   Serial.println("Warming Up");
  delay(15000);
  conc_current_check();// current is measured soon after the warm up and the values are used in the void loop when HpressureInput < 70
  
}

void loop() {
  
  unsigned long currentMillis = millis();//keeps the current tracked time in the variable currentMills, used if HpressureValue>50&<60 
  unsigned long currentMillis1 = millis();//keeps the current tracked time in the variable currentMills1, used if HpressureValue>60&<70 
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
    digitalWrite(sol2,LOW);//turn off the output valve 
   }
  if(HpressureValue > 20){
    digitalWrite(sol2,HIGH);
   }
  if (LpressureValue > 4 && HpressureValue < 50){  
    compressorSol_on();}         // turn on compressor and valve
  if(LpressureValue <1){
    if (a==0 && b==0 && c==0 && d==0 && e==0 && f==0){ 
     lcd.clear();
     pressurePrinting ();
     lcd.setCursor(0,1);   
     lcd.print("Off: ");
     lcd.setCursor(5,1);   
     lcd.println("All"); 
    }
   else if(HpressureValue > 50){
    digitalWrite(SSR4comp,LOW);
    digitalWrite(sol,LOW); 
   }
   else{
   compressorSol_off();
  }}
   // turn off compressor and valve and turns on all concentrators and relief solenoid valve
  if (LpressureValue > 15 && HpressureValue <71){
    all_conc_off();
    compressorSol_on();
    delay(1000);
  }
  if (LpressureValue > 15 && HpressureValue >70){
    all_off();
  }
  if ((HpressureValue < 50) &&(LpressureValue > 1)&& (LpressureValue < 4)){
   all_on();}
   
  
 //if high pressure is greater tahn 50 but less than 61 and lowpressure is less than 8 do the following
  if((HpressureValue >50) && (HpressureValue < 61) && (LpressureValue < 4)){ 
   runTime=currentMillis - previousMillis;//time taken by left and right side concentrators when HpressureValue is greater than 50 but < 60  
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

 
 if((HpressureValue >61) && (HpressureValue < 71) && (LpressureValue < 4)){ 
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
      
    }
        
 if((HpressureValue > 50) && (HpressureValue < 70) && (LpressureValue > 4)){//switches on soleid valves if HpressureValue > 50
      compressorSol_on();  
  }

 if(HpressureValue > 70){//switches off all concentrators and compressor and then it remesures the current iif 
  
  all_off();
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
 digitalWrite(sol1,HIGH);
 delay(1000);
 digitalWrite(sol1,LOW);

 

}
void compressorSol_on(){//this function switches off all concentrators and switches on the outlet compressor
  int value= digitalRead(SSR4comp);

  if(value ==1){
    digitalWrite(SSR4comp,HIGH);
  
    } 
  else{
        digitalWrite(sol1,HIGH);
        delay(2000);
        //digitalWrite(sol1,LOW);
        digitalWrite(SSR4comp,HIGH);
        delay(500);
        
        digitalWrite(sol1,LOW);
        //delay(5000);
        //digitalWrite(SSR4comp,HIGH);
       digitalWrite(sol,HIGH);
        //digitalWrite(SSR4comp,LOW);
        
        //digitalWrite(sol1,HIGH);
        //delay(500);
        //digitalWrite(sol1,LOW);
        
      
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

  
  
 void all_off(){// all concentrators,solenoid and compressor off
  digitalWrite(SSR1,LOW);
  digitalWrite(SSR2,LOW);
  digitalWrite(SSR3,LOW);
  digitalWrite(SSR5,LOW);
  digitalWrite(SSR6,LOW);
  digitalWrite(SSR7,LOW);
  digitalWrite(SSR4comp,LOW);
  digitalWrite(sol,LOW);
  digitalWrite(f,LOW);
  digitalWrite(sol2,HIGH);
  lcd.clear();
  pressurePrinting ();
  lcd.setCursor(0,1);   
  lcd.print("off: ");
  lcd.setCursor(5,1);   
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
 
       
      
     
     
           
      
         
    
      
   
