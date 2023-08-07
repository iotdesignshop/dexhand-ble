#include <ArduinoBLE.h>
#include <UniversalTimer.h>

#include "ManagedServo.h"
#include "Finger.h"
#include "Thumb.h"
#include "Wrist.h"



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


// The table below defines the servo parameters for each of the servos in the hand.
// You may need to adjust these if you are using different servos, or wire the
// servos to different GPIO pins.

ManagedServo managedServos[NUM_SERVOS] = 
{
  // Servo GPIO Pin, Min, Max, Default, Inverted
  ManagedServo(2, 20, 130, 20, false),  // Index Lower 0
  ManagedServo(3, 20, 130, 20, false),  // Index Upper 1
  ManagedServo(4, 30, 140, 30, false),  // Middle Lower 2
  ManagedServo(5, 30, 140, 30, false),  // Middle Upper 3
  ManagedServo(7, 20, 140, 20, true),   // Ring Lower 4
  ManagedServo(6, 20, 140, 20, true),   // Ring Upper  5
  ManagedServo(9, 20, 140, 20, true),   // Pinky Lower 6
  ManagedServo(8, 20, 150, 20, true),   // Pinky Upper 7
  ManagedServo(10, 50, 100, 50, false),  // Index Tip 8
  ManagedServo(11, 30, 90, 50, false), // Middle Tip 9
  ManagedServo(12, 50, 120, 50, true),  // Ring Tip 10
  ManagedServo(13, 70, 110, 70, true),  // Pinky Tip 11
  ManagedServo(14, 20, 120, 20, true),  // Thumb Tip 12
  ManagedServo(15, 20, 160, 20, false),  // Thumb Right 13
  ManagedServo(16, 20, 150, 20, false),  // Thumb Left 14
  ManagedServo(17, 20, 95, 20, false),  // Thumb Rotate 15
  ManagedServo(0, 20, 160, 80, false),  // Wrist Left 16
  ManagedServo(1, 20, 160, 80, false)   // Wrist Right 17
};

#define NUM_FINGERS 4
Finger fingers[NUM_FINGERS] = {
  Finger(managedServos[SERVO_INDEX_LOWER], managedServos[SERVO_INDEX_UPPER], managedServos[SERVO_INDEX_TIP]),
  Finger(managedServos[SERVO_MIDDLE_LOWER], managedServos[SERVO_MIDDLE_UPPER], managedServos[SERVO_MIDDLE_TIP]),
  Finger(managedServos[SERVO_RING_LOWER], managedServos[SERVO_RING_UPPER], managedServos[SERVO_RING_TIP]),
  Finger(managedServos[SERVO_PINKY_LOWER], managedServos[SERVO_PINKY_UPPER], managedServos[SERVO_PINKY_TIP])
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


void setDefaultPose() {
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    managedServos[index].setServoPosition(managedServos[index].getDefaultPosition());
  }
}

void setZeroPose() {
  
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

void setOnePose() {

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

void setTwoPose() {

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

void setThreePose() {

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

void setFourPose() {

  setDefaultPose();

  // Move all thumb joints to min
  managedServos[SERVO_THUMB_TIP].moveToMaxPosition();
  managedServos[SERVO_THUMB_RIGHT].moveToMinPosition();
  managedServos[SERVO_THUMB_LEFT].moveToMaxPosition();
  managedServos[SERVO_THUMB_ROTATE].setServoPosition(120);

}

void wave()
{
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

void count() {
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

}


void processCommand(String cmd) {
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
  if (cmdType == "hb") {
    connectionTimeout.resetTimerValue();
    Serial.println("Heartbeat received");
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

      // Update the hand angles based on any changes or computations
      //updateHand();

      // See if there's any data for us
      //if (rxCharacteristic.written()) {
      //  Serial.print("BLELoop: ");
      //#  Serial.println(reinterpret_cast<const char*>(rxCharacteristic.value()));
      //}
      
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    setDefaultPose();
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
  int16_t unpackedAngles[DOF_COUNT];

  for (int i = 0; i < DOF_COUNT; i++) {
    float angle = (data[i] - 128)*360.0/256.0;
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

  // Now, these angles can be sent to our controllers to adjust the hand poses

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
