#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <String.h>




//LiquidCrystal lcd(12,11,5,4,3,6);
//12 -->7
//11  --> 8
//5  --> A3
//4  --> A2
//3  --> A1
//6  --> A0
LiquidCrystal lcd (7,8,A3,A2,A1,A0);


//pin 1 from H Bridge enables or disables motor
//int HBridge1=9;
int HBridge1=5;
//pin 2 and 7 from H Bridge make motor turn right or left or stop depending on the following combinations
/*
2 High 7 Low --> Motor runs to the right
2 Low 7 High --> Motor runs to the left
any other combination the motor does not move
*/
int HBridge2=10;
int HBridge7=13;
int button=2;
int buttonPressed=0;
unsigned long wateringIntervalMillis=604800000;
unsigned long wateringIntervalMillisPeriodic=604800000; 
unsigned long wateringIntervalMillisFirst=604800000+14400000; //14400000
unsigned long estimatedMillisToWater=10000; 
unsigned long lcdRefresh=500;//half second
unsigned long currentMillis=0;
unsigned long previousMillis=0;
unsigned long wateringPreviousMillis=0;
unsigned long lcdCurrentMillis=0;
unsigned long lcdPreviousMillis=0;
unsigned long lastButtonMillis=0;
unsigned long currentButtonMillis=0;
unsigned long doubleClickMillis=500;
unsigned long measuringStart=0;
unsigned long measuringEnd=0;
unsigned long resetButtonCounterMillis=3000;
int status=0;
unsigned long buttonPressedArray[2]={0,0};
volatile unsigned long debounceMicros=150000;
volatile unsigned long currentButtonMicros=0;
volatile unsigned long lastButtonMicros=0;
volatile unsigned long firstButtonMicros=0;
volatile unsigned long secondButtonMicros=0;
volatile unsigned long doubleClickMicros=1000000;//1 second
//status 0 is normal mode
//status 1 is measuring mode
int buttonPressedCounter=0;
String lcdLineOne = "1234567890123456";
String lcdLineTwo = "1234567890123456";
int eepromAdress=0;
int timesWatered=0;


void setup() {
Serial.begin(9600);
delay(2000);
Serial.println("Starting..");
pinMode(HBridge7,OUTPUT);
pinMode(HBridge2,OUTPUT);
pinMode(HBridge1,OUTPUT);
pinMode(button,INPUT);
attachInterrupt(0,buttonPress,RISING);

//Startup LCD stating length and number of columns
lcdLineOne="Jarvis";
lcdLineTwo="Water Pump";
lcd.begin(16,2);
lcd.setCursor(0,0);
lcd.print(lcdLineOne);
lcd.setCursor(0,1);
lcd.print(lcdLineTwo);

unsigned long eepromValue= 0;
EEPROM.get(eepromAdress,eepromValue);

if(eepromValue != 4294967145){
   estimatedMillisToWater=eepromValue;
   Serial.print("overwritten estimatedMillistoWater with eepromValue ");
   Serial.println(eepromValue); //4294967145
}

}

void loop() {

 /* Serial.println("-------------------------------- LOOP ----------------------------------------");
   Serial.print("buttonPressedCounter ");
   Serial.println(buttonPressedCounter);
   Serial.print("lastButtonMillis ");
   Serial.println(lastButtonMillis);
   Serial.print("currentButtonMillis ");
   Serial.println(currentButtonMillis);
   */
  if(timesWatered==0){
     wateringIntervalMillis=wateringIntervalMillisFirst;
  }else{
    wateringIntervalMillis=wateringIntervalMillisPeriodic;
  }
  currentMillis= millis();
  lcdCurrentMillis= millis();
  if (currentMillis - previousMillis > wateringIntervalMillis){
    //Serial.print("currentMillis - previousMillis");
    //Serial.print(currentMillis - previousMillis);
    //Serial.print("wateringIntervalMillis");
    //Serial.print(wateringIntervalMillis);
    
    previousMillis = currentMillis; 
    timesWatered++;
    waterPlant();
  }
  if (lcdCurrentMillis - lcdPreviousMillis > lcdRefresh){
     lcdPreviousMillis = lcdCurrentMillis;
     lcdLineOne="Jarvis OWM";
     lcdLineOne.concat("");
     lcdLineTwo="     ";
     lcdLineTwo.concat(TimeToString2((wateringIntervalMillis - (currentMillis - previousMillis))/1000));
     //lcdLineTwo="Water Pump";

     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print(lcdLineOne);
     lcd.setCursor(0,1);
     lcd.print(lcdLineTwo);
     //Serial.println("Refreshing LCD ");
  }  
//  buttonPressed= digitalRead(button);
/*  if(buttonPressed){
     buttonPressedCounter++;
     Serial.print("Button Counter ");
     Serial.println(buttonPressedCounter);
     lastButtonMillis=currentButtonMillis;
     currentButtonMillis=millis();
     //lastButtonMillis=millis();
  }else{
     //Serial.println("Button not Pressed");
     lcd.setCursor(15,1);
     lcd.print(" ");  
  }

  currentButtonMillis=millis();
  */  
  if (buttonPressedCounter>0){ 
     delay(2000);//wait two seconds for the click to be single or double    
     Serial.print("buttonPressedCounter ");
     Serial.println(buttonPressedCounter);
     Serial.print("secondButtonMicros ");
     Serial.println(secondButtonMicros);
     Serial.print("firstButtonMicros ");
     Serial.println(firstButtonMicros);
     Serial.print("doubleClickMicros ");
     Serial.println(doubleClickMicros);
     Serial.print(firstButtonMicros);
     Serial.print(" - ");
     Serial.print(secondButtonMicros);
     Serial.print(" = ");
     Serial.println(firstButtonMicros-secondButtonMicros);
     if((firstButtonMicros - secondButtonMicros < doubleClickMicros)){ //if distance between clicks is less than one second
        Serial.println("doubleClick ");
        //double click
        measuringStart=millis();
        status=1;
        lcd.clear();
        lcd.begin(16,2);
        lcd.print("Jarvis");
        lcd.setCursor(0,1);
        lcd.print("Water Pump");
        lcd.setCursor(14,1);
        lcd.print("MW");
        lcd.setCursor(7,0);
        lcd.print("Measuring     ");
        
        Serial.println("Measuring");
        digitalWrite(HBridge1,HIGH);
        digitalWrite(HBridge7,HIGH);
        digitalWrite(HBridge2,LOW);
        buttonPressedCounter=0;
        lcd.clear();
        lcd.begin(16,2);
        lcd.print("Jarvis");
        lcd.setCursor(0,1);
        lcd.print("Water Pump");
        lcd.setCursor(14,1);
        lcd.print("MW");
        lcd.setCursor(7,0);
        lcd.print("Measuring     ");
        while(buttonPressedCounter==0){
         Serial.println("Measuring");  
         delay(lcdRefresh);
        }
     }else{
       Serial.println("single Click ");
        //single click
       if(status==0){
         Serial.println("Status 0 ");
         //normal mode
         Serial.println("Button Pressed");
         lcd.setCursor(15,1);
         lcd.print("B");
         waterPlant();
       }else if (status == 1){
         Serial.println("Status 1 ");
         
         //measuring mode 
         measuringEnd=millis();
         estimatedMillisToWater=measuringEnd-measuringStart;
         EEPROM.put(eepromAdress,estimatedMillisToWater);
         Serial.print("Writing to EEPROM ");
         Serial.print(estimatedMillisToWater);
         lcd.clear();
         lcd.begin(16,2);
         lcd.print("Jarvis");
         lcd.setCursor(0,1);
         lcd.print("Water Pump");
         lcd.setCursor(14,1);
         lcd.print("MD");
         lcd.setCursor(7,0);
         lcd.print("Draining     ");

         boolean full=true;
          //drain plant
         digitalWrite(HBridge1,HIGH);
         digitalWrite(HBridge7,LOW);
         digitalWrite(HBridge2,HIGH);
         while(full){
            currentMillis= millis();
            if (currentMillis-measuringEnd > estimatedMillisToWater){
              full=false;
            }
            delay(1000);            
         }
         digitalWrite(HBridge1,LOW);
         status=0;
       }
     }
     //lastButtonMillis = currentButtonMillis;

      
     buttonPressedCounter=0;// all button presses must have been processed in one loop iteration
   }
}

void waterPlant(){ 
  
  lcdLineOne="Jarvis";
  lcdLineTwo="Water Pump";
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(lcdLineOne);
  lcd.setCursor(0,1);
  lcd.print(lcdLineTwo);


  Serial.println("Watering");
  digitalWrite(HBridge1,HIGH);
  digitalWrite(HBridge7,HIGH);
  digitalWrite(HBridge2,LOW);
  boolean timeout=false;
  int counter=0;
  currentMillis= millis();
  wateringPreviousMillis=currentMillis;
  while(!timeout){
      currentMillis= millis();
      if (currentMillis-wateringPreviousMillis > estimatedMillisToWater){
        timeout=true;
      }
      lcdLineOne="Jarvis ";
      lcdLineOne.concat(TimeToString2((estimatedMillisToWater - (currentMillis - wateringPreviousMillis))/1000));
      Serial.println(((estimatedMillisToWater - (currentMillis - wateringPreviousMillis))/1000));
      lcdLineTwo="Watering       W";
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(lcdLineOne);
      lcd.setCursor(0,1);
      lcd.print(lcdLineTwo);
      delay(lcdRefresh);
      counter++;
  }
  //lcd.clear();
  //lcd.begin(16,2);
  //lcd.print("Jarvis");
  //lcd.setCursor(0,1);
  //lcd.print("Water Pump");  
  //lcd.setCursor(15,1);
  //lcd.print("D");
  //lcd.setCursor(7,0);
  //lcd.print("Draining     ");
  Serial.println("Draining");
  //drain plant
  digitalWrite(HBridge1,HIGH);
  digitalWrite(HBridge7,LOW);
  digitalWrite(HBridge2,HIGH);
  timeout=false;
  counter=0;
  currentMillis= millis();
  wateringPreviousMillis=currentMillis;
  while(!timeout){
      currentMillis= millis();
      if (currentMillis-wateringPreviousMillis > estimatedMillisToWater){
        timeout=true;
      }
      lcdLineOne="Jarvis ";
      //lcdLineOne.concat(TimeToString2((estimatedMillisToWater - currentMillis % estimatedMillisToWater)/1000));
      lcdLineOne.concat(TimeToString2((estimatedMillisToWater-(currentMillis-wateringPreviousMillis))/1000));
      lcdLineTwo="Draining       D";
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(lcdLineOne);
      lcd.setCursor(0,1);
      lcd.print(lcdLineTwo);
      delay(lcdRefresh);
      counter++;
      Serial.print("Draining for estimatedMillisToWater ");
      Serial.println(((estimatedMillisToWater-(currentMillis-wateringPreviousMillis))/1000));
  }  
  
  //stop pump
  delay(3000);//drain for 3 extra seconds just to make sure
  digitalWrite(HBridge1,LOW);
  
} 

// t is time in seconds = millis()/1000;
/*char * TimeToString(unsigned long t)
{
 static char str[20];
 int d = t / 86400;
 t = t % 86400;
 int h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 int s = t % 60;
 sprintf(str, "%02dd,%02d:%02d:%02drrr",d, h, m, s);
 return str;
}
*/
String TimeToString2(unsigned long t)
{
 String str="";
 if(t >= 86400 ){
    int d = t / 86400;
    t = t % 86400;
    str.concat(d);
    str.concat("d ");
 }
 int h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 int s = t % 60;
 //sprintf(str, "%02d:%02d:%02drrr", h, m, s);

 if(h<10){str.concat("0");}
 str.concat(h);
 str.concat(":");
 if(m<10){str.concat("0");}
 str.concat(m);
 str.concat(":");
 if(s<10){str.concat("0");}
 str.concat(s);
 return str;
}

void buttonPress(){
     //delayMicroseconds(1000000);
     Serial.println("Button press ");

     currentButtonMicros=micros();  
     if (currentButtonMicros - lastButtonMicros >debounceMicros){
       //buttonPressedArray[0]=currentButtonMillis;
       secondButtonMicros=firstButtonMicros;
       firstButtonMicros=currentButtonMicros;
       buttonPressedCounter=buttonPressedCounter+1;

       Serial.println("////////////////////////////////////////////////////////////////// ");
       Serial.print("INTERRUPT - Button Counter ");
       Serial.println(buttonPressedCounter);
       Serial.print("INTERRUPT - lastButtonMicros ");
       Serial.println(lastButtonMicros);
       Serial.print("INTERRUPT - currentButtonMicros ");
       Serial.println(currentButtonMicros);
       lastButtonMicros=currentButtonMicros;
     }
     
}



