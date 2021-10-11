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

bool display_menu_open = false;
unsigned long last_debounce_time_milliseconds = 0;
unsigned long menu_opened_milliseconds = 0;
bool menu_timer_started = false;
int current_menu_selection = 0;
const int MENU_TIMEOUT_MILLISECONDS = 5000;
bool lcd_display_updated = false;

// Displays menu to set alarms and change modes
void display_menu(bool BUTTON_PRESSED) {
    if(!menu_timer_started || BUTTON_PRESSED) {
      menu_opened_milliseconds = millis();
      lcd_display_updated = false;
      
      if(!menu_timer_started){
        lcd.clear();
      }
      menu_timer_started = true;
    }

    if(!lcd_display_updated){
      lcd.setCursor(0,0);
      lcd.print("1) Set Alarm Clk");
      lcd.setCursor(0,1);
      lcd.print("2) Cancel Alarm Clk");

      lcd.setCursor(2, current_menu_selection);
      lcd.print(">");
      lcd_display_updated = true;
    }
    
    if(menu_opened_milliseconds + MENU_TIMEOUT_MILLISECONDS < millis()) {
      display_menu_open = false;
      menu_timer_started = false;
      menu_opened_milliseconds = 0;
      current_menu_selection = 0;
      lcd.clear();
    }
}

void loop() {
  struct tm current_time;

  const bool BUTTON_PRESSED = check_button_state();

  if(BUTTON_PRESSED) {
    display_menu_open = true;
    last_debounce_time_milliseconds = millis();
  }

  Serial.println(last_debounce_time_milliseconds < millis());

  current_time = get_time_now();
  
  if(!display_menu_open){
    lcd.setCursor(0,0);
    lcd.print("The time is...");
    lcd_display_formatted_time(current_time, 1, 0);
  } 

  if(display_menu_open) {
    display_menu(BUTTON_PRESSED);
  }

  delay(10);
}