#include <DHT.h>
#include <DHT_U.h>

/***************************************************************************
  This is an example program for the sending a counter to Adafruit IO using
  an ESP8266 WiFi module.  You will need to correct the WiFi SSID and password
  and add your Adafruit IO username and Key.

  written by Theo Fleck and Rick Martin
  03/25/2020
 ***************************************************************************/
#include "Arduino.h"
#include <SoftwareSerial.h>		          //Allows us to use two GPIO pins for a second UART
#define TEMPPIN D3
SoftwareSerial espSerial(11,10);	      //Create software UART to talk to the ESP8266
String IO_USERNAME = "MEarley";
String IO_KEY  =     "aio_kdyu82j8scwf9zQ65mE348JAyuWq";
String WIFI_SSID = "UD Devices"; 	    //Only need to change if using other network, eduroam won't work with ESP8266
String WIFI_PASS = ""; 		            //Blank for open network
float num = 1.0; 			                  //Counts up to show upload working
float temperature;

// DHT pin 3 type DHT11
  DHT dht(3, 11);

void setup() {
	Serial.begin(9600);		// set up serial monitor with 9600 baud rate
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
	resp = espData("setup_feed=1,CPEG-ELEG298",2000,false);	//start the data feed
	
  // Sets pin 3 to output
  pinMode(3, OUTPUT);
  temperature = digitalRead(3);
  
  dht.begin();

  Serial.println("------ Setup Complete ----------");
}

void loop() {

	// free version of Adafruit IO only allows 30 uploads/minute, it discards everything else
	delay(5000);			// Wait 5 seconds between uploads
	Serial.print("Num is: ");  
	Serial.println(num);
  
	String resp = espData("send_data=1,"+String(num),2000,false); //send feed to cloud
	num = num +0.5;			// Count by 0.5 increments

  Serial.print("Attempting to Read from pin 3\n");
  temperature = digitalRead(3);  
  Serial.println(temperature);

  Serial.print("Attempting to read temperature from function\n");
  temperature = dht.readTemperature(false);
  Serial.println(temperature);
}

String espData(String command, const int timeout, boolean debug) {
	String response = "";
	espSerial.println(command);	//send data to ESP8266 using serial UART
	long int time = millis();
	while ( (time + timeout) > millis()) {	//wait the timeout period sent with the command
		while (espSerial.available()) {	//look for response from ESP8266
			char c = espSerial.read();
			response += c;
			Serial.print(c);	//print response on serial monitor
		}
	}
	if (debug) {
		Serial.println("Resp: "+response);
	}
	response.trim();
	return response;
}
