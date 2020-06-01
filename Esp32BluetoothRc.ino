
#define M_LEFT_A  25
#define M_LEFT_B  26
#define M_RIGHT_A 18 
#define M_RIGHT_B 19

#include "BluetoothSerial.h"

#define CHANNEL_LEFT_A  0
#define CHANNEL_LEFT_B  1
#define CHANNEL_RIGHT_A 2
#define CHANNEL_RIGHT_B 3

#define PWM_FREQ        100
#define PWM_RESOLUTION  8


#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;


void setup() {
  Serial.begin(115200);
  
  pinMode(M_LEFT_A, OUTPUT);
  pinMode(M_LEFT_B, OUTPUT);
  pinMode(M_RIGHT_A, OUTPUT);
  pinMode(M_RIGHT_B, OUTPUT);

  digitalWrite(M_LEFT_A, 1);
  digitalWrite(M_LEFT_B, 1);
  digitalWrite(M_RIGHT_A, 1);
  digitalWrite(M_RIGHT_B, 1);

  setupBluetooth();

  setupPwm();

//  analogWrite(M_RIGHT, 50);
}

void setupBluetooth() {
  SerialBT.register_callback(bluetoothCallback);
  SerialBT.begin("Car RC 2"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void setupPwm() {
  ledcSetup(CHANNEL_LEFT_A, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(M_LEFT_A, CHANNEL_LEFT_A);

  ledcSetup(CHANNEL_RIGHT_A, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(M_RIGHT_A, CHANNEL_RIGHT_A);

  ledcSetup(CHANNEL_LEFT_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(M_LEFT_B, CHANNEL_LEFT_B);

  ledcSetup(CHANNEL_RIGHT_B, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(M_RIGHT_B, CHANNEL_RIGHT_B);
}

void dataParsing(char* message) {
  int8_t startIndex, endIndex;
  char strLeft[32];
  char strRight[32];
  Serial.print("Message = ");
  Serial.println(message);

  endIndex = indexOf(message, '$');
  if (endIndex == -1) return;
  memcpy(strLeft, message, endIndex);
  strLeft[endIndex] = '\0';

  startIndex = endIndex + 1;
  endIndex = indexOf(message + startIndex, '$');
  if (endIndex == -1) return;
  memcpy(strRight, message + startIndex, endIndex);
  strRight[endIndex] = '\0';

  Serial.print("strLeft -> ");
  Serial.println(strLeft);
  Serial.print("strRight -> ");
  Serial.println(strRight);
  Serial.println();

  int16_t speedLeft = atoi(strLeft);
  int16_t speedRight = atoi(strRight);

  controlMotor(true, speedRight);
  controlMotor(false, speedLeft);
}

int8_t indexOf(char* str, char find) {
  const char *ptr = strchr(str, find);
  if (ptr == NULL) return -1;
  return ptr - str;
}

uint8_t count = 0;
char text[100];

void loop() {
  uint8_t length = SerialBT.available();
  if (length > 0) {
    for (uint8_t i = 0; i < length; i++) {
      text[count++] = (char) SerialBT.read();
    };
    if(text[count - 1] == '\n') {
      text[count - 1] = '\0';
      count = 0;
      dataParsing(text);
    }
  }
}

void controlMotor(boolean isRight, int16_t speed) {
  if (isRight) {
    if (speed >= 0) {
      ledcWrite(CHANNEL_RIGHT_A, 255 - speed);
      ledcWrite(CHANNEL_RIGHT_B, 255);
    } else {
      ledcWrite(CHANNEL_RIGHT_B, 255 + speed);
      ledcWrite(CHANNEL_RIGHT_A, 255);
    }

  } else {
    if (speed >= 0) {
      ledcWrite(CHANNEL_LEFT_A, 255 - speed);
      ledcWrite(CHANNEL_LEFT_B, 255);
    } else {
      ledcWrite(CHANNEL_LEFT_B, 255 + speed);
      ledcWrite(CHANNEL_LEFT_A, 255);
    }
  }
}

void brakeMotor() {
  controlMotor(true, 0);
  controlMotor(false, 0);
}

void bluetoothCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    brakeMotor();
  }
 
  if(event == ESP_SPP_CLOSE_EVT ){
    Serial.println("Client disconnected");
    brakeMotor();
  }
}