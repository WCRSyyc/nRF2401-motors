/*
* Use data received from nRF24L01 radio to set motor speed and direction.
* Report received data to serial port for testing and debugging
*
* Initially based on the GettingStarted RF24 library and the
* DCMotorTest Adafruit Motor Shield v2 library example sketches.
*
* Use nRF2401-Joy2 sketch, or similar to send data needed here
* https://github.com/WCRSyyc/nRF2401-Joy2.git
*/

#include <SPI.h>
#include "RF24.h"

#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Configure which ports are used on the motor shield
const unsigned int LEFT_MOTOR_PORT = 1;  // M1
const unsigned int RIGHT_MOTOR_PORT = 3;  // M3
const byte addresses[][6] = {"1Node", "2Node"};  // read and write channels

// Data Packet (payload)
struct appPacket {
  int buffType;
  int x, y, z;
};

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotorL = AFMS.getMotor(LEFT_MOTOR_PORT);
Adafruit_DCMotor *myMotorR = AFMS.getMotor(RIGHT_MOTOR_PORT);

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7, 8);

appPacket myMessage;


void setup() {
  Serial.begin(115200);
  Serial.println(F("From RF24 to Adafruit Motorshield v2"));

  AFMS.begin();  // create with the default frequency 1.6KHz
  myMotorL->run(RELEASE); // turn on motors (stopped)
  myMotorR->run(RELEASE);

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // test and debugging sketch, and the likelihood of close proximity of the
  // devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(addresses[1]);  // Pipe to write (respond) to
  // TODO: verify, but probably the writing pipe is not needed:
  //   auto ack uses the reading pipe?
  radio.openReadingPipe(1,addresses[0]);  // Pipe to listen to (read)
  radio.startListening();  // Start the radio listening for data
}

void loop() {
  while (radio.available()) {      // While there is data ready from the radio
    //Serial.println("available");  // Maybe could toggle LED
    radio.read(&myMessage, sizeof(appPacket));  // Get the payload

    // Show the packet content
    Serial.print(myMessage.x);
    Serial.print(F(", "));
    Serial.print(myMessage.y);
    Serial.print(F(", "));
    Serial.println(myMessage.z);

    // Simplified motor control: both motors set to speed of `y` value
    if((myMessage.y >= -10) && (myMessage.y <= 10)) {  // Very low speed, assume stop
      myMotorL->run(RELEASE);
      myMotorR->run(RELEASE);
      Serial.println(F("Release"));
    } else if (myMessage.y <= 0) {  // negative speed means reverse direction
      myMotorL->run(BACKWARD);
      myMotorR->run(BACKWARD);
      Serial.print(F("Back "));
      Serial.println(abs(myMessage.y));
      myMotorL->setSpeed(abs(myMessage.y));
      myMotorR->setSpeed(abs(myMessage.y));
    } else {
      myMotorL->run(FORWARD);
      myMotorR->run(FORWARD);
      Serial.print(F("Forward "));
      Serial.println(myMessage.y);
      myMotorL->setSpeed(myMessage.y);
      myMotorR->setSpeed(myMessage.y);
    }
  } // end of while radio avail
} // ./loop
