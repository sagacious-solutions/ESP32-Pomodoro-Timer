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

const int BUTTON_LEFT = 35;
const int BUTTON_RIGHT = 32;
const int BUTTON_UP = 33;
const int BUTTON_DOWN = 25;
const int BUTTON_BACK = 23;
const int BUTTON_SELECT = 27;
const int NUMBER_OF_BUTTONS = 6;

const int DEBOUNCE_DELAY = 50;

const int BUTTON_GPIO_PIN[NUMBER_OF_BUTTONS] = {BUTTON_LEFT, BUTTON_RIGHT, BUTTON_UP, BUTTON_DOWN, BUTTON_BACK, BUTTON_SELECT};
int button_state[NUMBER_OF_BUTTONS] = {LOW,LOW,LOW,LOW,LOW,LOW};

void init_gpio_buttons() {
  pinMode(BUTTON_LEFT ,INPUT);
  pinMode(BUTTON_RIGHT ,INPUT);
  pinMode(BUTTON_UP ,INPUT);
  pinMode(BUTTON_DOWN ,INPUT);
  pinMode(BUTTON_BACK ,INPUT);
  pinMode(BUTTON_SELECT ,INPUT);
}

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

  init_gpio_buttons();

  connect_to_wifi();

  init_lcd();

  init_time_client();
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

bool check_button_state() {
  bool button_pressed = false;

  for(int i = 0; i <= 5; i++){
    button_state[i] = digitalRead(BUTTON_GPIO_PIN[i]); 

    if (button_state[i]) {
      button_pressed = true;
    }
  }

  return button_pressed;
}

bool display_menu = false;
unsigned long lastDebounceTime = 0;

void loop() {
  struct tm current_time;

  if(check_button_state()) {
    display_menu = true;
    lastDebounceTime = millis();
  }

  Serial.println(lastDebounceTime < millis());

  current_time = get_time_now();
  
  if(!display_menu){
    lcd.setCursor(0,0);
    lcd.print("The time is...");
    lcd_display_formatted_time(current_time, 1, 0);
  } 

  delay(10);
}