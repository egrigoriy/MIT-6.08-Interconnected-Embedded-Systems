#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.

/*#########################################
  Simple example script demonstrating:
  Connection with an Access Point (AP)
  Creation of client object
  Connection of client object to host
  Sending of GET request with several arguments
  verbose printing of HTTP response from host
  Simple parsing of HTTP response

  We'll use the open API found at:
  http://numbersapi.com
  Intended for 6.08 Spring 2022 Lab 01B
*/

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int GETTING_PERIOD = 5000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

uint32_t loop_controller; //used for timing
uint32_t last_time; //used for timing

//wifi network credentials for 6.08 Lab (this is a decent 2.4 GHz router that we have set up...try to only use for our ESP32s)
char network[] = "klg-firpo-guest";
char password[] = "QutYGJnU6M";

uint8_t scanning = 0;//set to 1 if you'd like to scan for wifi networks (see below):
/* For later use when doing wifi development at home or in dorm:
    Having network issues since there are 50 MIT and MIT_GUEST networks?. Do the following:
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


void setup() {
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  Serial.begin(115200); //begin serial
  if (scanning){
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
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 6) { //can change this to more attempts
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);  //acceptable since it is in the setup function.
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  randomSeed(analogRead(A0)); //"seed" random number generator
}

/*-----------------------------------
  Generate a request to the numbersapi server for a random number
  Display the response both on the TFT and in the Serial Monitor
*/
void loop() {
  if ((millis() - last_time) > GETTING_PERIOD) { // GETTING_PERIOD since last lookup? Look up again
    build_http_get();
    //submit to function that performs GET.  It will return output using response_buffer char array
    do_http_GET("egrigoriy.pythonanywhere.com", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    Serial.println(response_buffer); //print to serial monitor
    http_response_tft();
    last_time = millis();//remember when this happened so we perform next lookup in GETTING_PERIOD milliseconds
  }
}

void http_response_tft() {
    tft.fillScreen(TFT_BLACK); //black out TFT Screen
    tft.setCursor(0, 0, 1);
    tft.println(response_buffer);
}
void build_http_get() {
      //formulate GET request...first line:
    sprintf(request_buffer, "GET https://egrigoriy.pythonanywhere.com/iot608/numbersapi/%d/trivia/ HTTP/1.1\r\n", random(200));
    strcat(request_buffer, "Host: egrigoriy.pythonanywhere.com\r\n"); //add more to the end
    strcat(request_buffer, "\r\n"); //add blank line!
}
/*----------------------------------
   char_append Function:
   Arguments:
      char* buff: pointer to character array which we will append a
      char c:
      uint16_t buff_size: size of buffer buff

   Return value:
      boolean: True if character appended, False if not appended (indicating buffer full)
*/
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

/*----------------------------------
   do_http_GET Function:
   Arguments:
      char* host: null-terminated char-array containing host to connect to
      char* request: null-terminated char-arry containing properly formatted HTTP GET request
      char* response: char-array used as output for function to contain response
      uint16_t response_size: size of response buffer (in bytes)
      uint16_t response_timeout: duration we'll wait (in ms) for a response from server
      uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
   Return value:
      void (none)
*/
void do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line! (end of response header)
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);  //empty in prep to store body
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");
  } else {
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}
