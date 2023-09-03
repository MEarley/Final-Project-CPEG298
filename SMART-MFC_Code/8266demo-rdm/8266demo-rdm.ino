/***************************************************************************
  SMART MFC
    IoT Device code for a Microbial Fuel Cell: MFC for short. The purpose of this
    device is to measure the voltage and the surrounding environment
    of said cell for research purposes.
    Measures:
      Millivolts, Temperature, Humidity, Light, and Soil moisture 

  written by Michael Earley
  05/8/2023
 ***************************************************************************/
#include "Arduino.h"
#include <SoftwareSerial.h>		          //Allows us to use two GPIO pins for a second UART
#include <Wire.h>
#include <DHT20.h>                      // Library used to access the DHT20 (By Rob Tillaart)
DHT20 DHT(&Wire);

#define TEMPPIN D3
#define TEMPTHRESHOLD 25
#define TEXTDIVIDER "__________________________________________"

SoftwareSerial espSerial(10,11);	      //Create software UART to talk to the ESP8266

String IO_USERNAME = "";		    // Adafruit IO used
String IO_KEY  =     "";
String WIFI_SSID = ""; 	    
String WIFI_PASS = ""; 		            //Blank for open network

float temperature;
int lightLevel;
float humidity;
float voltage = 0.000000000;

float moisture;
const float VOLTREF = 1.1;

// Define Pins
const int LIGHTPIN = A0;
const int VOLTPIN = A1;
const int SOILPIN = A2;

/***************************************************************************
  This is an example program for sending a counter to Adafruit IO using
  an ESP8266 WiFi module.  You will need to correct the WiFi SSID and password
  and add your Adafruit IO username and Key.

  written by Theo Fleck and Rick Martin
  03/25/2020
 ***************************************************************************/

void setup() {
	Serial.begin(9600);		// set up Serial monitor with 9600 baud rate
	espSerial.begin(9600);		// set up software UART to ESP8266 @ 9600 baud rate
	Serial.println("setting up");
	String resp = espData("get_macaddr",2000,true);	//get MAC address of 8266
	resp = espData("wifi_ssid="+WIFI_SSID,2000,true);	//send Wi-Fi SSID to connect to
	resp = espData("wifi_pass="+WIFI_PASS,2000,true);	//send password for Wi-Fi network
	resp = espData("io_user="+IO_USERNAME,2000,true);	//send Adafruit IO info
	resp = espData("io_key="+IO_KEY,2000,true);
	resp = espData("setup_io",15000,true);			//setup the IoT connection
	if(resp.indexOf("connected") < 0) {
		Serial.println("\nAdafruit IO Connection Failed");
		while(1);
	}


  // DHT20 Library Setup
  #if defined(ESP8266) || defined(ESP32)
  DHT.begin(12, 13); //  select your pin numbers here
  #else
  DHT.begin();
  #endif

  Serial.print("DHT20 LIBRARY VERSION: ");
  Serial.println(DHT20_LIB_VERSION);
  Serial.println();

  // Sets Pins for Senors
  pinMode(LIGHTPIN, INPUT);
  Serial.print("Light Sensor Pin set to: ");
  Serial.println(LIGHTPIN);
  lightLevel = analogRead(LIGHTPIN);
  pinMode(VOLTPIN, INPUT);
  Serial.print("Voltage Pin set to: ");
  Serial.println(VOLTPIN);
  pinMode(SOILPIN,INPUT);
  Serial.print("Soil Moisture Sensor Pin set to: ");
  Serial.println(SOILPIN);
  moisture = analogRead(SOILPIN);  

  //start the data feed  
  resp = espData("setup_feed=1,Voltage (mV)",2000,false);	// Voltage
  resp = espData("setup_feed=2,Light Level",2000,false); // Light
  resp = espData("setup_feed=3,Temperature (C)",2000,false); // Temperature
  resp = espData("setup_feed=4,Humidity (P)",2000,false); // Humidity
  resp = espData("setup_feed=5,Soil Moisture Level",2000,false); // Soil
  

  // Temp Threshold LED
  pinMode(6, OUTPUT);
  // Moisture Threshold LED
  pinMode(7, OUTPUT);

  Serial.println("------ Setup Complete ----------");
}

// DISCLAIMER: Possibly due to the amount of noise generated in the GND by other components, the millivolt reader does not read the correct values. 
// More tweaking is needed
//returns Voltage in millivolts
void calculateVoltage(void){
  analogReference(INTERNAL);  // Sets voltage reference to 1.1v to allow precise millivolt reading 
  voltage = VOLTREF * analogRead(VOLTPIN);
  Serial.println(voltage);
  analogReference(DEFAULT);  
  delay(1000);
  return;
}

void loop() {

	// free version of Adafruit IO only allows 30 uploads/minute, it discards everything else
  delay(5000);	 // Wait 5 seconds  between uploads	
  // Voltage Meter
  Serial.println(TEXTDIVIDER); // Serial Monitor Divider
  Serial.print("Voltage (mV), ");
  calculateVoltage();
  String resp = espData("send_data=1,"+String(voltage),2000,false); //send feed to cloud
  
  // Reading Grove Light Sensor
  Serial.println(TEXTDIVIDER); // Serial Monitor Divider
  Serial.print("Light Level, ");
  lightLevel = analogRead(LIGHTPIN);  
  Serial.print(lightLevel);
  resp = espData("send_data=2,"+String(lightLevel),2000,false); //send feed to cloud
  
  // Reading DHT20 Grove Temperature sensor
  //-------------------------------------------------------------
  // DHT20 Temperature and Humidity Sensor Demo code
  // By library author: Rob Tillaart
  // Edited by: Michael Earley  

  //  READ DATA
  Serial.println(TEXTDIVIDER); // Serial Monitor Divider
  Serial.print("DHT20, \t");
  int status = DHT.read();
  switch (status)
  {
  case DHT20_OK:
    Serial.print("OK,\t");
    break;
  case DHT20_ERROR_CHECKSUM:
    Serial.print("Checksum error,\t");
    break;
  case DHT20_ERROR_CONNECT:
    Serial.print("Connect error,\t");
    break;
  case DHT20_MISSING_BYTES:
    Serial.print("Missing bytes,\t");
    break;
  case DHT20_ERROR_BYTES_ALL_ZERO:
    Serial.print("All bytes read zero");
    break;
  case DHT20_ERROR_READ_TIMEOUT:
    Serial.print("Read time out");
    break;
  case DHT20_ERROR_LASTREAD:
    Serial.print("Error read too fast");
    break;
  default:
    Serial.print("Unknown error,\t");
    break;
  }

  //  DISPLAY DATA, sensor has only one decimal.
  humidity = DHT.getHumidity();    
  Serial.print(humidity, 1);
  Serial.print("%,\t");
  temperature = DHT.getTemperature();
  Serial.print(temperature, 1);
  Serial.println(" °C");

  //-------------------------------------------------------------

  resp = espData("send_data=3,"+String(temperature),2000,false); //send feed to cloud
  resp = espData("send_data=4,"+String(humidity),2000,false); //send feed to cloud

  // Reading Grove Soil Moisture Sensor
  Serial.println(TEXTDIVIDER); // Serial Monitor Divider
  Serial.print("Soil Moisture, ");
  moisture = analogRead(SOILPIN);
  Serial.println(moisture);
  resp = espData("send_data=5,"+String(moisture),2000,false); //send feed to cloud

  // Soil Moisture Metrics

  if(moisture<=300){
    Serial.println("The soil is dry.");
  }
  else if(moisture > 300 && moisture <= 700){
    Serial.println("The soil is humid.");
  }
  else if(moisture > 700 && moisture <= 950){
    Serial.println("The soil is wet.");
  }
  else{
    Serial.println("ERROR: Undefined Soil Moisture Level");
  }


  // Threshold for Temperature
  Serial.println(TEXTDIVIDER); // Serial Monitor Divider
  if(temperature > TEMPTHRESHOLD){
    digitalWrite(6, HIGH);   
    Serial.print("Temperature exceeds set threshold of ");
    Serial.print(TEMPTHRESHOLD);
    Serial.println(" °C");      
  } else{ digitalWrite(6, LOW); }

  // Threshold for moisture
  if(moisture > 250 /* Place Holder*/){
    digitalWrite(7,HIGH);
  } else{ digitalWrite(7,LOW);}

}

String espData(String command, const int timeout, boolean debug) {
	String response = "";
	espSerial.println(command);	//send data to ESP8266 using Serial UART
	long int time = millis();
	while ( (time + timeout) > millis()) {	//wait the timeout period sent with the command
		while (espSerial.available()) {	//look for response from ESP8266
			char c = espSerial.read();
			response += c;
			Serial.print(c);	//print response on Serial monitor
		}
	}
	if (debug) {
		Serial.println("Resp: "+response);
	}
	response.trim();
	return response;
}
