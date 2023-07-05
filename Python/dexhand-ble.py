# DexHand-BLE - Demo of BLE Interface to DexHand on Arduino RP2040 Connect Board

# Released under the MIT License (MIT). See LICENSE file for details.
# Trent Shumay - trent@iotdesignshop.com

import asyncio
import sys
import cv2

from bleak import BleakScanner

import mediapipe as mp
from mediapipe import solutions
from mediapipe.framework.formats import landmark_pb2
#from mediapipe.tasks import python
#from mediapipe.tasks.python import vision

import numpy as np

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

        ts = 0
        try: 
            while cap.isOpened():
                ret, frame = cap.read()
                if not ret:
                    break

                mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=frame)

                detection_result = landmarker.detect_for_video(mp_image,ts)
                ts += 20
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

                    
            
        
async def ble_communication():
        stop_event = asyncio.Event()

        def callback(device, advertisement_data):
            if (advertisement_data.local_name and advertisement_data.local_name.startswith("DexHand")):
                #print(f"Device {device.address} (name: {device.name}) found.")
                #print(f"Advertised data: {advertisement_data}")
                pass

        async with BleakScanner(callback) as scanner:
            try:
                await(stop_event.wait())
                print('Scanning complete')
            except asyncio.CancelledError:
                print('BLE communication task has been cancelled.')
                #await scanner.stop()
                

        
        # Wait for 10 seconds
        asyncio.run(asyncio.sleep(10.0))


async def main():
    
    # Create the tasks
    hand_tracking_task = asyncio.create_task(hand_tracking())
    ble_communication_task = asyncio.create_task(ble_communication())

    # Run the tasks concurrently
    
    await asyncio.gather(hand_tracking_task, ble_communication_task, return_exceptions=True)

asyncio.run(main())