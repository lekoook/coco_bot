#include <Arduino.h>
#include <DataPackage.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Joystick related.
#define JS_1_X A0
#define JS_1_Y A1
#define JS_1_SW 4
#define JS_2_X A2
#define JS_2_Y A3
#define JS_2_SW 7

// Transmitter radio driver related.
#define CE_PIN 5
#define CS_PIN 6

// Transmitter radio driver related.
RF24 transmitter(CE_PIN, CS_PIN);
const uint16_t BUF_LEN = DataPackage::DATA_LEN;
const uint64_t COMM_ADDR = 0xFABFADCAFE;
uint8_t buf[BUF_LEN];
DataPackage::Packet data;

void setup() {
  Serial.begin(9600);

  pinMode(JS_1_X, INPUT);
  pinMode(JS_1_Y, INPUT);
  pinMode(JS_1_SW, INPUT_PULLUP);
  pinMode(JS_2_X, INPUT);
  pinMode(JS_2_Y, INPUT);
  pinMode(JS_2_SW, INPUT_PULLUP);

  transmitter.begin();
  transmitter.openWritingPipe(COMM_ADDR);
  transmitter.setPALevel(RF24_PA_LOW);
  transmitter.stopListening();
}

void loop() {
  data.js1X = (uint16_t)analogRead(JS_1_X);
  data.js1Y = (uint16_t)analogRead(JS_1_Y);
  data.js1SW = (uint16_t)digitalRead(JS_1_SW);
  data.js2X = (uint16_t)analogRead(JS_2_X);
  data.js2Y = (uint16_t)analogRead(JS_2_Y);
  data.js2SW = (uint16_t)digitalRead(JS_2_SW);

  DataPackage::serialiseData(data, buf);
  transmitter.write(buf, BUF_LEN);
}
