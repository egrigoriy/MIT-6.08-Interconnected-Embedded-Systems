#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
const int BUTTON = 4; //connect to pin 4

uint8_t draw_state;  //used for remembering state
uint8_t previous_value;  //used for remembering previous button
uint8_t LOOP_DURATION = 20;
uint8_t loop_time;

void setup() {
    Serial.begin(115200); //initialize serial!
    tft.init(); //initialize the screen
    tft.setRotation(2); //set rotation for our layout
    pinMode(BUTTON,INPUT_PULLUP); //sets IO pin 4 as an input which defaults to a 3.3V signal when unconnected and 0V when the switch is pushed
    draw_state = 0; //initialize to 0
    previous_value = 1; //initialize to 1
    loop_time = millis();
}


void loop() {
  uint8_t value = digitalRead(BUTTON); //get reading
  // Manage STATE transitions
  fsm(value, &draw_state);

  // Manage ACTION based on current STATE
  if (value != previous_value) {
    draw(draw_state);
  }

  // For Debuging
  Serial.println(draw_state);

  previous_value = value; //remember for next time

  // equilibre the loop duration
  while (millis() - loop_time < LOOP_DURATION);
  loop_time = millis();
}

bool isPressed(uint8_t value) {
  return value == 0;
} 

void fsm(uint8_t value, uint8_t* draw_state) {
  if ((*draw_state == 0) && isPressed(value)) {
    *draw_state = 1;
  } else if ((*draw_state == 1) && !isPressed(value)) {
    *draw_state = 2;
  } else if ((*draw_state == 2) && isPressed(value)) {
    *draw_state = 3;
  } else if ((*draw_state == 3) && !isPressed(value)) {
    *draw_state = 0;
  }
}

void draw(uint8_t draw_state) {
  switch (draw_state) {
      case 0:
      case 2:
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("BOSCO",0,100,4);
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.drawString("BOSCO",0,120,4);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("BOSCO",0,140,4);
        break;
      case 1:
      case 3:
        tft.fillScreen(TFT_BLUE);
        for (int i=0; i<160; i=i+1) {
          tft.drawLine(0, 0, 128, i, TFT_GREEN);
        }
        for (int i=0; i<160; i=i+4) {
          tft.drawLine(0, 159, 128, i, TFT_RED);
        }
        tft.setTextColor(TFT_BLACK, TFT_GREEN);
        break;
      default:
        tft.fillScreen(TFT_WHITE);
  }

}
