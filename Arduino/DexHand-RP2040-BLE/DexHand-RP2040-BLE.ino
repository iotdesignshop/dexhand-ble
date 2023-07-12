#include <ArduinoBLE.h>
#include <UniversalTimer.h>

#include "ManagedServo.h"





#define SERVO_PIN_1       2
#define SERVO_PIN_2       3
#define SERVO_PIN_3       4
#define SERVO_PIN_4       5
#define SERVO_PIN_5       6
#define SERVO_PIN_6       7
#define SERVO_PIN_7       8
#define SERVO_PIN_8       9
#define SERVO_PIN_9       10
#define SERVO_PIN_10      11
#define SERVO_PIN_11      12
#define SERVO_PIN_12      13
#define SERVO_PIN_13      14
#define SERVO_PIN_14      15
#define SERVO_PIN_15      16
#define SERVO_PIN_16      17
#define SERVO_PIN_17      18
#define SERVO_PIN_18      19



#define NUM_SERVOS       18

#define SERVO_INDEX_LOWER   0
#define SERVO_INDEX_UPPER   1
#define SERVO_MIDDLE_LOWER  2
#define SERVO_MIDDLE_UPPER  3
#define SERVO_RING_LOWER  4
#define SERVO_RING_UPPER  5
#define SERVO_PINKY_LOWER  6
#define SERVO_PINKY_UPPER  7
#define SERVO_INDEX_TIP 8
#define SERVO_MIDDLE_TIP 9
#define SERVO_RING_TIP 10
#define SERVO_PINKY_TIP 11
#define SERVO_THUMB_TIP 12
#define SERVO_THUMB_RIGHT 13
#define SERVO_THUMB_LEFT 14
#define SERVO_THUMB_ROTATE 15
#define SERVO_WRIST_L 16
#define SERVO_WRIST_R 17


ManagedServo managedServos[NUM_SERVOS] = 
{
  ManagedServo(SERVO_PIN_1, 10, 120, 10, false),  // Index Lower 0
  ManagedServo(SERVO_PIN_2, 60, 160, 60, false),  // Index Upper 1
  ManagedServo(SERVO_PIN_3, 50, 150, 50, false),  // Middle Lower 2
  ManagedServo(SERVO_PIN_4, 50, 160, 50, false),  // Middle Upper 3
  ManagedServo(SERVO_PIN_5, 20, 140, 20, true),   // Ring Lower 4
  ManagedServo(SERVO_PIN_6, 20, 140, 20, true),   // Ring Upper  5
  ManagedServo(SERVO_PIN_7, 20, 140, 20, true),   // Pinky Lower 6
  ManagedServo(SERVO_PIN_8, 20, 140, 20, true),   // Pinky Upper 7
  ManagedServo(SERVO_PIN_9, 50, 120, 50, false),  // Index Tip 8
  ManagedServo(SERVO_PIN_10, 50, 120, 50, false), // Middle Tip 9
  ManagedServo(SERVO_PIN_11, 50, 120, 50, true),  // Ring Tip 10
  ManagedServo(SERVO_PIN_12, 50, 130, 50, true),  // Pinky Tip 11
  ManagedServo(SERVO_PIN_13, 50, 160, 50, true),  // Thumb Tip 12
  ManagedServo(SERVO_PIN_14, 0, 130, 0, false),  // Thumb Right 13
  ManagedServo(SERVO_PIN_15, 0, 150, 10, false),  // Thumb Left 14
  ManagedServo(SERVO_PIN_16, 80, 110, 90, false),  // Thumb Rotate 15
  ManagedServo(SERVO_PIN_17, 0, 180, 90, false),  // Wrist Left 16
  ManagedServo(SERVO_PIN_18, 0, 180, 90, false)   // Wrist Right 17
};





// ----- BLE Setup -----
BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART service

// UART characteristics
BLECharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 32);
BLECharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite, 32);

// Heartbeat timer
uint32_t heartbeat = 0;
UniversalTimer heartbeatTimer(5000, true);  // 5 second message interval

// Connection timeout timer
UniversalTimer connectionTimeout(10000, true); // 10 second timeout


void setDefaultPose() {
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    managedServos[index].setServoPosition(managedServos[index].getDefaultPosition());
  }
}

void setZeroPose()
{
  setDefaultPose();
  
  // Move all thumb joints to min
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  managedServos[SERVO_THUMB_ROTATE].setServoPosition(120);

  // Move all fingers other than index finger to max position
  for (int index = SERVO_INDEX_LOWER; index <= SERVO_PINKY_UPPER; index++)
  {
    managedServos[index].moveToMaxPosition();
  }
  // Move all tips other than index to max position
  for (int index = SERVO_INDEX_TIP; index <= SERVO_PINKY_TIP; index++)
  {
    managedServos[index].moveToMaxPosition();
  }


}

void setOnePose()
{
  setDefaultPose();
  
  // Move all thumb joints to min
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  managedServos[SERVO_THUMB_ROTATE].setServoPosition(120);

  // Move all fingers other than index finger to max position
  for (int index = SERVO_MIDDLE_LOWER; index <= SERVO_PINKY_UPPER; index++)
  {
    managedServos[index].moveToMaxPosition();
  }
  // Move all tips other than index to max position
  for (int index = SERVO_MIDDLE_TIP; index <= SERVO_PINKY_TIP; index++)
  {
    managedServos[index].moveToMaxPosition();
  }

}

void setTwoPose()
{
  setDefaultPose();

  // Move all thumb joints to min
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  managedServos[SERVO_THUMB_ROTATE].setServoPosition(120);


  // Move all fingers other than index,middle finger to max position
  for (int index = SERVO_RING_LOWER; index <= SERVO_PINKY_UPPER; index++)
  {
    managedServos[index].moveToMaxPosition();
  }
  // Move all tips other than index,middle to max position
  for (int index = SERVO_RING_TIP; index <= SERVO_PINKY_TIP; index++)
  {
    managedServos[index].moveToMaxPosition();
  }
}

void setThreePose()
{
  setDefaultPose();

  // Move all thumb joints to min
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  managedServos[SERVO_THUMB_ROTATE].setServoPosition(120);



  for (int index = SERVO_PINKY_LOWER; index <= SERVO_PINKY_UPPER; index++)
  {
    managedServos[index].moveToMaxPosition();
  }
  for (int index = SERVO_PINKY_TIP; index <= SERVO_PINKY_TIP; index++)
  {
    managedServos[index].moveToMaxPosition();
  }
}

void setFourPose()
{
  
  setDefaultPose();

  // Move all thumb joints to min
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  managedServos[SERVO_THUMB_ROTATE].setServoPosition(120);

}



void count()
{
  setZeroPose();
  delay(1000);
  setOnePose();
  delay(1000);
  setTwoPose();
  delay(1000);
  setThreePose();
  delay(1000);
  setFourPose();
  delay(1000);
  setDefaultPose();
  delay(1000);
  setFourPose();
  delay(1000);
  setThreePose();
  delay(1000);
  setTwoPose();
  delay(1000);
  setOnePose();
  delay(1000);
  setZeroPose();
  delay(1000);
  setDefaultPose();
  
}



void setup() {
  Serial.begin(9600);    // initialize serial communication

  // ----- Servo Setup -----
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    pinMode(managedServos[index].getServoPin(), OUTPUT);
    digitalWrite(managedServos[index].getServoPin(), LOW);
    managedServos[index].setupServo();
  }

  
  // ----- BLE Setup -----
  if (!BLE.begin()) {   // initialize BLE
    Serial.println("starting BLE failed!");
    while (1);
  }

  heartbeatTimer.start();  // Start heartbeat timer
  connectionTimeout.start(); // Start timeout

  BLE.setLocalName("DexHand");  // Set name for connection
  BLE.setAdvertisedService(uartService); // Add the service UUID
  
  uartService.addCharacteristic(rxCharacteristic); // Add the rxCharacteristic
  uartService.addCharacteristic(txCharacteristic); // Add the txCharacteristic

  BLE.addService(uartService); // Add the service
  rxCharacteristic.setEventHandler(BLEWritten, rxHandler);  // Assign event handler for characteristic
  
  BLE.advertise();  // Start advertising
  Serial.println("Bluetooth device active, waiting for connections...");

  setDefaultPose();

}


void processCommand(String cmd)
{
  // Basic command parser for servo commands - nothing special, but it works
  // Commands are of the form: "S:1:90" where S is the command, 1 is the servo index, and 90 is the position

  cmd.toLowerCase();

  // Split the string
  int colonPos = cmd.indexOf(':');
  String cmdType;
  String servoIndex;
  String servoPosition;
  int index = 0;
  int position = 0;

  if (colonPos != -1)
  {
    cmdType = cmd.substring(0, colonPos);
    cmd = cmd.substring(colonPos + 1);
    colonPos = cmd.indexOf(':');
    servoIndex = cmd.substring(0, colonPos);
    servoPosition = cmd.substring(colonPos + 1);

    // Convert to integers
    index = servoIndex.toInt();
    position = servoPosition.toInt();

  }
  else
  {
    // Single word command
    cmdType = cmd;
    cmdType.trim();
  }

  
  // Clamp servo position
  if (position < 0) position = 0;
  if (position > 180) position = 180;

  Serial.print("CMD:");
  Serial.print(cmdType);
  Serial.print(":");
  Serial.print(index);
  Serial.print(":");
  Serial.println(position);

  // Set the servo position
  if (cmdType == "set")
  {
    Serial.print("Setting Servo ");
    Serial.print(index);
    Serial.print(" to ");
    Serial.println(position);

    managedServos[index].setServoPosition(position);
  }
  if (cmdType == "max")
  {
    if (position != 0)
    {
      managedServos[index].setMaxPosition(position);
    }
    Serial.print("Setting Servo ");
    Serial.print(index);
    Serial.print(" to max");

    managedServos[index].moveToMaxPosition();
  }
  if (cmdType == "min")
  {
    if (position != 0)
    {
      managedServos[index].setMinPosition(position);
    }
    Serial.print("Setting Servo ");
    Serial.print(index);
    Serial.print(" to min");

    managedServos[index].moveToMinPosition();
  }
  if (cmdType == "one")
  {
    setOnePose();
  }
  if (cmdType == "two")
  {
    setTwoPose();
  }
  if (cmdType == "three")
  {
    setThreePose();
  }
  if (cmdType == "four")
  {
    setFourPose();
  }
  if (cmdType == "default")
  {
    setDefaultPose();
  }
  if (cmdType == "count")
  {
    count();
  }
  if (cmdType == "hb")
  {
    connectionTimeout.resetTimerValue();
  }

}


char buffer[255];
int bufPos = 0;

void loop() {

  // ---- Serial Input Loop -----
  // If there is no active BLE connection to the peripheral, then we will
  // process serial commands for debugging/tuning/testing etc.

  // Is there serial data available for input?
  if (Serial.available()) {
    String cmd = Serial.readString();

    Serial.print("Received CMD: ");
    Serial.println(cmd);

    processCommand(cmd);
  }

  
  // ----- BLE Loop -----
  // If there is an active BLE connection to the peripheral, then we will
  // ignore serial processing and run in a tight loop where we receive
  // packets with the hand angles and convert those into servo positions
  
  BLEDevice central = BLE.central();  // wait for a BLE central

  if (central) {  // if a central is connected to the peripheral:
    Serial.print("Connected to central - entering BLE streaming mode:");
    Serial.println(central.address());  // print the central's MAC address
    
    // Start a fresh connection timeout timer
    connectionTimeout.resetTimerValue();

    while (central.connected()) {  // while the central is still connected to peripheral:
       
      // Check if it's time to send a heartbeat
      if (heartbeatTimer.check()) {
        heartbeat++;
        String data = "HB:" + String(heartbeat);
        txCharacteristic.writeValue(data.c_str());
      }

      // Make sure we've received a heartbeat from the central recently
      if (connectionTimeout.check())
      {
        Serial.println("Connection timeout - disconnecting");
        central.disconnect();
        break;
      }

      // See if there's any data for us
      //if (rxCharacteristic.written()) {
      //  Serial.print("BLELoop: ");
      //  Serial.println(reinterpret_cast<const char*>(rxCharacteristic.value()));
      //}
      
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void rxHandler(BLEDevice central, BLECharacteristic characteristic) {
  String receivedData = reinterpret_cast<const char*>(characteristic.value());  // get the data

  // Extract all data up to newline
  int newlinePos = receivedData.indexOf('\n');
  if (newlinePos != -1)
  {
    receivedData = receivedData.substring(0, newlinePos);
    processCommand(receivedData);

    Serial.print("rxHandler:");
    Serial.println(receivedData);  // print the data
  }

  
  
}
