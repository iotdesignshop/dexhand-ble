#include <ArduinoBLE.h>
#include <UniversalTimer.h>

BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART service

// UART characteristics
BLECharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 32);
BLECharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite, 32);

// Heartbeat timer
uint32_t heartbeat = 0;
UniversalTimer heartbeatTimer(1000, true);  // 1 second interval

void setup() {
  Serial.begin(9600);    // initialize serial communication
  
  if (!BLE.begin()) {   // initialize BLE
    Serial.println("starting BLE failed!");
    while (1);
  }

  heartbeatTimer.start();  // Start heartbeat timer

  BLE.setLocalName("DexHand");  // Set name for connection
  BLE.setAdvertisedService(uartService); // Add the service UUID
  
  uartService.addCharacteristic(rxCharacteristic); // Add the rxCharacteristic
  uartService.addCharacteristic(txCharacteristic); // Add the txCharacteristic

  BLE.addService(uartService); // Add the service
  rxCharacteristic.setEventHandler(BLEWritten, rxHandler);  // Assign event handler for characteristic
  
  BLE.advertise();  // Start advertising
  Serial.println("Bluetooth device active, waiting for connections...");

}

void loop() {
  BLEDevice central = BLE.central();  // wait for a BLE central

  if (central) {  // if a central is connected to the peripheral:
    Serial.print("Connected to central: ");
    Serial.println(central.address());  // print the central's MAC address
    
    
    while (central.connected()) {  // while the central is still connected to peripheral:
       
      // Check if it's time to send a heartbeat
      if (heartbeatTimer.check()) {
        heartbeat++;
        String data = "HB:" + String(heartbeat);
        txCharacteristic.writeValue(data.c_str());
        Serial.println(data);
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void rxHandler(BLEDevice central, BLECharacteristic characteristic) {
  String receivedData = reinterpret_cast<const char*>(characteristic.value());  // get the data
  Serial.println(receivedData);  // print the data
}
