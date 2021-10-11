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

const int BUTTON_LEFT_PIN = 35;
const int BUTTON_RIGHT_PIN = 32;
const int BUTTON_UP_PIN = 33;
const int BUTTON_DOWN_PIN = 25;
const int BUTTON_BACK_PIN = 23;
const int BUTTON_SELECT_PIN = 27;

const int BUTTON_LEFT = 0;
const int BUTTON_RIGHT = 1;
const int BUTTON_UP = 2;
const int BUTTON_DOWN = 3;
const int BUTTON_BACK = 4;
const int BUTTON_SELECT = 5;
const int NUMBER_OF_BUTTONS = 6;

const int MENU_TIMEOUT_MILLISECONDS = 5000;
const int DEBOUNCE_DELAY = 50;
const int BUTTON_GPIO_PIN[NUMBER_OF_BUTTONS] = {BUTTON_LEFT_PIN, BUTTON_RIGHT_PIN, BUTTON_UP_PIN, BUTTON_DOWN_PIN, BUTTON_BACK_PIN, BUTTON_SELECT_PIN};

int button_state[NUMBER_OF_BUTTONS] = {LOW,LOW,LOW,LOW,LOW,LOW};

void init_gpio_buttons() {
  pinMode(BUTTON_LEFT_PIN ,INPUT);
  pinMode(BUTTON_RIGHT_PIN ,INPUT);
  pinMode(BUTTON_UP_PIN ,INPUT);
  pinMode(BUTTON_DOWN_PIN ,INPUT);
  pinMode(BUTTON_BACK_PIN ,INPUT);
  pinMode(BUTTON_SELECT_PIN ,INPUT);
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

const int MENU_CHOICES_COUNT_INT = 4;
unsigned long last_debounce_time_milliseconds = 0;
unsigned long menu_opened_milliseconds = 0;
bool lcd_display_updated = false;
bool menu_timer_started = false;
bool display_menu_open = false;
bool menu_cursor_updated = false;
int current_menu_selection = 0;

void DEBUGGING_serial_print_button_state() {
  for(int i = 0; i < NUMBER_OF_BUTTONS; i++) {
    Serial.print("Button : ");
    Serial.print(i + 1);
    Serial.print(button_state[i]);
    Serial.println("");
  }
}

void display_menu_items_on_lcd(bool BUTTON_PRESSED) {
  String MENU_OPTIONS[MENU_CHOICES_COUNT_INT] = {
    "1) Set Alarm Clk", 
    "2) Cancel Alarm Clk", 
    "3) Start Pomodoro",
    "4) Stop Pomodoro"
  };

  int scrolling_offset = 0;

  if(current_menu_selection >= 2) {
    scrolling_offset = current_menu_selection - 1;
  }

  lcd.setCursor(0,0);
  lcd.print(MENU_OPTIONS[0 + scrolling_offset]);
  lcd.setCursor(0,1);
  lcd.print(MENU_OPTIONS[1 + scrolling_offset]);
  
  if(!BUTTON_PRESSED) {
    if(current_menu_selection == 0){
      lcd.setCursor(2, 0);
    } else {
      lcd.setCursor(2, 1);
    }
    lcd.print(">");
    lcd_display_updated = true;
  }
}

// Displays menu to set alarms and change modes
void display_menu(bool BUTTON_PRESSED) {

    if(!menu_timer_started || BUTTON_PRESSED) {
      menu_opened_milliseconds = millis();
      lcd_display_updated = false;
      menu_cursor_updated = false;
      
      if(!menu_timer_started){
        lcd.clear();
      }

      if(BUTTON_PRESSED && menu_timer_started && !menu_cursor_updated){
        Serial.print("Current Menu Selection is");        
        Serial.print(current_menu_selection);
        Serial.println("");

        if(button_state[BUTTON_DOWN] == HIGH && current_menu_selection < MENU_CHOICES_COUNT_INT - 1) {
          current_menu_selection++;
        }
        if(button_state[BUTTON_UP] == HIGH && current_menu_selection > 0) {
          current_menu_selection--;
        }        

        // DEBUGGING_serial_print_button_state();

        menu_cursor_updated = true;
      }

      menu_timer_started = true;
    }

    if(!lcd_display_updated){
      display_menu_items_on_lcd(BUTTON_PRESSED);
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