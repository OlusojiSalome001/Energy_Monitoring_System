  #define INFLUXDB_URL "https://us-east-1-1.aws.cloud2.influxdata.com"
  #define INFLUXDB_TOKEN "5Ara8VmYGuFAdTkwqcrfBcR5rCOJplBOCtm7qrU97EoZDCTU6UmGlgWrsyrcXASKPqml_ELXon01cPNnqFEqbg=="
  #define INFLUXDB_ORG "879aa95d396984da"
  #define INFLUXDB_BUCKET "energy_monitoring"
  /*API 
  lfUZOB6pPMbewsJ5Q8i0KcOniodlS9DRV5MWCn8PsjiOaBwRyfoExSUd5Et0GEr4mCij5nfv3IsMtGN7QllCXg==*/
  //defining Time Zone
  #define TZ_INFO "WAT-1"

  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  Point param("measurements");


//Include Libraries
 #include <TinyGsmClient.h> // Library for comunicating with SIM 800L
 #define TINY_GSM_MODEM_SIM800      // Modem is SIM800
 #define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb
 #include <PZEM004Tv30.h> //pzem library
 #include <HardwareSerial.h> // communicating with hardware serial

  
 
 // Configure TinyGSM library
 #include <time.h> // Time 

 #include <FS.h> // include flash storage 
 #include <SPIFFS.h> // include spiff storage
 File file;


 // TTGO T-Call pins
 #define Modem_RST            5
 #define Modem_PWKEY          4
 #define Modem_POWER_ON       23
 #define Modem_TX             27
 #define Modem_RX             26

 //  GPRS credentials (leave empty, if not needed)
 const char* apn     = "internet.ng.airtel.com "; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
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

  //
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 3600000;
  const int   daylightOffset_sec = 0;

  int trigger= 36;
  int echo = 39;
  int travelTime;
  int  acMains = 14; // pin for switch from mains power source
  int solarVoltage = 2; // pin for voltage sensor used for measuring Solar Voltage 
  int current= 15; // pin for current sensor used for measuring Solar current
  int batteryVoltage = 32; // pin for voltage sensor used for measuring battery Voltage 
  int percentage; // defining the percentage value of the battery
  float previousEnergy = 0; 

  unsigned long previousTime = 0; // defining previous time for millis
  unsigned long solarPreviousTime = 0; // defining solar previous time for millis
  unsigned long interval=180000; // defining interval as 3 Minutes
  float acEnergy=0.0; // setting a default value for the mains energy
  double solarEnergy=0.0; // setting a default value for solar energy
  String tamperStatus;
  


void setup() {
  // put your setup code here, to run once:

  // Init and getSerial.begin(115200);
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

  param.addTag("sensors", "pzem, ultrasonic, voltage sensor, current sensor");

  if (SPIFFS.begin())  // initializes flash storage
  {
     Serial.println("SPIFFS initialized.");
    File file = SPIFFS.open("/Readings.txt", "a");
  }
  

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
  // Init and get the time
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  //tags
  param.addTag("sensors", "pzem, ultrasonic, voltage sensor, current sensor");

  //influxdb connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } 
  else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  }

void loop() {
  // put your main code here, to run repeatedly:
  for (unsigned long start = millis(); millis() - start < 5997;)
    { 
       //data();
    if (data()){
      write_to_influxdB(param); 
      delay(1000);
     }
  
    }
   }
bool data() {
  bool energyReadingG2g = false;

  String status; 
    struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      //return;
    }
   double solarPowr= 0; // define current as 0 at the beginning of the loop
   double batteryVol= 0 ; // define current as 0 at the beginning of the loop
   double solarVolt =0; // define current as 0 at the beginning of the loop
   int batteryVolt= 0; // define current as 0 at the beginning of the loop....................................................................................................................................................................  
  // code for calculating ac mains energy supply
  unsigned long currentTime = millis(); // checking the time 
    if (currentTime-previousTime >=1000) 
    {
    if (acMains == HIGH) 
    {
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
  
  unsigned long solarCurrentTime = millis(); // Solar Current Time 
    int bucket =0;
   for (int i=0; i<=150; i=i+1) {// created the for  loop to get average value. 
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

    digitalWrite ( trigger, LOW);
    delayMicroseconds(10);
    digitalWrite ( trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite ( trigger, LOW);
    travelTime= pulseIn(echo, HIGH);
    travelDistance = travelTime/2 * (34.4/1000);
    bucket = bucket+travelDistance;
    delay(100);

    }

   double Batteryvolt = batteryVolt/150; // getting the average battery voltage values

   percentage =  map(Batteryvolt, 10.8, 12.6, 0, 100);// converting battery voltage to percentage

    Serial.print("Battery Percentage: "); 
    Serial.print(percentage); 
    Serial.print ("%");

   double solarPower =  solarPowr/150; // averaging out power
   double solEnergy= solarPower * (solarCurrentTime-solarPreviousTime)/3600000; // multiplying  solar power by delay and converting to kwh
   solarEnergy = solarEnergy + solEnergy;
    Serial.print("Solar Energy: "); 
    Serial.print(solarEnergy,3); 
    Serial.print (" kWh");
   solarPreviousTime = solarCurrentTime;

    Average = bucket/150;
   
  if (travelDistance>= 8)
  {
    tamperStatus = "Tamper";
  }
  else {
   tamperStatus = "No Tamper";
  }
  Serial.print("PowerCube Status:");
      Serial.print(travelDistance);
  float energyOut = pzem.energy(); // reading the output energy using pzem
    if(!isnan(energyOut) ) // 
    {
        Serial.print("Energy: "); 
        Serial.print(energyOut,3); 
        Serial.println("kWh");
    } 
    else {
        Serial.println("Error reading energy");
    }
   if (solarPower>=10 || acMains == HIGH){
    status = "charging"; 
   }
   else {
    status = "Not charging";
   }
   unsigned energyReadings = !isnan(energyOut)|| solarPower>=5 || acMains == HIGH;
   if (energyReadings )
    {
        // param;
     
     param.addField("solar_energy", solarEnergy);
     param.addField("mains_energy", acEnergy);
     param.addField("battery_level", percentage);
     param.addField("energy_consumed", energyOut);
     param.addField("charging_status", status);
     param.addField("powercube_tampering_status", tamperStatus);
     param.addField("battery_level", percentage);

    energyReadingG2g = true;
   }
   else
   {param.addField("solar_energy", "Not Charging");
     param.addField("mains_energy","Not Charging");
     param.addField("battery_level", percentage);
     param.addField("energy_consumed", "No energy Output");
     param.addField("charging_status", status);
     param.addField("powercube_tampering_status", tamperStatus);
     param.addField("battery_level", percentage);

    Serial.println("No any valid Energy data.");
   }
   return energyReadingG2g;
   }

void write_to_influxdB(Point param) {
  // Print what are we exactly writing
     Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(param));

  

  
  // Write point into buffer
  client.writePoint(param);

  // Clear fields for next usage. Tags remain the same.
  param.clearFields();
  // Write point
  if (client.writePoint(param)) {
    Serial.print("InfluxDB write Successful ");
    while (file.available()) {
        char c = file.read();
        Serial.print(c);
        client.writePoint(c);
      file.flush(); }
         }
   else {
     Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
      if (file) {
      Serial.println("File opened for writing:");
      // Write data to the file
      file.println(params);  
        }
 
    }
 }


