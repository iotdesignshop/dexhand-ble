# dexhand-ble

Arduino-based firmware, and a Python-based hand tracking demo to demonstrate the V1.0 DexHand - a low-cost, open-source, 3D printed humanoid robot hand.

*Note: This project covers the software and firmware for basic control of a V1.0 DexHand. If you'd like to learn more about how to build your own hand, you can visit our page explaining this process at (https://www.dexhand.org). It has a lot more information on the hardware and physical build of this low-cost, open source dexterous hand.*


# Attribution
The original DexHand project and mechanical designs were created by [The Robot Studio](http://www.therobotstudio.com) and released in the [V1.0-Dexhand project on GitHub](https://github.com/TheRobotStudio/V1.0-Dexhand). This project draws upon that mechanical design, adding software and firmware function to the original work. The original project was released under the Creative Commons "Attribution-NonCommercial-ShareAlike 4.0 International" License (or CC BY-NC-SA 4.0) and as such this project is released with the same license to comply with those terms.

We would like to thank The Robot Studio for releasing such an interesting and inspiring design to Open Source and we are happy to support that effort with this repo which will hopefully augment the original project in useful ways with software, firmware, and some additional assembly instructions.

# Software Project Overview

## Project Contents 
This project consists of two subfolders:
- **Arduino** Contains Arduino-based Firmware to install on an [Arduino Nano RP2040 Connect](https://docs.arduino.cc/hardware/nano-rp2040-connect) board to control the servos inside the DexHand, and to provide a Bluetooth Low Energy (BLE) connection to the hand controls for wirelessly streaming data to the hand.
- **Python** Contains a Python demo script that uses [Google MediaPipe Hand Tracker](https://developers.google.com/mediapipe/solutions/vision/hand_landmarker) to generate poses for the hand based on a webcam feed, sending those poses over the BLE connection to the Arduino board

## Demo Software Requirements 

### Python Demo Application: Python Running on a PC/Mac/Linux
<img src="https://github.com/iotdesignshop/dexhand-ble/assets/2821763/8220275d-d958-4193-950f-b063d92b0877" width=300px>

The Python scripts should run reasonably well on a properly configured Linux Machine, PC, or Mac if you follow the instructions carefully below. 

The demonstration script uses the Google Mediapipe Hand Tracker to perform software-based skeletal tracking of the user's hand motion in a 2D Webcam on the host computer. This doesn't require any special hardware, other than a computer with a Webcam but a GPU may improve performance of this tracker.

### Firmware: Arduino on Nano RP2040 Connect Board
<img width="300px" alt="Arduino-Community" src="https://github.com/iotdesignshop/dexhand-ble/assets/2821763/9ecde78a-03c9-43b5-a738-ea8ba403c2b7">

The hardware build is based on the Arduino Nano RP2040 Connect board. This is an Arduino board based on the Raspberry Pi Pico MCU with additional wireless functions provided by a UBlox module. You can see full specs for this board on the Arduino website (https://docs.arduino.cc/hardware/nano-rp2040-connect). You can purchase the board direct from Arduino or from distributors such as DigiKey. 

Wiring up the board to the servos in the DexHand is a somewhat complex task, and is covered on the main DexHand page at (https://www.dexhand.org). 



# Arduino Firmware Set Up and Compilation
## Ardunio IDE Set-Up
To compile the firmware, you will require Arduino IDE 2.x. Configuration of the Arduino environment and how to upload code to a board, etc is well covered in other tutorials, so we'll just list the basics here and the assumption is you can compile and flash the RP2040 Connect Board. The Getting Started Guide for the Nano RP2040 Connect is available here: https://docs.arduino.cc/software/ide-v1/tutorials/getting-started/cores/arduino-mbed_nano

## Library Dependencies
The following libraries must be installed via the Arduino Library Manager in the IDE:

* Arduino BLE (v1.3.4 was used at time of writing)
* Universal Timer (v1.0.0 was used at time of writing)
* RP2040 ISR Servo (v1.1.2 was used at time of writing)

If you are not familiar with the process of installing libraries, you may wish to reference this guide: https://support.arduino.cc/hc/en-us/articles/5145457742236-Add-libraries-to-Arduino-IDE

## Compile and Upload
Compile and flash your board with the Arduino project found in the ```dexhand-ble/Arduino/DexHand-RP2040-BLE``` folder. 


# Arduino Firmware Usage 

## Modes of Operation
The firmware has two distinct modes of operation:

- **Direct Control** In Direct Control mode, you can issue text commands to the DexHand via the Arduino IDE or any other tool that supports serial connections to the Arduino via the host USB connection. Details on the supported command set are provided below. Or, feel free to come up with your own animations and states and add them to the hand.
- **Bluetooth Low Energy Streaming** In Bluetooth Streaming mode, angles specifying the position of the degrees of freedom in the hand are streamed across a Bluetooth LE connection to the hand. Additionally, you can send over the same command supported in Direct Control mode as well via a UART service provided by the firmware in tha hand.

## Direct Control Commands

The following commands are supported via USB serial, or via the Bluetooth LE UART Service (more on that below in the Bluetooth Streaming Section). The Direct Control Commands are the quickest way to get started and to test your DexHand, and the command set contains some functions that are useful for setting up and tuning the tendons in the hand, so start here first.

### Setting a Servo Position

```set:<servonum>:<angle>```

Sets the servo index provided in *servonum* to the specified *angle*. Note that angles are range-limited by the servo configuration table, so if you ask for an angle outside the range of a servo it may not move. The servo table and ranges are found near the top of the [DexHand-RP2040-BLE.ino](Arduino/DexHand-RP2040-BLE/DexHand-RP2040-BLE.ino) source file in the Arduino project. 

This function can be useful for experimenting with the range of motion of servos, or for trying to figure out hand poses for animations.

The servo indices are as follows:

```
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
```

### Moving a Servo to Min Range, Max Range (and Optionally Overriding Ranges for Testing)

```min:<servonum>(:<angle>)```
```max:<servonum>(:<angle>)```

The min and max commands are analagous. If you do not specify the *angle* parameter, they are used to move the servo to the current minimum or maximum end of it's range. For example:

```min:8``` would move servo index 8 (SERVO_INDEX_TIP) to the minimum value of it's range in the servo table.

```max:16``` would move servo index 16 (SERVO_WRIST_L) to the maximum value of it's range in the servo table.

During the tuning process, it may be helpful to override these ranges temporarily while testing the limits of motion of your hand. If you specify the *angle* parameter, the command will both set a new min or max for the servo, and move to that value:

```min:8:30``` would set the minumum range for servo index 8 (SERVO_INDEX_TIP) to 30 degrees, and then move the servo to that value.

```max:16:160``` would set the maximum range index 16 (SERVO_WRIST_L) to 160 degrees, and then move the servo to that value.


### Moving a Finger to Minimum or Maximum Extension

```fingermax:<fingernum>```
```fingermin:<fingernum>```

For testing, you can move an entire finger to it's minimum or maximum range. The finger indices are as follows:

```0 = Index, 1 = Middle, 2 = Ring, 3 = Pinky```

*Note: The thumb is not included in this function as it's motion is more complex than a finger, and those angles should be set directly*


### Wrist Pitch and Yaw Movements

```wrist:pitch:<angle>```
```wrist:yaw:<angle>```

Fairly self explanatory - used for testing the range of motion of the wrist joints. In the default firmware, the angular range is -20 to + 20 degrees on both the pitch and yaw of the wrist. 


### Returning to Default Pose

```default```

After a while, experimenting with different poses can get you into a bit of an unknown state. Calling the ```default``` function will return all the angles to their default initial values.


### Fun Animations - Counting, Waving, and Shaka

```count```
```wave```
```shaka```

These commands can be issued to get the hand to count to five, to wave at you, or to show you a shaka. 



# How to Set Up and Run the Python Demo

![Demo_AdobeExpress-2](https://github.com/iotdesignshop/dexhand-ble/assets/2821763/eac379b5-f14b-4bde-b8e9-66fad7c3517d)


The Python Demo should run on any PC, Mac, or Linux machine with Python 3.7 or higher. We recommend creating a virtual environment for running the script in order to keep dependencies in check.

The quick guide below will help you get a Python Virtual Environment set up. A full explanation of Python Virtual Environments is available here if you run into trouble: https://docs.python.org/3/library/venv.html

Change to the ```dexhand-ble/Python``` folder and follow the process below:

## Mac and Linux
```
$ python3 -m venv mp_env && source mp_env/bin/activate
```
Then, install the requirements.txt as follows:
```
(mp-env)$ pip install -r requirements.txt
```

## Windows
```
python -m venv mp_env
.\mp_env\Scripts\activate
```
Then, install the requirements.txt as follows:
```
(mp-env)> pip install -r requirements.txt
```



# Running the Demo

To launch the demo script:
```
(mp-env)$ python dexhand-ble.py
```

## What Do the Debug Numbers Mean?
<img width="800" alt="Debug-Numbers" src="https://github.com/iotdesignshop/dexhand-ble/assets/2821763/e61605aa-c3ab-4246-8bf1-f8a07437e5b0">

The numbers shown at the top left of the screen are the **DOF Angles** corresponding to the fingers and thumb. From top to bottom, they are the pitch, yaw, and long flexion tendon angle. These are the values that are streamed to the firmware via Bluetooth LE. It can help to understand what exactly your DexHand is doing if you can see those angles in real time, so we leave them up on the screen.

## How Does the Hand Pose Data Get Sent to the DexHand

When the demo boots up, it starts scanning for Bluetooth Low Energy devices that match the signature of the DexHand. Once connected, it streams the angles of the joints observed in the demo over to the DexHand as fast as possible (usually the connection is pretty quick and responsive, although BLE connections can vary depending on the hardware involved). 

This is done by packing all of the DOF angles (there are 17 in total) into bytes, and sending that data over as a BLE characteristic. Generally, BLE Maximum Transmission Unit (MTU) sizes are 23 bytes for a single packet. So, we compress our angles each into a single byte to make them small enough to send in a single transmission for speed. 

We do this by mapping the angular range from -180 to 180 degrees into 8-bits, meaning 0 = -180 degrees, 127 = 0 degrees, and 255 = 180 degrees.

Although this is slightly lossy, it's more than good enough for the fidelity of the demo, and allows us to use a very simple transmission method to get our data from the demo to the hand.

### DOF Service and Characteristic
If you want to connect to the hand using different software, the service details are as follows:
```
BLE DOF Service ID:        1e16c1b4-1936-4f0e-ab62-5e0a702a4935
BLE DOF Characteristic:    1e16c1b5-1936-4f0e-ab62-5e0a702a4935 (Write without response)
```

## UART Service and Command Stream

In addition to the DOF Service, you can also access a standard UART emulation service on the DexHand firmware. This allows you to send the same commands that you can send via USB serial to the device for debugging and testing. 

*Note: If you stream DOF angles while trying to use the UART commands, the stream will obliterate all the commands every time it updates. So, you can only do one or the other at the same time. Choose wisely.*

### UART Service and Characteristic
The UART Service uses the Nordic UART Service ID and Characteristics which is sort of the defacto standard for implementing UART via BLE. Many BLE apps will understand this service intrinsically. 
```
Nordic UART Service ID:    6E400001-B5A3-F393-E0A9-E50E24DCCA9E
TX Characteristic (from DexHand to App):  6E400003-B5A3-F393-E0A9-E50E24DCCA9E (Supports BLE Notify)
RX Characteristic (from App to Dexhand): 6E400002-B5A3-F393-E0A9-E50E24DCCA9E (BLE Write)
```


# Digging Deeper

For the most part, the math, and even some of the tricker bits of the hand control is straight forward and documented inside the Arduino and Python source code provided in the project. That's sort of your "first assignment" - go review that code and have a look at how things work. 

You will notice there actually is not very much code at all on the Python side! Most of the heavy lifting is done by Google Media Pipe, and the Bleak BLE Library. The Arduino firmware is slightly more involved, but largely because C/C++ is a verbose language with a lot of boilerplate code. The actual math and logic for the hand function is also quite similar. 

If you have questions or issues, [reach out via the project issues tracker](https://github.com/iotdesignshop/dexhand-ble/issues) and we will try to help.


