#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


uint8_t state; //state variable
const uint8_t UP = 0;
const uint8_t DOWN = 1;
int steps; //counting steps
float old_acc_mag, older_acc_mag; //maybe use for remembering older values of acceleration magnitude
float acc_mag = 0;  //used for holding the magnitude of acceleration
float avg_acc_mag = 0; //used for holding the running average of acceleration magnitude
float threshold = 10.5;
const float ZOOM = 9.81; //for display (converts readings into m/s^2)...used for visualizing only
const int BUTTON = 4;

const uint8_t LOOP_PERIOD = 20; //milliseconds
uint32_t primary_timer = 0;
float x, y, z; //variables for grabbing x,y,and z values

MPU6050 imu; //imu object called, appropriately, imu

void setup() {
  Serial.begin(115200); //for debugging if needed.
  delay(50); //pause to make sure comms get set up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }
  pinMode(BUTTON,INPUT_PULLUP); //sets IO pin 4 as an input which defaults to a 3.3V signal when unconnected and 0V when the switch is pushed
  tft.init(); //initialize the screen
  tft.setRotation(2); //set rotation for our layout
  primary_timer = millis();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  steps = 0; //initialize steps to zero!
  older_acc_mag = 0;
  old_acc_mag = 0;
  state = DOWN;
}

void step_count_fsm() {
  switch(state) {
    case UP:
      if (avg_acc_mag < threshold) {
        state = DOWN;
        steps++;
      }
      break;
    case DOWN:
      if (avg_acc_mag > threshold) {
        state = UP;
      }
      break;
  }
}

bool isPressed() {
  return digitalRead(BUTTON) == 0;
}
void loop() {
  if (isPressed()) {
    Serial.println("RESET STEPS");
    tft.fillScreen(TFT_BLACK);
    steps = 0;
  }
  imu.readAccelData(imu.accelCount);
  x = ZOOM * imu.accelCount[0] * imu.aRes;
  y = ZOOM * imu.accelCount[1] * imu.aRes;
  z = ZOOM * imu.accelCount[2] * imu.aRes;

  acc_mag = sqrt(x * x + y * y + z * z);
  avg_acc_mag = (older_acc_mag + old_acc_mag + acc_mag) / 3;
  older_acc_mag = old_acc_mag;
  old_acc_mag = acc_mag;
  step_count_fsm();
  //Serial printing:
  char output[100];
  sprintf(output, "X:%4.2f,Y:%4.2f,Z:%d", acc_mag, avg_acc_mag, steps); //render numbers with %4.2 float formatting
  Serial.println(output); //print to serial for plotting
  //redraw for use on LCD (with new lines):
  //sprintf(output, "%4.2f  \n%4.2f  \n%d", acc_mag, avg_acc_mag , steps); //render numbers with %4.2 float formatting
  sprintf(output, "Steps: %d", steps); //render numbers with %4.2 float formatting
  tft.setCursor(0, 0, 4);
  tft.println(output);
  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
  //no code here please.
}
