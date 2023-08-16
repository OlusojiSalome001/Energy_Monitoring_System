#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb
#include <PZEM004Tv30.h> //pzem library
#include <HardwareSerial.h> // communicating with hardware serial
#include <HTTPClient.h> // library for sending data server
#include <TinyGsmClient.h> // Library for comunicating with SIM 800L
// Configure TinyGSM library




String GOOGLE_SCRIPT_ID = "AKfycbwc5yRfYYGhUo074myV_bKWDnWWe2nNQsA6dUk8H_cezDG4vRrFrgUiJtV6Ee3z0UwY";


// TTGO T-Call pins
#define Modem_RST            5
#define Modem_PWKEY          4
#define Modem_POWER_ON       23
#define Modem_TX             27
#define Modem_RX             26

//  GPRS credentials (leave empty, if not needed)
const char* apn     = " "; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char* gprsUser = " "; // GPRS User
const char* gprsPass = " "; // GPRS Password



// Defining TTGO TCall Serial with Serial 2
HardwareSerial gsmSerial(2);



#ifdef DUMP_AT_COMMANDS // checks if the DUMP_AT_COMMANDS macro has been defined
  #include <StreamDebugger.h> // This library helps  monitor the communication between your program and the GSM module by logging AT commands and responses.
  StreamDebugger debugger(gsmSerial, Serial); //If the DUMP_AT_COMMANDS macro is defined, this line creates a StreamDebugger object named debugger. It takes two parameters: gsmSerial is typically the serial port connected to your GSM module, Serial is used for monitoring/debugging.
  TinyGsm modem(debugger); // This line initializes the TinyGsm object named modem
#else
  TinyGsm modem(gsmSerial); // initializes the TinyGsm object using just the Serial connection without debugging
#endif

// TinyGSM Client for Internet connection
TinyGsmClient client(modem);

//PZEM004Tv30 pins
#define RXD2 16 // RX pin for PZEM
#define TXD2 17 // TX pin for PZEM
HardwareSerial pzemSerial(1); //setting serial1 for pzem, and renaming it as pzemSerial
PZEM004Tv30 pzem;



int  acMains = 14; // pin for switch from mains power source
int solarVoltage = 2; // pin for voltage sensor used for measuring Solar Voltage 
int current= 15; // pin for current sensor used for measuring Solar current
int batteryVoltage = 32; // pin for voltage sensor used for measuring battery Voltage 
int percentage; // defining the percentage value of the battery
// float previousEnergy = 0; 

unsigned long previousTime = 0; // definining previous time for millis
unsigned long interval=180000; // defining interval as 3 Minutes
float acEnergy=0.0; // setting a default value for the mains energy
double solarEnergy=0.0; // setting a default value for solar energy







void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
 // Set GSM module baud rate and UART pins
 gsmSerial.begin(9600, SERIAL_8N1, Modem_RX , Modem_TX );

// Set pzem baud rate and UART pins
pzemSerial.begin (9600, SERIAL_8N1, RXD2 , TXD2 );

pzem.setAddress(0x01); // Set the PZEM-004T address if needed
delay(1000);

  // Set modem reset, enable, power pins
  pinMode(Modem_PWKEY, OUTPUT);
  pinMode(Modem_RST, OUTPUT);
  pinMode(Modem_POWER_ON, OUTPUT);
  digitalWrite(Modem_PWKEY, LOW);
  digitalWrite(Modem_RST, HIGH);
  digitalWrite(Modem_POWER_ON, HIGH);

modem.init(); // initializes modem

// TinyGSM Client for Internet connection
TinyGsmClient client(modem);


  if (!modem.init()) {
    Serial.println("Modem initialization failed.");
    while (true);
  }

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println("GPRS connection failed.");
    while (true);
  }
}



void loop() {
  // put your main code here, to run repeatedly:

  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (gsmSerial.available())
    {
       data();
    }
    }
  
}
void data() {
  double solarPowr= 0; // define current as 0 at the beginning of the loop
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

int solarCurren=analogRead(current); // reading the current sensor
double solarCurrent= map(solarVolt, 0, 4095, 0, 5);// converting analog reading to a voltage value
double Curren = (solarCurrent-2.5)/0.066; // converting voltage value  to current 
double solarPow = solarVol*Curren;
solarPowr = solarPowr +solarPow; // Getting aggregate power



delay(2000);
}
double Batteryvolt = batteryVolt/150; // getting the average battery voltage values

percentage =  map(Batteryvolt, 10.8, 12.6, 0, 100);// converting battery voltage to percentage

  Serial.print("Battery Percentage: "); 
   Serial.print(percentage); 
    Serial.print ("%");

double solarPower =  solarPowr/150; // averaging out power
double solEnergy= solarPower * 3/60000; // multiplying  solar power by 3 minutes delay and converting to kwh
  solarEnergy = solarEnergy + solEnergy;
Serial.print("Solar Energy: "); 
   Serial.print(solarEnergy,3); 
    Serial.print (" kWh");

float energyOut = pzem.energy(); // reading the output energy using pzem

    if(!isnan(energyOut) ){
        Serial.print("Energy: "); 
        Serial.print(energyOut,3); 
        Serial.println("kWh");
    } 
    else {
        Serial.println("Error reading energy");
  
}

if (!isnan(energyOut)|| solarPower<=50 || acMains == HIGH )
  {


    String param;
    param  = "solar_energy="+String(solarEnergy);
    param += "&mains_energy="+String(acEnergy);
    param += "&battery_level="+String(percentage);
    param += "&energy_consumed="+String(energyOut);
    //Serial.println(param);
    write_to_google_sheet(param);
  }
  else
  {
    Serial.println("No any valid Energy data.");
  }

  
}
void write_to_google_sheet(String params) {
   HTTPClient http;
   String url=" https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
  
   //Serial.print(url);
    Serial.println("Posting Energy Consumption data to Google Sheet");
    //---------------------------------------------------------------------
    //starts posting data to google sheet
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);     
    }
    //---------------------------------------------------------------------
    http.end();

}
