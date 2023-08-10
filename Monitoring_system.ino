#include <PZEM004Tv30.h>
#include <HardwareSerial.h>

#define RXD2 16
#define TXD2 17
int  acMains = 4;
int solarVoltage = 2;
int current= 15;
int batteryVoltage = 32;
int percentage;

unsigned long previousTime = 0;
unsigned long interval=180000;
float mainsEnergy;
float acEnergy;
double solarEnergy=0;




PZEM004Tv30 pzem(&Serial2);


void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // put your main code here, to run repeatedly:
  double Current= 0;
  double batteryVol= 0 ;
  double solarVolt =0;
  int Batteryvolt= 0;
unsigned long currentTime = millis();
if (acMains ==HIGH);
  if (currentTime-previousTime >=interval);
   mainsEnergy= 65/20000;
   acEnergy= acEnergy+ mainsEnergy;
    Serial.print("Ac supply Energy: "); 
    Serial.print(acEnergy,3); 
    Serial.print (" kWh");
  else
  {
  }
  currentTime=previousTime;
else{
  if (currentTime-previousTime >=interval){
   acEnergy= acEnergy;
   Serial.print("Ac supply Energy: "); 
    Serial.print(acEnergy,3); 
    Serial.print (" kWh");
  }
   currentTime=previousTime;
}
int i;
for (i=0, i<=150; i=i+1;){
int batteryVolt = analogRead(batteryVoltage);
double batteryVolt= map(batteryVolt, 0, 4095, 0, 25 );
batteryVol= batteryVol+ batteryVolt;


int solarVol= analogRead(solarVoltage);
double solarVol= map(solarVol, 0, 4095, 0, 25 );
solarVolt = solarVolt + solarVol;

int solarCurren=analogRead(current);
double solarCurren= map(solarVolt, 0, 4095, 0, 5);
double Curren = (solarCurren-2.5)/0.066;
double Current = Current + Curren;
delay(2000);
}
Batteryvolt = batteryVolt/150;
percentage =  map(Batteryvolt, 10.8, 12.6, 0, 100);

  Serial.print("Battery Percentage: "); 
   Serial.print(percentage); 
    Serial.print ("%");

double solarPower =  solarVolt* Current/2250;
double solEnergy= solarPower * interval;
  solarEnergy = solarEnergy + solEnergy
Serial.print("Solar Energy: "); 
   Serial.print(solarEnergy,3); 
    Serial.print (" kWh");

float energyOut = pzem.energy();
    if(!isnan(energyOut)){
        Serial.print("Energy: "); 
        Serial.print(energyOut,3); 
        Serial.println("kWh");
    } 
    else {
        Serial.println("Error reading energy");

}
