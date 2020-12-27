/*
  Set baud rate to 9600 and // rate on npt server setup
  The following sketch sends data to the Nextion display without any library.
  works with -> WiFi remote temp receiver ESP12 Blynk
  ESP to ESP based on example from https://robotzero.one/sending-data-esp8266-to-esp8266/
*/
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


#include <SPI.h>
#include <Wire.h>
//________________________Block for time sketch_________________________________

#include <TimeLib.h>
//#include <ESP8266WiFi.h>
#include<strings.h>

//__________________________Log_in_______________________________________________
const char *ssid = "RoboLand";          //  Router SSID (name)
const char *pass = "P0pShop9724";       //  Router password

//Client/Server login
const char *ssid_local = "RoverTemp"; //took out * before ssid, ditto password
const char *password_local = "18816194";
ESP8266WebServer server(80);  //port
//______________________________temp variables__________________________________
float T1;
int tempOUT;
float T2;                               // Inside temp
int tempIN;                             // Inside temp from onewire
int current_temp = 0;
int maxi = 0, mini = 99 ;               // Max/Min inside temperature variables with initial values.
int maxiperp = 0, miniperp = 99;        // long term inside high and low
int maxo = 0, mino = 99 ;               // Max/Min outside temperature variables with initial values.
int maxoperp = 0, minoperp = 99;
int i;                                  // DStemp ndex

// values passed from remote
int readingInt; 
int voltage;// make readingInt a global variable
int voltage_reading;

// reset long term high,low to 0,99 respectively
int ResetPin = 5;                       
int ResetValue;
int address = 0;

// scale temp to dial on Nextion
int maptempIN;
int newtempIN;


//byte value;
int hourFormat12();
uint32_t timeStamp = now();//out for timezone test


//variables for Nextion temp fields
int maxinhourperp, maxinminperp, In_High_Month, In_High_Day, In_High_Year;
int mininhourperp, mininminperp, In_Low_Month, In_Low_Day, In_Low_Year, In_L_Yr;
int minhroutperp, minminoutperp, Out_max_low_month, Out_max_low_day, Out_max_low_year;
int Out_max_hi_hour, Out_max_hi_minute, Out_max_hi_month, Out_max_hi_day, Out_max_hi_year;

//________________________From here down to Setup is NPT_________________________
//Packets with the time
#include <WiFiUdp.h>
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

/*-------- NTP code ----------*/
// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = -8;  // Pacific Standard Time (USA)//-8 for standard time

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

//________________________Onewire________________________________________________
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 13                                         // Data wire is plugged into pin 13
OneWire oneWire(ONE_WIRE_BUS);                                  // Pass oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
int deviceCount = 0;

//&&&&&&&&&&&&&&&&&SETUP&&&&&&&&&&&&&&&&SETUP&&&&&&&&&&&&&&&&&&&&&&SETUP&&&&&&&&&&&&&&&&&&&&

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Serial.print(__FILE__);
  Serial.print (", ");
  Serial.print(__DATE__);
  Serial.print (", ");
  Serial.println(__TIME__);

  pinMode (ResetPin, INPUT);  //pin 5

  //________________________Onewire temp sensors_____________________________________________
  sensors.begin(); // Start up the library
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println("\n*** Source File: Desk_clock_BASE_Combo-rework ***\n");

  //______________________Setup_for_NPT_____________________________________________________
  //Serial.begin(9600);
  Serial.println("TimeNTP Example");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  time_t prevDisplay = 0;                  // when the digital clock was displayed

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  //_______________________________Connect to remote ESP to get temp_______________
  WiFi.softAP(ssid_local, password_local);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
delay(500);

  // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.on("/data/", HTTP_GET, handleSentVar);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("End of Setup");

} // End of setup

//__________________________routine to contact remote ESP to get the outside temp______________
void handleSentVar() {
  Serial.println("handleSentVar function called...");

    server.arg("sensor_reading"); { // this is the variable sent from the client   
    readingInt = server.arg("sensor_reading").toInt();
    server.arg("voltage_reading");  // this is the variable sent from the client   
    voltage = server.arg("voltage_reading").toInt();
    }
    Serial.println("Sensor reading received...");
    Serial.print("Reading To Print2 = ");
    Serial.println(readingInt);
    Serial.print("readingInt = "); Serial.println(readingInt);
    Serial.println("Voltage reading received...");   
    Serial.print("Voltage: ");
    Serial.println(voltage_reading);
    Serial.println();
    server.send(200, "text/html", "Data received");
   //let client know data was received and reset Hi/Lo temps
}    

void OutsideTemp() {
  Serial.println();
  Serial.println("Start of OutsideTemp function");
  Serial.print("readingInt = "); Serial.println(readingInt);
  //Serial.print("mino = "); Serial.println(mino);
  //Serial.println("first IF");

  if (readingInt != mino) {
    mino = readingInt;      //update low temp
    //Serial.print("mino = "); Serial.println(mino);
  }
  //Serial.println("second IF");
  if (readingInt >0 and readingInt <= EEPROM.read(8)) //update low temp
  {
    minoperp = readingInt;
    minhroutperp = (hour()); //mininhourperp
    minminoutperp = (minute()); //mininminperp
    Out_max_low_month = (month());
    Out_max_low_day = (day());
    Out_max_low_year = (year());
  }
 // Serial.println("third IF");
  if (readingInt >= maxo) {
    maxo = readingInt;
  } //update max temp
  
 // Serial.println("4th IF");
  if (readingInt >= maxoperp)
  {
    maxoperp = readingInt;        //update max temp
    Out_max_hi_hour = (hour());
    Out_max_hi_minute = (minute());
    Out_max_hi_month = (month());
    Out_max_hi_day = (day());
    Out_max_hi_year = (year());
  }
 // Serial.println("Last IF");
  if (readingInt != current_temp)
  {
    current_temp = readingInt;
    
        Serial.print("n0.val=");
        Serial.print(hour());
        Serial.print("\xFF\xFF\xFF");

        Serial.print("n0.val=");
        Serial.print(hour());
        Serial.print("\xFF\xFF\xFF");

        //Minute
        Serial.print("n1.val=");
        Serial.print(minute());
        Serial.print("\xFF\xFF\xFF");
      }
  }

  //************LOOP************************LOOP************************
  void loop() {
    Serial.println();
    server.handleClient();
    InSidetemp();
    OutsideTemp();
    EEprom();
    Nextion_Day();
    Nextion_Time();
    Low_High_IN_Out();
    Time_Stamp();
    Reset_LoHi_Temps();    
  }

  void EEprom() {
    Serial.println("Start of EEprom function");
    ResetValue = digitalRead (ResetPin);// pull pin 5 high to clear EEPROM
    EEPROM.begin(512);

    Serial.print ("Reset Pin start = "); Serial.println (ResetValue);
    if (ResetValue == 1)
    {
      EEPROM.write(2, 0);             //max in perpetual
      delay(100);
      EEPROM.write(4, 99);            //min in perpetual
      delay(100);
      EEPROM.write(6, 0);             //max out perpetual
      delay(100);
      EEPROM.write(8, 99);            //min out perpetual
      delay(100);
      //EEPROM.commit();
      Serial.print ("Reset Value = "); Serial.println(ResetValue);
    }
    Serial.println("EEPROM values at start");
    Serial.print("EEPROM 2 Max IP = ");
    Serial.println(EEPROM.read(2));
    Serial.print("EEPROM 4 Min IP= ");
    Serial.println(EEPROM.read(4));
    Serial.print("EEPROM 6 Max OP= ");
    Serial.println(EEPROM.read(6));
    Serial.print("EEPROM 8 Min OP= ");
    Serial.println(EEPROM.read(8));

    //Inside temps
    // save max in temp
    if (maxiperp > EEPROM.read(2))
    {
      EEPROM.write(2, maxiperp);
      EEPROM.write(30, ("%02d\n", (year()) % 100));
      EEPROM.write(32, In_High_Month);
      EEPROM.write(34, In_High_Day);
      EEPROM.write(36, maxinminperp);
      EEPROM.write(38, maxinhourperp);
      Serial.print ("Max In Perp = "); Serial.println (EEPROM.read(2));
    }
    // save min in temp
    if (miniperp < EEPROM.read(4))
    {
      EEPROM.write(4, miniperp);
      EEPROM.write(10, ("%02d\n", (year()) % 100));
      EEPROM.write(12, In_Low_Month);
      EEPROM.write(14, In_Low_Day);
      EEPROM.write(16, mininminperp);
      EEPROM.write(18, mininhourperp);
      Serial.print ("Min In Perp = "); Serial.println (EEPROM.read(4));
    }
    //Outside temps
    // save max out temp

    if (maxoperp > EEPROM.read(6))
    {
      EEPROM.write(6, maxoperp);
      EEPROM.write(20, Out_max_hi_month);
      EEPROM.write(22, Out_max_hi_day);
      EEPROM.write(24, ("%02d\n", (year()) % 100));
      EEPROM.write(26, Out_max_hi_hour);
      EEPROM.write(28, Out_max_hi_minute);
      Serial.print ("Max Out Perp_If = "); Serial.println (EEPROM.read(6));
    }
    Serial.print(EEPROM.read(8));
    if (minoperp < EEPROM.read(8))
    {
      EEPROM.write(8, minoperp);
      EEPROM.write(40, Out_max_low_month);
      EEPROM.write(42, Out_max_low_day);
      EEPROM.write(44, ("%02d\n", (year()) % 100));
      EEPROM.write(46, minhroutperp);
      EEPROM.write(48, minminoutperp);
    }

    EEPROM.commit();
    Serial.println("End of EEprom function");
  }

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%__NEXTION__%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  void Nextion_Day() {

    Serial.print ("TempIN "); Serial.println (tempIN);
    Serial.print ("New TempIN "); Serial.println (newtempIN);
    Serial.print ("ReadingInt Value = "); Serial.println (readingInt);
    Serial.print ("Current Temp Out = "); Serial.println (current_temp);
    Serial.print ("Voltage = "); Serial.println (voltage);
    Serial.print ("Voltage Reading = "); Serial.println (voltage_reading);
    
    
    Serial.print("\xFF\xFF\xFF"); //has to be here even though above not printed to Nextion

    //Main Nextion page
    //Inside current
    Serial.print("tempIN.val=");
    Serial.print(tempIN);
    Serial.print("\xFF\xFF\xFF");

    //Day's inside high temp
    Serial.print("maxi.val=");   // dot val because val is the attribute that changes on that object.
    Serial.print(maxi);  // This is the value to print to Nextion object n1.
    Serial.print("\xFF\xFF\xFF");

    //Day's inside low temp
    Serial.print("mini.val=");
    Serial.print(mini);
    Serial.print("\xFF\xFF\xFF");

    //Outside current
    Serial.print("current_temp.val=");  //Nextion object
    Serial.print(current_temp);
    Serial.print("\xFF\xFF\xFF");

    //Day's outside high temp
    Serial.print("maxo.val=");
    Serial.print(maxo);
    Serial.print("\xFF\xFF\xFF");

    //Day's outside low temp
    Serial.print("mino.val=");
    Serial.print(mino);
    Serial.print("\xFF\xFF\xFF");

  }
  void Nextion_Time() {
    //Hour
    Serial.print("hour.val=");
    Serial.print(hourFormat12()); //Serial.print(hour() - 12); //convert to 12 hr format
    Serial.print("\xFF\xFF\xFF");

    //Minute
    Serial.print("minute.val=");
    Serial.print(minute());
    Serial.print("\xFF\xFF\xFF");

  }
  void Low_High_IN_Out() {

    //Perpetual Low Out
    Serial.print("minoperp.val=");
    Serial.print(EEPROM.read(8));
    Serial.print("\xFF\xFF\xFF");

    //Perpetual High Out
    Serial.print("maxoperp.val=");
    Serial.print(EEPROM.read(6));
    Serial.print("\xFF\xFF\xFF");

    //Perp Low In
    Serial.print("miniperp.val=");
    Serial.print(EEPROM.read(4));
    Serial.print("\xFF\xFF\xFF");

    Serial.print("IlMP.val=");
    Serial.print(EEPROM.read(12));
    Serial.print("\xFF\xFF\xFF");

    Serial.print("IlDP.val");
    Serial.print(EEPROM.read(14));
    Serial.print("\xFF\xFF\xFF");

    Serial.print("IlYP.val");
    Serial.print(EEPROM.read(10));
    Serial.print("\xFF\xFF\xFF");

  }

  void Time_Stamp() {

    //Perp High In
    Serial.print("maxiperp.val=");
    Serial.print(EEPROM.read(2));
    Serial.print("\xFF\xFF\xFF");

    //HIGH IN hour timestamp
    Serial.print("maxhrinperp.val=");
    Serial.print(EEPROM.read(38)); //(maxinhourperp);
    Serial.print("\xFF\xFF\xFF");

    //HIGH IN minute timestamp
    Serial.print("maxmininperp.val=");
    Serial.print(EEPROM.read(36)); //(maxinminperp);
    Serial.print("\xFF\xFF\xFF");

    //HIGH IN month timestamp
    Serial.print("IhMP.val=");
    Serial.print(EEPROM.read(32)); //(In_High_Month);
    Serial.print("\xFF\xFF\xFF");

    //HIGH IN daytimestamp
    Serial.print("IhDP.val=");
    Serial.print (EEPROM.read(34)); //(In_High_Day);
    Serial.print("\xFF\xFF\xFF");

    //HIGH IN year timestamp
    Serial.print("IhYP.val=");
    Serial.print( EEPROM.read(30)); //(In_High_Year);
    Serial.print("\xFF\xFF\xFF");


    //LOW IN hour timestamp
    Serial.print("minhrinperp.val=");
    Serial.print (EEPROM.read(18));  //(minhrinperp);
    Serial.write("\xFF\xFF\xFF");

    //LOW IN minute timestamp
    Serial.print("minininperp.val=");
    Serial.print(EEPROM.read(16)); //(mininminperp);
    Serial.print("\xFF\xFF\xFF");

    //LOW IN month timestamp
    Serial.print("IlMP.val=");
    Serial.print (EEPROM.read(12)); //(In_Low_Month);
    Serial.print("\xFF\xFF\xFF");

    //LOW IN day timestamp
    Serial.print("IlDP.val=");
    Serial.print(EEPROM.read(14)); //(In_Low_Day);
    Serial.print("\xFF\xFF\xFF");

    //LOW IN year timestamp
    Serial.print("IlYP.val=");
    Serial.print(EEPROM.read(10)); //(In_Low_Year);
    Serial.print("\xFF\xFF\xFF");

    // LOW OUT TIMESTAMP
    //LOW OUT Month timestamp
    Serial.print("OlMP.val=");
    Serial.print(EEPROM.read(40));
    Serial.print("\xFF\xFF\xFF");

    //LOW OUT Day timestamp
    Serial.print("OlDyP.val=");
    Serial.print(EEPROM.read(42));
    Serial.print("\xFF\xFF\xFF");

    //LOW OUT Year timestamp
    Serial.print("OlYyP.val=");
    Serial.print(EEPROM.read(44));
    Serial.print("\xFF\xFF\xFF");

    // LOW OUT hour timestamp
    Serial.print("minhroutperp.val=");
    Serial.print(EEPROM.read(46));
    Serial.print("\xFF\xFF\xFF");

    //LOW OUT minute timestamp
    Serial.print("minminoutperp.val=");
    Serial.print(EEPROM.read(48));
    Serial.print("\xFF\xFF\xFF");


    // HIGH OUT TIMESTAMP
    //HI OUT hour timestamp
    Serial.print("OxHrP.val=");
    Serial.print(EEPROM.read(26)); //(Out_max_hi_hour);//OxHrP
    Serial.print("\xFF\xFF\xFF");

    //HI OUT minute timestamp
    Serial.print("OxMiP.val=");
    Serial.print(EEPROM.read(28)); //(Out_max_hi_minute);//OxMiP
    Serial.print("\xFF\xFF\xFF");

    //HI OUT Month timestamp
    Serial.print("OxMP.val=");
    Serial.print(EEPROM.read(20)); //(Out_max_hi_month);
    Serial.print("\xFF\xFF\xFF");

    //HI OUT Day timestamp
    Serial.print("OxDyP.val=");
    Serial.print(EEPROM.read(22)); //(Out_max_hi_day);
    Serial.print("\xFF\xFF\xFF");

    //HI OUT Year timestamp
    Serial.print("OxYyP.val=");
    Serial.print(EEPROM.read(24)); //(Out_max_hi_year);
    Serial.print("\xFF\xFF\xFF");


    //Gauge
    Serial.print("gauge.val="); //gauge pointer
    maptempIN = map(tempIN, 40, 80, 0, 180);

    if (tempIN != newtempIN)//if temp hasn't changed don't update display to eliminate flicker
    {
      maptempIN = map(tempIN, 40, 80, 0, 180);
      Serial.print(maptempIN);  // mapped values to increase sensitivity.
    }
    newtempIN = tempIN;        //if no temp change this prints just ???, no value
    Serial.print("\xFF\xFF\xFF");

    // Battery level
    Serial.print("n2.val=");
    Serial.print(voltage); //(Out_max_hi_year);
    Serial.print("\xFF\xFF\xFF");
  }
