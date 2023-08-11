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




PZEM004Tv30 pzem(Serial2, RXD2, TXD2);


void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  // put your main code here, to run repeatedly:
  double Current= 0; // define current as 0 at the beginning of the loop
  double batteryVol= 0 ; // define current as 0 at the beginning of the loop
  double solarVolt =0; // define current as 0 at the beginning of the loop
  int batteryVolt= 0; // define current as 0 at the beginning of the loop
unsigned long currentTime = millis(); // code for calculating ac mains supply
if (currentTime-previousTime >=interval) 
  {
    if (acMains == HIGH) {
   double mainsEnergy= 65*(currentTime-previousTime)/3600000000 ; // multiply 65W power by duration between current time and the previous time
   double acEnergy= acEnergy+ mainsEnergy;
    }
    else
    { acEnergy= acEnergy;}
    previousTime= currentTime; // set the previous time as current time
    Serial.print("Ac supply Energy: "); 
    Serial.print(acEnergy,3); 
    Serial.print (" kWh");

  }

int i;// defining variable i
for (i=0; i<=150; i=i+1) // created the for  loop to get average value.
{ 
int batteryVolt = analogRead(batteryVoltage); // reading the voltage sensor 
double batteryVolta= map(batteryVolt, 0, 4095, 0, 25 ); // mapping the analog readings to battery voltage value
batteryVol= batteryVol+ batteryVolta;


int solarVo= analogRead(solarVoltage); // reading the voltage sensor
double solarVol= map(solarVo, 0, 4095, 0, 25 ); // mapping the analog readings to solar voltage value
solarVolt = solarVolt + solarVol;

int solarCurren=analogRead(current); // reading the current sensor
double solarCurrent= map(solarVolt, 0, 4095, 0, 5);// converting analog reading to a voltage value
double Curren = (solarCurrent-2.5)/0.066; // converting voltage value  to current 
double Current = Current + Curren;
delay(2000);
}
double Batteryvolt = batteryVolt/150; // getting the average battery voltage values

percentage =  map(Batteryvolt, 10.8, 12.6, 0, 100);// converting battery voltage to percentage

  Serial.print("Battery Percentage: "); 
   Serial.print(percentage); 
    Serial.print ("%");

double solarPower =  solarVolt* Current/2250; // averaging out power
double solEnergy= solarPower * 3/60000; // multiplying  solar power by 3 minutes delay and converting to kwh
  solarEnergy = solarEnergy + solEnergy;
Serial.print("Solar Energy: "); 
   Serial.print(solarEnergy,3); 
    Serial.print (" kWh");

float energyOut = pzem.energy(); // reading the output energy using pzem 
    if(!isnan(energyOut)){
        Serial.print("Energy: "); 
        Serial.print(energyOut,3); 
        Serial.println("kWh");
    } 
    else {
        Serial.println("Error reading energy");
}
}