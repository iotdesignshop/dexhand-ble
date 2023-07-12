# DexHand-BLE - Demo of BLE Interface to DexHand on Arduino RP2040 Connect Board

# Released under the MIT License (MIT). See LICENSE file for details.
# Trent Shumay - trent@iotdesignshop.com

import asyncio
import sys
import cv2
import time

from bleak import BleakScanner

import mediapipe as mp
from mediapipe import solutions
from mediapipe.framework.formats import landmark_pb2

from itertools import count, takewhile
from typing import Iterator

import numpy as np

from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData



MARGIN = 10  # pixels
FONT_SIZE = 1
FONT_THICKNESS = 1
HANDEDNESS_TEXT_COLOR = (88, 205, 54) # vibrant green

def draw_landmarks_on_image(rgb_image, detection_result):
  hand_landmarks_list = detection_result.hand_landmarks
  handedness_list = detection_result.handedness
  annotated_image = np.copy(rgb_image)

  # Loop through the detected hands to visualize.
  for idx in range(len(hand_landmarks_list)):
    hand_landmarks = hand_landmarks_list[idx]
    handedness = handedness_list[idx]

    # Draw the hand landmarks.
    hand_landmarks_proto = landmark_pb2.NormalizedLandmarkList()
    hand_landmarks_proto.landmark.extend([
      landmark_pb2.NormalizedLandmark(x=landmark.x, y=landmark.y, z=landmark.z) for landmark in hand_landmarks
    ])
    solutions.drawing_utils.draw_landmarks(
      annotated_image,
      hand_landmarks_proto,
      solutions.hands.HAND_CONNECTIONS,
      solutions.drawing_styles.get_default_hand_landmarks_style(),
      solutions.drawing_styles.get_default_hand_connections_style())

    # Get the top left corner of the detected hand's bounding box.
    height, width, _ = annotated_image.shape
    x_coordinates = [landmark.x for landmark in hand_landmarks]
    y_coordinates = [landmark.y for landmark in hand_landmarks]
    text_x = int(min(x_coordinates) * width)
    text_y = int(min(y_coordinates) * height) - MARGIN

    # Draw handedness (left or right hand) on the image.
    cv2.putText(annotated_image, f"{handedness[0].category_name}",
                (text_x, text_y), cv2.FONT_HERSHEY_DUPLEX,
                FONT_SIZE, HANDEDNESS_TEXT_COLOR, FONT_THICKNESS, cv2.LINE_AA)

  return annotated_image

def millis() -> int:
    """Get the current time in milliseconds."""
    return int(round(time.time() * 1000))

# Main hand tracking function
async def hand_tracking():
    BaseOptions = mp.tasks.BaseOptions
    HandLandmarker = mp.tasks.vision.HandLandmarker
    HandLandmarkerOptions = mp.tasks.vision.HandLandmarkerOptions
    VisionRunningMode = mp.tasks.vision.RunningMode


    # Create a hand landmarker instance with the video mode:
    options = HandLandmarkerOptions(
        base_options=BaseOptions(model_asset_path='models/hand_landmarker.task'),
        running_mode=VisionRunningMode.VIDEO)
    with HandLandmarker.create_from_options(options) as landmarker:

        cap = cv2.VideoCapture(0)

        # Get elapsed millis
        ts = millis()
        
        try: 
            while cap.isOpened():
                ret, frame = cap.read()
                if not ret:
                    break

                # Mirror the image because we're facing the webcam
                frame = cv2.flip(frame, 1)

                mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame)

                detection_result = landmarker.detect_for_video(mp_image,millis()-ts)
                annotated_image = draw_landmarks_on_image(frame, detection_result)
                cv2.imshow('DexHand BLE', annotated_image)

                # Yield
                await asyncio.sleep(0.01)

                # Check for escape key
                if cv2.waitKey(5) & 0xFF == 27:
                    raise Exception('User pressed ESC')
        
        except asyncio.CancelledError:
            print('Hand tracking task has been cancelled.')
            # Clean Up
            cap.release()
            cv2.destroyAllWindows()



UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

       
async def ble_communication():
    """This is a simple "terminal" program that uses the Nordic Semiconductor
    (nRF) UART service. It reads from stdin and sends each line of data to the
    remote device. Any data received from the device is printed to stdout.
    """

    def dexhand_devices(device: BLEDevice, adv: AdvertisementData):
        if (adv.local_name is not None) and ("DexHand" in adv.local_name):
            return True
        return False

    device = await BleakScanner.find_device_by_filter(dexhand_devices)

    if device is None:
        print("No DexHand device was found to establish a connection.")
        sys.exit(1)

    def handle_disconnect(_: BleakClient):
        print("Device was disconnected, goodbye.")
        # cancelling all tasks effectively ends the program
        for task in asyncio.all_tasks():
            task.cancel()

    def handle_rx(_: BleakGATTCharacteristic, data: bytearray):
        # Convert received byte array to string
        data = data.decode("utf-8")

        # Responses are pretty basic - this can be extended later if more
        # data comes back from the hand
        if (data.startswith("HB:")):
            print("Heartbeat received: ", data[3:])
        else:
            print("Unknown message received:", data)

    async with BleakClient(device, disconnected_callback=handle_disconnect) as client:
        await client.start_notify(UART_TX_CHAR_UUID, handle_rx)

        print("Connected, start typing and press ENTER...")

        loop = asyncio.get_running_loop()
        nus = client.services.get_service(UART_SERVICE_UUID)
        rx_char = nus.get_characteristic(UART_RX_CHAR_UUID)

        # Schedule a timer to send a heartbeat message to the hand every 5 seconds
        async def send_heartbeat():
            hb = 0
            
            while True:
                await asyncio.sleep(5)
                hb_msg = "HB:"+str(hb)+"\n"
                hb += 1
                await client.write_gatt_char(rx_char, hb_msg.encode(), True)

        asyncio.create_task(send_heartbeat())

        while True:
            # This waits until you type a line and press ENTER.
            # A real terminal program might put stdin in raw mode so that things
            # like CTRL+C get passed to the remote device.
            data = await loop.run_in_executor(None, sys.stdin.buffer.readline)

            # data will be empty on EOF (e.g. CTRL+D on *nix)
            if not data:
                break

            # Send the data via BLE
            await client.write_gatt_char(rx_char, data, True)
            print("Sent:", data)


async def main():
    
    # Create the tasks
    hand_tracking_task = asyncio.create_task(hand_tracking())
    ble_communication_task = asyncio.create_task(ble_communication())

    # Run the tasks concurrently
    
    await asyncio.gather(hand_tracking_task, ble_communication_task, return_exceptions=True)

asyncio.run(main())