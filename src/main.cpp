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

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void connect_to_wifi(){
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
}

void init_lcd(){
  lcd.init();

  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Pomodoro Timer");
  delay(2000);
  lcd.clear();
}

// setup time client for RTC
void init_time_client(){
  const int ONE_TIMEZONE_OFFSET = 3600;
  const int MY_TIME_ZONE = -7;

  // Initialize a NTPClient to get time
  timeClient.begin();

  // GMT -1 = -3600 / GMT +1 = 3600
  timeClient.setTimeOffset(ONE_TIMEZONE_OFFSET * MY_TIME_ZONE);

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
}


void setup()
{
  Serial.begin(9600);
  Serial.println("Initializing....");

  connect_to_wifi();

  init_lcd();

  init_time_client();

  delay(5000);

}

// Prints out the passed time arguement to the LCD
void lcd_display_formatted_time(tm time_to_print, int row, int column){
  int hours = time_to_print.tm_hour;

  if(hours >= 13) {
    hours -= 12;

  }

  lcd.setCursor(column, row);

  lcd.print(hours);
  lcd.print(":");
  if(time_to_print.tm_min < 10){
    lcd.print("0");
  }
  lcd.print(time_to_print.tm_min);
  lcd.print(":");
  if(time_to_print.tm_sec < 10){
    lcd.print("0");
  }
  lcd.print(time_to_print.tm_sec);
}

// Fetch raw epoch time from NTP Client and return it as localtime as tm
tm get_time_now(){
  struct tm current_time;

  time_t rawTime = timeClient.getEpochTime();
  current_time = *localtime(&rawTime);

  return current_time;
}

void loop() {
  struct tm current_time;
  struct tm * tm_ptr;

  tm_ptr = &current_time;
  


  current_time = get_time_now();

  Serial.println(asctime(tm_ptr));


  lcd.setCursor(0,0);
  lcd.print("The time is...");

  lcd_display_formatted_time(current_time, 1, 0);

  delay(1000);
}