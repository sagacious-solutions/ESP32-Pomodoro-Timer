#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>


// https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "wifi_secrets.h"

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  Serial.begin(9600);
  Serial.println("SERIAL INIT!!!!");
  delay(1000);

  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.

  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Pomodoro Timer");
  lcd.setCursor(0,1);
  lcd.print("Here We Go!");
  delay(5000);
}


void loop()
{
  time_t timer;
  time(&timer);

  struct tm y2k = {0};
  struct tm pomodoro = {0};

  double seconds;

  y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
  y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

  seconds = difftime(timer,mktime(&y2k));

  lcd.setCursor(0,0);
  lcd.print("January 1, 2000");
  lcd.setCursor(0, 1);
  lcd.print(seconds);

  Serial.println(timer);
  Serial.println(seconds);
  Serial.println("Haha Same!");

  delay(1000);
}