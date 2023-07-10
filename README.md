# dexhand-ble
Example of Bluetooth Low Energy Connection/Control to a DexHand Powered by an Arduino RP2040 Connect Board

# Overview
This project consists of two subfolders - one for the Arduino Firmware to install on an Arduino RP2040 Connect board to control the servos inside the DexHand and a Python demo script that uses Google MediaPipe Hand Tracker to generate poses for the hand based on a webcam feed, sending those poses over a Bluetooth LE connection to the Arduino board

## Requirements
The Python scripts should run reasonably well on a properly configured Linux Machine, PC, or Mac if you follow the instructions carefully below. Your framerate will vary based on the computational power of your host machine.

To compile the firmware, you will require Arduino IDE 2.x. Configuration of the Arduino environment and how to upload code to a board, etc is well covered in other tutorials, so we'll just list the basics here and the assumption is you can compile and flash the RP2040 Connect Board.


# Arduino Setup

## Library Dependencies
The following libraries must be installed via the Arduino Library Manager in the IDE:

* Arduino BLE (v1.3.4 was used at time of writing)
* Universal Timer (v1.0.0 was used at time of writing)
* RP2040 ISR Servo (v1.1.2 was used at time of writing)

## Bluetooth LE
For simplicity, the device is set up to advertise the Nordic BLE UART Service (and to use this service for communications with the device). By default it is named "DexHand" as well, so you should be able to see the device with any BLE Scanner after you've successfully built and uploaded the Arduino firmware.

