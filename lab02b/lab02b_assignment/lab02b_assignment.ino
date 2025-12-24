#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include<math.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

char network[] = "klg-firpo-guest";
char password[] = "QutYGJnU6M";
/* Having network issues since there are 50 MIT and MIT_GUEST networks?. Do the following:
    When the access points are printed out at the start, find a particularly strong one that you're targeting.
    Let's say it is an MIT one and it has the following entry:
   . 4: MIT, Ch:1 (-51dBm)  4:95:E6:AE:DB:41
   Do the following...set the variable channel below to be the channel shown (1 in this example)
   and then copy the MAC address into the byte array below like shown.  Note the values are rendered in hexadecimal
   That is specified by putting a leading 0x in front of the number. We need to specify six pairs of hex values so:
   a 4 turns into a 0x04 (put a leading 0 if only one printed)
   a 95 becomes a 0x95, etc...
   see starting values below that match the example above. Change for your use:
   Finally where you connect to the network, comment out
     WiFi.begin(network, password);
   and uncomment out:
     WiFi.begin(network, password, channel, bssid);
   This will allow you target a specific router rather than a random one!
*/
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

const uint8_t LOOP_PERIOD = 20; //milliseconds
uint32_t primary_timer = 0;
uint32_t posting_timer = 0;
uint32_t step_timer = 0;
float x, y, z; //variables for grabbing x,y,and z values
const char USER[] = "grigoriy";

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const uint8_t BUTTON_CLEAR = 4; //pin connected to button
const uint8_t BUTTON_POST = 5; //pin connected to button
const uint8_t BUTTON_TOTAL = 15; //pin connected to button

MPU6050 imu; //imu object called, appropriately, imu
float old_acc_mag;  //previous acc mag
float older_acc_mag;  //previous prevoius acc mag
float threshold = 10.5;
const float ZOOM = 9.81; //for display (converts readings into m/s^2)...used for visualizing only

//some suggested variables you can use or delete:

enum post_states {
  POST_STATE_IDLE,
  POST_STATE_DOWN,
  POST_STATE_POST
};

post_states post_state;

enum step_states {
  STEP_STATE_IDLE,
  STEP_STATE_RISE,
  STEP_STATE_FALL
};

step_states step_state;
int steps = 0;
int total_steps = 0;

enum draw_states {
  DRAW_STATE_IDLE,
  DRAW_STATE_CURRENT,
  DRAW_STATE_CLEAR,
  DRAW_STATE_TOTAL
};

draw_states draw_state, prior_draw_state;

void setup() {
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background 
  tft.setTextFont(2); // Set the text font to font number 2
  Serial.begin(115200); //begin serial comms
  delay(100); //wait a bit (100 ms)
  Wire.begin();
  delay(500); //pause to make sure comms get set up
  tft.setCursor(0, 0);
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
    tft.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    tft.println("IMU Not Connected :/");
    tft.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  tft.println("Attempting to connect to WiFi AP");
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    tft.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    tft.println("CONNECTED!");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  pinMode(BUTTON_CLEAR, INPUT_PULLUP); //set input pin as an input!
  pinMode(BUTTON_POST, INPUT_PULLUP); //set input pin as an input!
  pinMode(BUTTON_TOTAL, INPUT_PULLUP); //set input pin as an input!

  step_state = STEP_STATE_IDLE; //
  post_state = POST_STATE_POST;
  draw_state = DRAW_STATE_IDLE;
  prior_draw_state = DRAW_STATE_IDLE;
  tft.fillScreen(TFT_BLACK); //fill background
}

bool isPushed(uint8_t button) {
  return button == 0;
}

void loop() {
  //get IMU information:
  imu.readAccelData(imu.accelCount);
  float x, y, z;
  x = ZOOM * imu.accelCount[0] * imu.aRes;
  y = ZOOM * imu.accelCount[1] * imu.aRes;
  z = ZOOM * imu.accelCount[2] * imu.aRes;
  float acc_mag = sqrt(x * x + y * y + z * z);
  float avg_acc_mag = 1.0 / 3.0 * (acc_mag + old_acc_mag + older_acc_mag);
  older_acc_mag = old_acc_mag;
  old_acc_mag = acc_mag;

  //get button readings:
  uint8_t button_clear = digitalRead(BUTTON_CLEAR);
  uint8_t button_post = digitalRead(BUTTON_POST);
  uint8_t button_total = digitalRead(BUTTON_TOTAL);

  step_reporter_fsm(button_clear, avg_acc_mag); //run step_reporter_fsm (from lab02a)
  post_reporter_fsm(button_post); //run post_reporter_fsm (written here)
  lcd_display(button_clear, button_total); //update display (minimize pixels you change)

  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}

//Post reporting state machine, uses button1 as input
//use post_state for your state variable!
void post_reporter_fsm(uint8_t button_post) {
  switch (post_state) {
    case POST_STATE_IDLE:
      if (isPushed(button_post)) {
        post_state = POST_STATE_DOWN;
      }
      break;
    case POST_STATE_DOWN:
      if (!isPushed(button_post)) {
        post_state = POST_STATE_POST;
      }
      break;
    case POST_STATE_POST:
      build_http_post(request_buffer, USER, steps);
      Serial.println(request_buffer);
      do_http_request("egrigoriy.pythonanywhere.com", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
      Serial.printf("\n POST REQUEST RETUNS: %s \n", response_buffer);
      total_steps = atoi(response_buffer);
      post_state = POST_STATE_IDLE;
      step_state = STEP_STATE_IDLE;
      break;
  }
}


//Step Counting FSM from Lab02A.  (bring over and adapt global variables as needed!)
void step_reporter_fsm(uint8_t button_clear, float avg_acc_mag) {
  if (isPushed(button_clear)) {
    step_state = STEP_STATE_IDLE;
  }

  switch(step_state) {
    case STEP_STATE_IDLE:
      steps = 0;
      if (avg_acc_mag > threshold) {
        step_state = STEP_STATE_RISE;
      }
      break;
    case STEP_STATE_RISE:
      if (avg_acc_mag < threshold) {
        steps++;
        step_state = STEP_STATE_FALL;
      }
      break;
    case STEP_STATE_FALL:
      if (avg_acc_mag > threshold) {
        step_state = STEP_STATE_RISE;
      }
      break;
  }
}

void draw_idle() {
      draw_state = DRAW_STATE_CURRENT;
      prior_draw_state = DRAW_STATE_IDLE;
}

void draw_current(uint8_t button_total) {
      if (draw_state != prior_draw_state) {
          tft.fillScreen(TFT_BLACK); //fill background
          prior_draw_state = DRAW_STATE_CURRENT;
      }
      
      tft.setCursor(0, 0);      
      tft.print("Steps: "); tft.println(steps);

      if (isPushed(button_total)) {
        draw_state = DRAW_STATE_TOTAL;
      }
}

void draw_total(uint8_t button_total) {
      if (draw_state != prior_draw_state) {
          tft.fillScreen(TFT_BLACK); //fill background
          prior_draw_state = DRAW_STATE_TOTAL;
      }

      tft.setCursor(0, 0);      
      tft.println("Total Steps");
      tft.print(USER); tft.print(" : "); tft.println(total_steps);

      if (!isPushed(button_total)) {
        draw_state = DRAW_STATE_CURRENT;
      }
}

void draw_clear(uint8_t button_clear) {
  if(isPushed(button_clear)) {
    tft.fillScreen(TFT_BLACK); //fill background 
    draw_state = DRAW_STATE_CURRENT;   
  }
}

//Display information on LCD based on button value (stateless)
void lcd_display(uint8_t button_clear, uint8_t button_total) {
  switch (draw_state) {
    case DRAW_STATE_IDLE:
      draw_idle();
      break;
    case DRAW_STATE_CURRENT:
      draw_current(button_total);
      break;
    case DRAW_STATE_CLEAR:
      draw_clear(button_clear);
      break;
    case DRAW_STATE_TOTAL:
      draw_total(button_total);
      break;
  }
}
