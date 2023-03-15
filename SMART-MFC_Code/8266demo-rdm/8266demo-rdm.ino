/***************************************************************************
  This is an example program for the sending a counter to Adafruit IO using
  an ESP8266 WiFi module.  You will need to correct the WiFi SSID and password
  and add your Adafruit IO username and Key.

  written by Theo Fleck and Rick Martin
  03/25/2020
 ***************************************************************************/
#include "Arduino.h"
#include <SoftwareSerial.h>		          //Allows us to use two GPIO pins for a second UART
#include "Seeed_BMP280.h"
#include <Wire.h>
//#include <DHT.h>
//#include <DHT_U.h>
#define TEMPPIN D3
#include <U8g2lib.h>


// u8g2 Library Definitions
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
//#include <Wire.h>
#endif

//U8G2_SSD1306_128X64_ALT0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // SSD1306 and SSD1308Z are compatible
 
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);    //Low spped I2C
// End of u8g2


SoftwareSerial espSerial(11,10);	      //Create software UART to talk to the ESP8266
BMP280 BMP;
String IO_USERNAME = "MEarley";
String IO_KEY  =     "aio_kdyu82j8scwf9zQ65mE348JAyuWq";
String WIFI_SSID = "UD Devices"; 	    //Only need to change if using other network, eduroam won't work with ESP8266
String WIFI_PASS = ""; 		            //Blank for open network
float num = 1.0; 			                  //Counts up to show upload working
float temperature;
int lightLevel;
float humidity;

// DHT pin 3 type DHT11
  //DHT dht(6, 11);

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
	resp = espData("setup_feed=1,CPEG-ELEG298",2000,false);	//start the data feed

  // Initiate BMP280
  if(!BMP.init()){
    Serial.println("Failed to initate BMP280");
  } else{
    Serial.println("Successfully initated BMP280");
  }
	
  // Sets pin 3 to output
  pinMode(6, OUTPUT);
  lightLevel = analogRead(6);
  
  //dht.begin();
  //u8g2.begin();

  Serial.println("------ Setup Complete ----------");
}

void loop() {

	// free version of Adafruit IO only allows 30 uploads/minute, it discards everything else
	delay(5000);			// Wait 5 seconds between uploads
	//Serial.print("Num is: ");  
	//Serial.println(num);
  
	
	//num = num +0.5;			// Count by 0.5 increments

  Serial.print("Attempting to read light levels from pin 6\n");
  lightLevel = analogRead(6);  
  Serial.println(lightLevel);

  Serial.print("Attempting to read temperature from BMP280\n");
  temperature = BMP.getTemperature();  
  Serial.print(temperature);
  Serial.println(" C");

  //Serial.print("Attempting to read humidity from BMP280\n");
  //humidity = BMP.getHumidity();  
  //Serial.print(humidity);
  //Serial.println(" UNIT");

  /*

  Serial.print("Attempting to read temperature from function\n");
  temperature = dht.readTemperature(false);
  Serial.println(temperature);*/
/*
  // u8g2 Display
  u8g2.clearBuffer();                   // clear the internal memory
  u8g2.setFlipMode(1);
  u8g2.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  //char *TemperatureString;
  //TemperatureString = malloc(sizeof(char) * 30);
  //sprintf(TemperatureString,"P: %.2f C",25.3423);
  //u8g2.drawStr(0,10,TemperatureString);    // write something to the internal memory
  u8g2.sendBuffer();                    // transfer internal memory to the display
  // End of Display    */

  String resp = espData("send_data=1,"+String(temperature),2000,false); //send feed to cloud
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
