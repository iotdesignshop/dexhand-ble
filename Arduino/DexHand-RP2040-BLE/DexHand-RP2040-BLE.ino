#include <ArduinoBLE.h>
#include <UniversalTimer.h>

#include "ManagedServo.h"
#include "Finger.h"
#include "Thumb.h"
#include "Wrist.h"
#include "WiFiNINA.h"


// ----- Servo Setup -----
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

#define DEMO_BUTTON 19    // Used for an optional external button to allow the hand to run through a series of canned poses


// The table below defines the servo parameters for each of the servos in the hand.
// You may need to adjust these if you are using different servos, or wire the
// servos to different GPIO pins.

ManagedServo managedServos[NUM_SERVOS] = 
{
  // Servo GPIO Pin, Min, Max, Default, Inverted
  ManagedServo(2, 30, 110, 30, false),  // Index Lower 0
  ManagedServo(1, 30, 140, 30, true),  // Index Upper 1
  ManagedServo(5, 30, 120, 30, false),  // Middle Lower 2
  ManagedServo(4, 30, 150, 30, true),  // Middle Upper 3
  ManagedServo(3, 30, 150, 30, true),   // Ring Lower 4
  ManagedServo(0, 30, 100, 30, false),   // Ring Upper  5
  ManagedServo(7, 30, 140, 30, true),   // Pinky Lower 6
  ManagedServo(6, 30, 100, 30, false),   // Pinky Upper 7
  ManagedServo(10, 30, 100, 30, false),  // Index Tip 8
  ManagedServo(11, 30, 90, 30, false), // Middle Tip 9
  ManagedServo(12, 30, 120, 30, true),  // Ring Tip 10
  ManagedServo(13, 30, 130, 30, true),  // Pinky Tip 11
  ManagedServo(14, 30, 130, 30, false),  // Thumb Tip 12
  ManagedServo(15, 30, 150, 30, false),  // Thumb Right 13
  ManagedServo(16, 20, 120, 20, false),  // Thumb Left 14
  ManagedServo(17, 30, 90, 30, false),  // Thumb Rotate 15
  ManagedServo(9, 30, 160, 95, false),  // Wrist Left 16
  ManagedServo(8, 30, 160, 95, false)   // Wrist Right 17
};


// Finger, Thumb, and Wrist objects for managing the DOF's in a more intuitive fashion.
typedef enum fingerIdx {
  FINGER_INDEX,
  FINGER_MIDDLE,
  FINGER_RING,
  FINGER_PINKY,
  NUM_FINGERS
} FINGER_IDX;

Finger fingers[NUM_FINGERS] = {
  Finger("index", managedServos[SERVO_INDEX_LOWER], managedServos[SERVO_INDEX_UPPER], managedServos[SERVO_INDEX_TIP]),
  Finger("middle", managedServos[SERVO_MIDDLE_LOWER], managedServos[SERVO_MIDDLE_UPPER], managedServos[SERVO_MIDDLE_TIP]),
  Finger("ring", managedServos[SERVO_RING_LOWER], managedServos[SERVO_RING_UPPER], managedServos[SERVO_RING_TIP]),
  Finger("pinky", managedServos[SERVO_PINKY_LOWER], managedServos[SERVO_PINKY_UPPER], managedServos[SERVO_PINKY_TIP])
};

Thumb thumb(managedServos[SERVO_THUMB_LEFT], managedServos[SERVO_THUMB_RIGHT], managedServos[SERVO_THUMB_TIP], managedServos[SERVO_THUMB_ROTATE]);
Wrist wrist(managedServos[SERVO_WRIST_L], managedServos[SERVO_WRIST_R]);






// ----- BLE Setup -----

// The device implments the Nordic UART service to use as a general purpose command
// channel, and a custom service to use for streaming the DOF angles for the joints 
// in the hand.

// UART service
BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART service

// UART characteristics
BLECharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 20);
BLECharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite, 20);

// Custom DOF streaming service
BLEService dofService("1e16c1b4-1936-4f0e-ab62-5e0a702a4935");

// Custom DOF streaming characteristics
BLECharacteristic dofCharacteristic("1e16c1b5-1936-4f0e-ab62-5e0a702a4935", BLEWriteWithoutResponse, 20);


// Heartbeat timer
uint32_t heartbeat = 0;
UniversalTimer heartbeatTimer(5000, true);  // 5 second message interval

// Connection timeout timer
UniversalTimer connectionTimeout(10000, true); // 10 second timeout

// Reset servos to default position
void setDefaultPose() {
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    managedServos[index].setServoPosition(managedServos[index].getDefaultPosition());
  }
}


// -- Canned Procedural Poses for Testing -----------------------

// Hand pose for countdown - all fingers closed
void setZeroPose() {
  
  setDefaultPose();
  
  // Move thumb to lower closed position
  thumb.setMaxPosition();

  // Move all fingers to max position
  for (int finger = 0; finger < NUM_FINGERS; finger++)
  {
    fingers[finger].setMaxPosition();
    fingers[finger].update();
  }
}

// Hand pose for countdown - one finger open
void setOnePose() {

  setDefaultPose();
  
  // Move thumb to lower closed position
  thumb.setMaxPosition();
  
  // Move all fingers other than index to max position
  for (int finger = 1; finger < NUM_FINGERS; finger++)
  {
    fingers[finger].setMaxPosition();
    fingers[finger].update();
  }

}

// Hand pose for countdown - two fingers open
void setTwoPose() {

  setDefaultPose();

  // Move thumb to lower closed position
  thumb.setMaxPosition();
  
  // Move all fingers other than index,middle finger to max position
  for (int finger = 2; finger < NUM_FINGERS; finger++)
  {
    fingers[finger].setMaxPosition();
    fingers[finger].update();
  }

}

// Hand pose for countdown - three fingers open
void setThreePose() {

  setDefaultPose();

  // Move thumb to lower closed position
  thumb.setMaxPosition();
  
  // Move all fingers other than index,middle,ring finger to max position
  for (int finger = 3; finger < NUM_FINGERS; finger++)
  {
    fingers[finger].setMaxPosition();
    fingers[finger].update();
  }
}

// Hand pose for countdown - four fingers open
void setFourPose() {

  setDefaultPose();

  // Move thumb to lower closed position
  thumb.setMaxPosition();
  
}



// Countdown and back up
void count() {

  // Move thumb to lower closed position
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  delay(200);

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

// Waves the hand side to side
void wave() {
  setDefaultPose();
  for (int cycle = 0; cycle < 5; ++cycle) {
    for (int yaw = wrist.getYawMin(); yaw <= wrist.getYawMax(); yaw ++) {
      wrist.setYaw(yaw);
      wrist.update();
      delay(5);
    }
    for (int yaw = wrist.getYawMax(); yaw >= wrist.getYawMin(); yaw --) {
      wrist.setYaw(yaw);
      wrist.update();
      delay(5);
    }
  }
  setDefaultPose();
}

// Perform a shaka
void shaka()
{
  const int SHAKA_RANGE = 10;
  setDefaultPose();

  for (int finger = FINGER_INDEX; finger < FINGER_PINKY; ++finger)
  {
    fingers[finger].setMaxPosition();
    fingers[finger].update();
  }
  thumb.setPitch(0);
  thumb.setYaw(0);
  thumb.setRoll(15);
  thumb.update();

  for (int cycle = 0; cycle < 5; ++cycle) {
    for (int yaw = -SHAKA_RANGE; yaw <= SHAKA_RANGE; yaw ++) {
      wrist.setYaw(yaw);
      wrist.update();
      delay(10);
    }
    for (int yaw = SHAKA_RANGE; yaw >= -SHAKA_RANGE; yaw --) {
      wrist.setYaw(yaw);
      wrist.update();
      delay(10);
    }
  }

  setDefaultPose();

}

void thumbRangeTest()
{
  // Cycle through the two thumb servos testing the range
  for (int right = managedServos[SERVO_THUMB_RIGHT].getMinPosition(); right <= managedServos[SERVO_THUMB_RIGHT].getMaxPosition(); right+=5)
  {
    managedServos[SERVO_THUMB_RIGHT].setServoPosition(right);
    
    for (int left = managedServos[SERVO_THUMB_LEFT].getMinPosition(); left <= managedServos[SERVO_THUMB_LEFT].getMaxPosition(); left+=5)
    {
      managedServos[SERVO_THUMB_LEFT].setServoPosition(left);
      delay(100);
    }
  }
  setDefaultPose();
}

void defaultFingers()
{
  for (int finger = FINGER_INDEX; finger <= FINGER_PINKY; ++finger)
  {
    fingers[finger].setExtension(100);
    fingers[finger].update();
  }
  thumb.setExtension(100);
  thumb.update();
}
void fingerTest()
{

  defaultFingers();
  delay(1000);

  // Pinky
  fingers[FINGER_PINKY].setExtension(25);
  fingers[FINGER_PINKY].update();
  thumb.setPitch(60);
  thumb.setYaw(0);
  thumb.setFlexion(45);
  thumb.setRoll(0);
  thumb.update();
  delay(1000);

  defaultFingers();
  delay(1000);

  // Ring
  fingers[FINGER_RING].setExtension(30);
  fingers[FINGER_RING].update();
  thumb.setPitch(60);
  thumb.setYaw(0);
  thumb.setFlexion(30);
  thumb.setRoll(0);
  thumb.update();
  delay(1000);

  defaultFingers();
  delay(1000);

  // Middle
  fingers[FINGER_MIDDLE].setExtension(35);
  fingers[FINGER_MIDDLE].update();
  thumb.setPitch(50);
  thumb.setYaw(0);
  thumb.setFlexion(40);
  thumb.setRoll(0);
  thumb.update();
  delay(1000);

  defaultFingers();
  delay(1000);

  // Index
  fingers[FINGER_INDEX].setExtension(35);
  fingers[FINGER_INDEX].update();
  thumb.setPitch(40);
  thumb.setYaw(0);
  thumb.setFlexion(45);
  thumb.setRoll(0);
  thumb.update();
  delay(1000);

  defaultFingers();
  delay(1000);
}

// Dump out the current DOF angles
String printDOFS()
{
  // This command outputs a JSON array of all of the angle ranges for the DOFS in the hand
    String result = "DOFS:[ ";

    // Would be nicer if this could be more automated based on the data structures, but for
    // now we just output the values in the right order

    for (int finger = 0; finger < NUM_FINGERS; ++finger)
    {
      // Fingers
      result += "{ \"name\": \"";
      result += fingers[finger].getName();
      result += "_pitch\", \"range\": [";
      result += fingers[finger].getPitchMin();
      result += ", ";
      result += fingers[finger].getPitchMax();
      result += "] }, ";

      result += "{ \"name\": \"";
      result += fingers[finger].getName();
      result += "_yaw\", \"range\": [";
      result += fingers[finger].getYawMin();
      result += ", ";
      result += fingers[finger].getYawMax();
      result += "] }, ";

      result += "{ \"name\": \"";
      result += fingers[finger].getName();
      result += "_flexion\", \"range\": [";
      result += fingers[finger].getFlexionMin();
      result += ", ";
      result += fingers[finger].getFlexionMax();
      result += "] }, ";
    }
    
    // Thumb
    result += "{ \"name\": \"thumb_pitch\", \"range\": [";
    result += thumb.getPitchMin();
    result += ", ";
    result += thumb.getPitchMax();
    result += "] }, ";

    result += "{ \"name\": \"thumb_yaw\", \"range\": [";
    result += thumb.getYawMin();
    result += ", ";
    result += thumb.getYawMax();
    result += "] }, ";

    result += "{ \"name\": \"thumb_flexion\", \"range\": [";
    result += thumb.getFlexionMin();
    result += ", ";
    result += thumb.getFlexionMax();
    result += "] }, ";

    // Roll is currently not used - this is here for when we want to enable it
    //result += "{ \"name\": \"thumb_roll\", \"range\": [";
    //result += thumb.getRollMin();
    //result += ", ";
    //result += thumb.getRollMax();
    //result += "] }, ";

    // Wrist
    result += "{ \"name\": \"wrist_pitch\", \"range\": [";
    result += wrist.getPitchMin();
    result += ", ";
    result += wrist.getPitchMax();
    result += "] }, ";

    result += "{ \"name\": \"wrist_yaw\", \"range\": [";
    result += wrist.getYawMin();
    result += ", ";
    result += wrist.getYawMax();
    result += "] } ";
    
    result += "]";

    return result;
}


// --- Main Setup -----------------------

void setup() {
  Serial.begin(9600);    // initialize serial communication
  delay(2000);

  // ----- Servo Setup -----
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    managedServos[index].setupServo();
  }

  
  // ----- BLE Setup -----
  if (!BLE.begin()) {   // initialize BLE
    Serial.println("starting BLE failed!");
    while (1);
  }

  BLE.setConnectionInterval(6, 10); // Ask for a fast connection interval: 7.5 ms minimum, 12.5s maximum

  heartbeatTimer.start();  // Start heartbeat timer
  connectionTimeout.start(); // Start timeout

  BLE.setLocalName("DexHand");  // Set name for connection
  BLE.setAdvertisedService(uartService); // Add the service UUID
  
  uartService.addCharacteristic(rxCharacteristic); // Add the rxCharacteristic
  uartService.addCharacteristic(txCharacteristic); // Add the txCharacteristic

  dofService.addCharacteristic(dofCharacteristic); // Add the dofCharacteristic

  BLE.addService(uartService); // Add the service
  BLE.addService(dofService); // Add the service

  rxCharacteristic.setEventHandler(BLEWritten, rxHandler);  // Assign event handler for characteristic
  dofCharacteristic.setEventHandler(BLEWritten, dofHandler);  // Assign event handler for characteristic
  
  BLE.advertise();  // Start advertising
  Serial.println("Bluetooth device active, waiting for connections...");

  setDefaultPose();

  // ----- Demo Button Setup -----
  pinMode(DEMO_BUTTON, INPUT_PULLUP);

}

// --- Main Loop and Processing -------------------------------

// Basic command parser for servo commands - nothing special, but it works
// See the README.md for details on the commands and format
void processCommand(String cmd) {

  cmd.toLowerCase();

  // Split the string
  int colonPos = cmd.indexOf(':');
  String cmdType;
  String servoIndex;
  String servoPosition;
  int index = 0;
  int position = 0;
  
  if (colonPos != -1) {
    cmdType = cmd.substring(0, colonPos);
    cmd = cmd.substring(colonPos + 1);
    colonPos = cmd.indexOf(':');
    servoIndex = cmd.substring(0, colonPos);
    
    if (colonPos != -1) {
      servoPosition = cmd.substring(colonPos + 1);
      position = servoPosition.toInt();
    }

    // Convert to integers
    index = servoIndex.toInt();
    
  }
  else {
    // Single word command
    cmdType = cmd;
    cmdType.trim();
  }

  
  Serial.print("CMD:");
  Serial.print(cmdType);
  Serial.print(":");
  Serial.print(index);
  Serial.print(":");
  Serial.println(position);

  // Set the servo position
  if (cmdType == "set") {
    Serial.print("Setting Servo ");
    Serial.print(index);
    Serial.print(" to ");
    Serial.println(position);

    managedServos[index].setServoPosition(position);
  }
  if (cmdType == "max") {
    if (position != 0)
    {
      managedServos[index].setMaxPosition(position);
    }
    Serial.print("Setting Servo ");
    Serial.print(index);
    Serial.print(" to max");

    managedServos[index].moveToMaxPosition();
  }
  if (cmdType == "min") {
    if (position != 0)
    {
      managedServos[index].setMinPosition(position);
    }
    Serial.print("Setting Servo ");
    Serial.print(index);
    Serial.print(" to min");

    managedServos[index].moveToMinPosition();
  }
  if (cmdType == "fingermax") {

    if (index < NUM_FINGERS) {
      fingers[index].setMaxPosition();
      fingers[index].update();
    
      Serial.print("Setting finger ");
      Serial.print(index);
      Serial.print(" to max");
    }
  }
  if (cmdType == "fingermin") {

    if (index < NUM_FINGERS) {
      fingers[index].setMinPosition();
      fingers[index].update();
    
      Serial.print("Setting finger ");
      Serial.print(index);
      Serial.print(" to min");
    }
  }
  if (cmdType == "fingerextension") {
    // Accepts range from 0-100 where 0 is fully retracted toward palm, and 100 is fully extended away from palm
    if (index < NUM_FINGERS)
    {
      fingers[index].setExtension(position);
      fingers[index].update();

      Serial.print("Setting finger ");
      Serial.print(index);
      Serial.print(" extension to ");
      Serial.println(position);
    }
    else if (index == NUM_FINGERS)
    {
      thumb.setExtension(position);
      thumb.update();

      Serial.print("Setting thumb extension to ");
      Serial.println(position);
    }
  }
  if (cmdType == "wrist") {
    if (servoIndex == "pitch") {
      wrist.setPitch(position);
      wrist.update();

      Serial.print("Setting wrist pitch to ");
      Serial.println(position);
    }
    if (servoIndex == "yaw") {
      wrist.setYaw(position);
      wrist.update();

      Serial.print("Setting wrist yaw to ");
      Serial.println(position);
    }
  }
  if (cmdType == "thumb") {
    if (servoIndex == "pitch") {
      thumb.setPitch(position);
      thumb.update();

      Serial.print("Setting thumb pitch to ");
      Serial.println(position);
    }
    if (servoIndex == "yaw") {
      thumb.setYaw(position);
      thumb.update();

      Serial.print("Setting thumb yaw to ");
      Serial.println(position);
    }
    if (servoIndex == "flexion") {
      thumb.setFlexion(position);
      thumb.update();

      Serial.print("Setting thumb flexion to ");
      Serial.println(position);
    }
    if (servoIndex == "roll") {
      thumb.setRoll(position);
      thumb.update();

      Serial.print("Setting thumb roll to ");
      Serial.println(position);
    }
  }
  if (cmdType == "one") {
    setOnePose();
  }
  if (cmdType == "two") {
    setTwoPose();
  }
  if (cmdType == "three") {
    setThreePose();
  }
  if (cmdType == "four") {
    setFourPose();
  }
  if (cmdType == "default") {
    setDefaultPose();
  }
  if (cmdType == "count") {
    count();
  }
  if (cmdType == "wave") {
    wave();
  }
  if (cmdType == "shaka") {
    shaka();
  }
  if (cmdType == "thumbtest") {
    thumbRangeTest();
  }
  if (cmdType == "fingertest") {
    fingerTest();
  }
  if (cmdType == "hb") {
    connectionTimeout.resetTimerValue();
    Serial.println("HB: Heartbeat received");
  }
  if (cmdType == "gesture") {
    if (servoIndex == "count") {
      count();
    }
    if (servoIndex == "wave") {
      wave();
    }
    if (servoIndex == "shaka") {
      shaka();
    }
    if (servoIndex == "reset") {
      setDefaultPose();
      delay(500);
    }
  }
  if (cmdType == "dofs") {
    Serial.println(printDOFS().c_str());
  }
}


// This method is called to process all of the higher level objects (Finger,Thumb)
// and update the servo positions based on the current hand angles.
void updateHand() {
  // Update fingers
  for (int index = 0; index < NUM_FINGERS; index++) {
    fingers[index].update();
  }
  // Update thumb
  thumb.update();

  // Update wrist
  wrist.update();

}

void loop() {

  // ---- Serial Input Loop -----
  // If there is no active BLE connection to the peripheral, then we will
  // process serial commands for debugging/tuning/testing etc.

  // Is there serial data available for input?
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');

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
      
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    setDefaultPose();
  }

  // ----- Demo Button Loop -----
  // If the demo button is pressed, then we will run through a series of canned poses
  // to demonstrate the hand functionality
  if (digitalRead(DEMO_BUTTON) == LOW) {
    Serial.println("Demo button pressed");
    wave();
    delay(500);
    fingerTest();
    delay(500);
    count();
    delay(500);
    shaka();
    delay(500);
    setDefaultPose();
    delay(500);
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
  }  
}

#define DOF_COUNT 17
void dofHandler(BLEDevice central, BLECharacteristic characteristic) {

  // The angles are packed into 8-bit values centered at 128, so we need to
  // unpack them and convert them back into decimal angles for the servos
  const uint8_t* data = characteristic.value();

  // Simple data integrity checks
  if (characteristic.valueLength() != DOF_COUNT+1) {
    Serial.print("Invalid DOF length: ");
    Serial.println(characteristic.valueLength());
    return;
  }
  
  // Last byte is a checksum - check against other bytes
  uint8_t checksum = 0;
  for (int i = 0; i < DOF_COUNT; i++) {
    checksum += data[i];
  }
  if (checksum != data[DOF_COUNT]) {
    Serial.print("Invalid DOF checksum: ");
    Serial.println(checksum);
    return;
  }

  Serial.println(characteristic.valueLength());
  int16_t unpackedAngles[DOF_COUNT];

  for (int i = 0; i < DOF_COUNT; i++) {
    float angle = (data[i] - 127)*360.0/256.0;
    unpackedAngles[i] = static_cast<int16_t>(angle);
  }

  #ifdef DEBUG  // Useful debug prints for tuning
  // Print out the angles
  Serial.print("DOF: ");
    
  for (int i = 0; i < DOF_COUNT; i++) {
    Serial.print(unpackedAngles[i]);
    Serial.print(" ");
  }
  Serial.println();
  #endif


  // Fingers first
  for (int i = 0; i < NUM_FINGERS; i++) {
    fingers[i].setPitch(unpackedAngles[i*3]);
    fingers[i].setYaw(unpackedAngles[i*3+1]);
    fingers[i].setFlexion(unpackedAngles[i*3+2]);
  }

  // Thumb
  thumb.setPitch(unpackedAngles[12]);
  thumb.setYaw(unpackedAngles[13]);
  thumb.setFlexion(unpackedAngles[14]);

  // Wrist
  wrist.setPitch(unpackedAngles[15]);
  wrist.setYaw(unpackedAngles[16]);


  updateHand();
    
}
