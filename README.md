# dexhand-ble
Example of Bluetooth Low Energy Connection/Control to a DexHand Powered by an Arduino RP2040 Connect Board

# Attribution
The original DexHand project and mechanical designs were created by The Robot Studio and released in the V1.0-Dexhand project on GitHub (https://github.com/TheRobotStudio/V1.0-Dexhand). This project draws upon that mechanical design, adding software and firmware function to the original work in addition to some optimized geometry which may make it easier to 3D print and assemble some of the mechanical components. The original project was released under the Creative Commons "Attribution-NonCommercial-ShareAlike 4.0 International" License (or CC BY-NC-SA 4.0) and as such this project is released with the same license to comply with those terms.

We would like to thank The Robot Studio for releasing such an interesting and inspiring design to Open Source and we are happy to support that effort with this repo which will hopefully augment the original project in useful ways with software, firmware, and some additional assembly instructions.

# Building a V1.0 DexHand 

Assembly of a V1 DexHand is a somewhat involved process which is both difficult and incredibly rewarding. We have a GitHub Pages Site that provides an overview of the process, but you should expect that you will need to be an intermediate level maker - familiar with 3D printing, electronics, and assembly processes in order to pull off a successful build. 

You can visit the assembly site here (TBD - Link)

# Software Project Overview
This project consists of two subfolders - one for the Arduino Firmware to install on an Arduino RP2040 Connect board to control the servos inside the DexHand and a Python demo script that uses Google MediaPipe Hand Tracker to generate poses for the hand based on a webcam feed, sending those poses over a Bluetooth LE connection to the Arduino board

## Requirements 

### Demo Application: Python Running on a PC/Mac/Linux
The Python scripts should run reasonably well on a properly configured Linux Machine, PC, or Mac if you follow the instructions carefully below. 

The demonstration script uses the Google Mediapipe Hand Tracker to perform software-based skeletal tracking of the user's hand motion in a 2D Webcam on the host computer. This doesn't require any special hardware, other than a computer with a Webcam but a GPU may improve performance of this tracker.

### Firmware: Arduino on Nano RP2040 Connect Board
The hardware build is based on the Arduino Nano RP2040 Connect board. This is an Arduino board based on the Raspberry Pi Pico MCU with additional wireless functions provided by a UBlox module. You can see full specs for this board on the Arduino website (https://docs.arduino.cc/hardware/nano-rp2040-connect). You can purchase the board direct from Arduino or from distributors such as DigiKey. Although this is not the cheapest Pi Pico board on the market, we felt that the security of the Arduino ecosystem and an officially supported board was well worth it for anyone building the hand. We may port the firmware to other boards in the future as well, and encourage the community to do so as well and contribute the results back to the project. 




# How to Set Up and Build the Arduino Firmware

## Ardunio IDE Set-Up
To compile the firmware, you will require Arduino IDE 2.x. Configuration of the Arduino environment and how to upload code to a board, etc is well covered in other tutorials, so we'll just list the basics here and the assumption is you can compile and flash the RP2040 Connect Board. The Getting Started Guide for the Nano RP2040 Connect is available here: https://docs.arduino.cc/software/ide-v1/tutorials/getting-started/cores/arduino-mbed_nano

## Library Dependencies
The following libraries must be installed via the Arduino Library Manager in the IDE:

* Arduino BLE (v1.3.4 was used at time of writing)
* Universal Timer (v1.0.0 was used at time of writing)
* RP2040 ISR Servo (v1.1.2 was used at time of writing)

If you are not familiar with the process of installing libraries, you may wish to reference this guide: https://support.arduino.cc/hc/en-us/articles/5145457742236-Add-libraries-to-Arduino-IDE

## Compile and Upload
Compile and flash your board with the Arduino project found in the ```dexhand-ble/Arduino/DexHand-RP2040-BLE``` folder. Once you have it running, you can launch the Python demo to connect to the Arduino via Bluetooth LE.


# How to Set Up and Run the Python Demo

![Demo_Python](https://github.com/iotdesignshop/dexhand-ble/assets/2821763/a217bcc1-1ed0-468a-91ad-5ab948d457ff)



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



