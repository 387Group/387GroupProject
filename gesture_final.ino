
// for both classes must be in the include path of your project
#include "I2Cdev.h"
 
#include "MPU6050_6Axis_MotionApps20.h"
 
#include<Servo.h>
Servo middle, left, right, claw;
#define pin5 5
#define pin7 7
#define pin12 12
#define pin8 8
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
 
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high
 
/* =========================================================================
   NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
   depends on the MPU-6050's INT pin being connected to the Arduino's
   external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
   digital I/O pin 2.
 
   For the Galileo Gen1/2 Boards, there is no INT pin support. Therefore
   the INT pin does not need to be connected, but you should work on getting
   the timing of the program right, so that there is no buffer overflow.
 * ========================================================================= */
 
/* =========================================================================
   NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
   when using Serial.write(buf, len). The Teapot output uses this method.
   The solution requires a modification to the Arduino USBAPI.h file, which
   is fortunately simple, but annoying. This will be fixed in the next IDE
   release. For more info, see these links:
 
   http://arduino.cc/forum/index.php/topic,109987.0.html
   http://code.google.com/p/arduino/issues/detail?id=958
 * ========================================================================= */
 
 
#define OUTPUT_READABLE_YAWPITCHROLL
 
// Unccomment if you are using an Arduino-Style Board
// #define ARDUINO_BOARD
 
// Uncomment if you are using a Galileo Gen1 / 2 Board
#define GALILEO_BOARD
 
#define LED_PIN 13      // (Galileo/Arduino is 13)
bool blinkState = false;
 
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
 
// orientation/motion vars
VectorFloat gravity;    // [x, y, z]            gravity vector
Quaternion q;           // [w, x, y, z]         quaternion container
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
 
 
 
// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================
 
// This function is not required when using the Galileo 
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}
 
 
 
// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================
 
void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        //int TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
 
    Serial.begin(115200);
    while (!Serial);
 
    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();
 
    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(F("MPU6050 connection "));
    Serial.print(mpu.testConnection() ? F("successful") : F("failed"));
 
    // wait for ready
    Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again
 
    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();
 
    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(124);
    mpu.setYGyroOffset(-78);
    mpu.setZGyroOffset(-53);
    mpu.setZAccelOffset(1212); // 1688 factory default for my test chip
 
    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);
 
        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(0, dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();
 
        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;
 
        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }
 
    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
    
    middle.attach(11);
    left.attach(10);
    right.attach(9);
    claw.attach(6);
    
    pinMode(pin5,OUTPUT);
    pinMode(pin7,OUTPUT);
    pinMode(pin12,OUTPUT);
    pinMode(pin8,OUTPUT);
}
 
 
 
// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================
 
void loop() {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;
 
    // wait for MPU interrupt or extra packet(s) available
 
    #ifdef ARDUINO_BOARD
        while (!mpuInterrupt && fifoCount < packetSize) {
        }
    #endif
 
    #ifdef GALILEO_BOARD
        delay(10);
    #endif
 
    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();
 
    // get current FIFO count
    fifoCount = mpu.getFIFOCount();
 
    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        Serial.println(F("FIFO overflow!"));
 
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
 
        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
 
 
        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            Serial.print("ypr\t");
            Serial.print(ypr[0] * 180/M_PI);
            Serial.print("\t");
            Serial.print(ypr[1] * 180/M_PI);
            Serial.print("\t");
            Serial.println(ypr[2] * 180/M_PI);
            
        #endif
 
        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
    }
    //initial position
   if((ypr[1]*180/M_PI)>-5 && (ypr[1]*180/M_PI)<5 && (ypr[2]*180/M_PI)>(-5) && (ypr[2]*180/M_PI)<5){//initial
      //middle.write(90);
      //left.write(125);
      //right.write(135);
     digitalWrite(pin5, LOW);
     digitalWrite(pin7, LOW);
     digitalWrite(pin12, LOW);
     digitalWrite(pin8, LOW);
   }
   else if(((ypr[1] * 180/M_PI)<-30 && (ypr[2] * 180/M_PI)<-40)){//up&left
      //middle.write(160);
      //left.write(160);
      //right.write(160);
         digitalWrite(pin5, HIGH);
     digitalWrite(pin7, LOW);
     digitalWrite(pin12, LOW);
     digitalWrite(pin8, LOW);
   }
    else if((ypr[1] * 180/M_PI)<-30 && (ypr[2]*180/M_PI)>-5 && (ypr[2]*180/M_PI)<5){//up
      //middle.write(90);
      //left.write(160);
      //right.write(160);
      digitalWrite(pin5, LOW);
     digitalWrite(pin7, HIGH);
     digitalWrite(pin12, LOW);
     digitalWrite(pin8, LOW);
   }
    else if((ypr[1] * 180/M_PI)<-30 && (ypr[2] * 180/M_PI)>20){//up right
      //middle.write(20);
      //left.write(160);
      //right.write(160);
      digitalWrite(pin5, HIGH);
     digitalWrite(pin7, HIGH);
     digitalWrite(pin12, LOW);
     digitalWrite(pin8, LOW);
   }
   else  if((ypr[1] * 180/M_PI)>-5&&(ypr[1] * 180/M_PI)<5&&(ypr[2] * 180/M_PI)<-40){//left
      //middle.write(160);
      //left.write(125);
      //right.write(135);
     digitalWrite(pin5, LOW);
     digitalWrite(pin7, LOW);
     digitalWrite(pin12, HIGH);
     digitalWrite(pin8, LOW);
   }
   else if((ypr[1] * 180/M_PI)>-5&&(ypr[1] * 180/M_PI)<5&&(ypr[2] * 180/M_PI)>20){//right
      //middle.write(20);
      //left.write(125);
      //right.write(135);
     digitalWrite(pin5, HIGH);
     digitalWrite(pin7, LOW);
     digitalWrite(pin12, HIGH);
     digitalWrite(pin8, LOW);
   }
   else if((ypr[1] * 180/M_PI)>30 && (ypr[2] * 180/M_PI)<-40){//down&left
      //middle.write(160);
      //left.write(90);
      //right.write(115);
      digitalWrite(pin5, LOW);
     digitalWrite(pin7, HIGH);
     digitalWrite(pin12, HIGH);
     digitalWrite(pin8, LOW);
   }
    else if((ypr[1] * 180/M_PI)>30 && (ypr[2]*180/M_PI)>-5 && (ypr[2]*180/M_PI)<5){//down
      //middle.write(90);
      //left.write(90);
      //right.write(115);
     digitalWrite(pin5, HIGH);
     digitalWrite(pin7, HIGH);
     digitalWrite(pin12, HIGH);
     digitalWrite(pin8, LOW);
   }
    else if((ypr[1] * 180/M_PI)>30 && (ypr[2] * 180/M_PI)>20){//down right
      //middle.write(20);
      //left.write(90);
      //right.write(115);
     digitalWrite(pin5, LOW);
     digitalWrite(pin7, LOW);
     digitalWrite(pin12, LOW);
     digitalWrite(pin8, HIGH);
    }
            
            
    
    
    
}
