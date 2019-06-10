#include <Arduino.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <SPI.h>
#include <A4990MotorShield.h>
#include <DataPackage.h>
#include <DigitalIO.h>

// Receiver radio driver related.
#define POW_PIN 3
#define CE_PIN 2
#define CS_PIN 4 

// Motors related.
#define FWD_TREHOLD 501
#define BWD_TREHOLD 521
#define L_TREHOLD 521
#define R_TREHOLD 501

// Function prototypes.
void resetBuf(void);

// Motors related.
A4990MotorShield motors;
bool m1HasStop = true;
bool m2HasStop = true;
int speed = 0;
int currSpeed = 0;
int leftSpeed = 0;
int rightSpeed = 0;

// Recevier radio driver related.
RF24 receiver(CE_PIN, CS_PIN);
const uint16_t BUF_LEN = DataPackage::DATA_LEN;
const uint64_t COMM_ADDR = 0xFABFADCAFE;
uint8_t buf[BUF_LEN];
DataPackage::Packet data;

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino Remote Motor Controller");

  // Wireless receiver setup.
  pinMode(POW_PIN, OUTPUT);
  digitalWrite(POW_PIN, HIGH);
  receiver.begin();
  receiver.openReadingPipe(0, COMM_ADDR);
  receiver.setPALevel(RF24_PA_LOW);
  receiver.startListening();
}

void loop() {
  if (receiver.available())
  {
    // Unpack received commands.
    receiver.read(buf, BUF_LEN);
    data = DataPackage::deserialiseData(buf);
    // Forward - Backward control is active
    if (data.js1X < FWD_TREHOLD || data.js1X > BWD_TREHOLD)
    {
      // Forward
      if (data.js1X < FWD_TREHOLD)
      {
        // Left is also active
        if (data.js2Y > L_TREHOLD)
        {
          int leftVal = map(data.js2Y, L_TREHOLD + 1, 1023, 0, 400);
          leftSpeed = currSpeed - leftVal;
          leftSpeed = leftSpeed < 0 ? 0 : leftSpeed;
          rightSpeed = currSpeed;
          motors.setM1Speed(rightSpeed);
          motors.setM2Speed(leftSpeed);
          Serial.println("Forward left");
        }
        // Right is also active
        else if (data.js2Y < R_TREHOLD)
        {
          leftSpeed = currSpeed;
          int rightVal = map(data.js2Y, 0, R_TREHOLD - 1, 400, 0);
          rightSpeed = currSpeed - rightVal;
          rightSpeed = rightSpeed < 0 ? 0 : rightSpeed;
          motors.setM1Speed(rightSpeed);
          motors.setM2Speed(leftSpeed);
          Serial.println("Forward right");
        }
        // Only forward is active
        else
        {
          currSpeed = map(data.js1X, 0, FWD_TREHOLD - 1, 400, 0);
          motors.setSpeeds(currSpeed, currSpeed);
          Serial.println("Forward");
        }
        
        m1HasStop = false;
        m2HasStop = false;
      }
      // Backward
      else if (data.js1X > BWD_TREHOLD)
      {
        // Left is also active
        if (data.js2Y > L_TREHOLD)
        {
          int leftVal = map(data.js2Y, L_TREHOLD + 1, 1023, 0, 400);
          leftSpeed = currSpeed + leftVal;
          leftSpeed = leftSpeed > 0 ? 0 : leftSpeed;
          rightSpeed = currSpeed;
          motors.setM1Speed(rightSpeed);
          motors.setM2Speed(leftSpeed);
          Serial.println("Backward left");
        }
        // Right is also active
        else if (data.js2Y < R_TREHOLD)
        {
          leftSpeed = currSpeed;
          int rightVal = map(data.js2Y, 0, R_TREHOLD - 1, 400, 0);
          rightSpeed = currSpeed + rightVal;
          rightSpeed = rightSpeed > 0 ? 0 : rightSpeed;
          motors.setM1Speed(rightSpeed);
          motors.setM2Speed(leftSpeed);
          Serial.println("Backward right");
        }
        // Only backward is active
        else
        {
          currSpeed = -1 * map(data.js1X, BWD_TREHOLD + 1, 1023, 0, 400);
          motors.setSpeeds(currSpeed, currSpeed);
          Serial.println("Backward");
        }
        
        m1HasStop = false;
        m2HasStop = false;
      }      
    }

    // Forward - Backward control is inactive
    if (data.js1X >= FWD_TREHOLD && data.js1X <= BWD_TREHOLD)
    {
      // If only left is active
      if (data.js2Y > L_TREHOLD)
      {
        leftSpeed = -1 * map(data.js2Y, L_TREHOLD + 1, 1023, 0, 400);
        rightSpeed = map(data.js2Y, L_TREHOLD + 1, 1023, 0, 400);
        motors.setM1Speed(rightSpeed);
        motors.setM2Speed(leftSpeed);
        Serial.println("Rotate left");

        m1HasStop = false;
        m2HasStop = false;
      }
      // If only right is active
      else if (data.js2Y < R_TREHOLD)
      {
        leftSpeed = map(data.js2Y, 0, R_TREHOLD - 1, 400, 0);
        rightSpeed = -1 * map(data.js2Y, 0, R_TREHOLD - 1, 400, 0);
        
        motors.setM1Speed(rightSpeed);
        motors.setM2Speed(leftSpeed);
        Serial.println("Rotate right");
        
        m1HasStop = false;
        m2HasStop = false;
      }
      // No joystick is active
      else
      {
        if (!m1HasStop && !m2HasStop)
        {
          currSpeed = 0;
          motors.setSpeeds(currSpeed, currSpeed);
          m1HasStop = true;
          m2HasStop = true;
          Serial.println("stop");
        }
      }
    }
  }
}

void resetBuf(void)
{
  // See first
}