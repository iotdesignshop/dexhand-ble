# ble-joint-streamer.py
#
# Sample application to demonstrate joint angle streaming to DexHand via BLE.
#
# Trent Shumay (trent@iotdesignshop.com)

import asyncio

from bleak import BleakScanner

import numpy as np

from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

import json


# Constants and controls - see the README.md file for details
JOINT_DEADBAND = 0  # Number of degrees to ignore for joint movement to help settle noise from MediaPipe

# Connection flag - set to True when connected to hand
hand_connected = False

# DOF table - ultimately this will be retrieved from the hand, but for now we're just using the string you can request from the firmware
dof_table = json.loads('[ { "name": "index_pitch", "range": [0, 40] }, { "name": "index_yaw", "range": [-20, 20] }, { "name": "index_flexion", "range": [0, 100] }, { "name": "middle_pitch", "range": [0, 40] }, { "name": "middle_yaw", "range": [-20, 20] }, { "name": "middle_flexion", "range": [0, 100] }, { "name": "ring_pitch", "range": [0, 40] }, { "name": "ring_yaw", "range": [-20, 20] }, { "name": "ring_flexion", "range": [0, 100] }, { "name": "pinky_pitch", "range": [0, 40] }, { "name": "pinky_yaw", "range": [-20, 20] }, { "name": "pinky_flexion", "range": [0, 100] }, { "name": "thumb_pitch", "range": [30, 60] }, { "name": "thumb_yaw", "range": [0, 45] }, { "name": "thumb_flexion", "range": [0, 45] }, { "name": "wrist_pitch", "range": [-40, 40] }, { "name": "wrist_yaw", "range": [-40, 40] } ]')
     
# User function - this is where you can do things with the joint angles
# Don't forget to yield periodically to allow the bluetooth communication to run
async def update_angles(tx_queue):

    while (not hand_connected):
        print("Waiting for hand to connect...")
        await asyncio.sleep(1)
    
    try:

        # Default angles
        joint_angles = np.zeros(len(dof_table))

        # Loop through all DOFs and test their range
        for dof in range(len(dof_table)):
            # Move to negative range
            joint_angles[dof] = dof_table[dof]["range"][0]
            await tx_queue.put(joint_angles)
            print(joint_angles)
            await asyncio.sleep(0.5)
            
            # Move to positive range
            joint_angles[dof] = dof_table[dof]["range"][1]
            await tx_queue.put(joint_angles)
            print(joint_angles)
            await asyncio.sleep(0.5)

            # Move back to zero
            joint_angles[dof] = 0
            await tx_queue.put(joint_angles)
            print(joint_angles)
            await asyncio.sleep(0.5)



       

    except asyncio.CancelledError:
            print('Task has been cancelled.')
            


# Services and characteristics we use for communication with the hand
UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

DOF_SERVICE_UUID = "1e16c1b4-1936-4f0e-ab62-5e0a702a4935"
DOF_CHAR_UUID = "1e16c1b5-1936-4f0e-ab62-5e0a702a4935"

# Main BLE communication task       
async def ble_communication(tx_queue):
    """ This task handles communication with the DexHand over BLE."""

    def dexhand_devices(device: BLEDevice, adv: AdvertisementData):
        if (adv.local_name is not None) and ("DexHand" in adv.local_name):
            return True
        return False

    try:
        # Scan for the DexHand device
        deviceFound = False
        while (not deviceFound):
            print("Scanning for DexHand devices...")

            device = await BleakScanner.find_device_by_filter(dexhand_devices, timeout=10.0)

            if device is None:
                print("No DexHand device was found. Scanning again...")
            else:
                print("Found DexHand device:", device.name)
                deviceFound = True
    except asyncio.CancelledError:
            print('Communication task has been cancelled.')
            return
    
    def handle_disconnect(_: BleakClient):
        print("Device was disconnected, goodbye.")
        

    def handle_rx(_: BleakGATTCharacteristic, data: bytearray):
        # Convert received byte array to string
        data = data.decode("utf-8")

        # Responses are pretty basic - this can be extended later if more
        # data comes back from the hand
        if (data.startswith("HB:")):
            print("Heartbeat received from hand: ", data[3:])
        else:
            print("Unknown message from hand received:", data)

    def update_values(previous_values, current_values, threshold):
        # Update the values only if they have changed by more than the threshold
        mask = np.abs(current_values - previous_values) > threshold
        updated_values = previous_values.copy()  # Create a copy to store the updated values
        updated_values[mask] = current_values[mask]
        return updated_values

    async with BleakClient(device, disconnected_callback=handle_disconnect) as client:
        await client.start_notify(UART_TX_CHAR_UUID, handle_rx)

        global hand_connected
        hand_connected = True

        print("Connected to DexHand")

        # Nordic UART Service (NUS) is used to send commands to the hand
        nus = client.services.get_service(UART_SERVICE_UUID)
        rx_char = nus.get_characteristic(UART_RX_CHAR_UUID)

        # Custom DOF service is used to send joint angles to the hand
        dof_service = client.services.get_service(DOF_SERVICE_UUID)
        dof_char = dof_service.get_characteristic(DOF_CHAR_UUID)

        # Schedule a timer to send a heartbeat message to the hand every 3 seconds via UART. 
        # The Arduino firmware auto-disconnects if it doesn't receive a periodic heartbeat
        # in order to prevent it from getting hung up on a connection that is left open.
        async def send_heartbeat():
            hb = 0
            
            while True:
                await asyncio.sleep(3)
                hb_msg = "HB:"+str(hb)+"\n"
                hb += 1
                await client.write_gatt_char(rx_char, hb_msg.encode(), True)

        asyncio.create_task(send_heartbeat())

        # Send a request to the hand to send the DOF table
        print("Requesting DOF table from hand...")
        await client.write_gatt_char(rx_char, "dofs\n".encode(), True)
        
        try:
            previous_angles = await tx_queue.get()

            
            while True:

                # Throw away any surplus queued joint angles - we only want to transmit the latest.
                # We're currently keeping them in a queue in case we want to do something else with them
                # in the future, but for now we just want the latest.
                while (tx_queue.qsize() > 1):
                    tx_queue.get_nowait()

                # Grab the latest set of angles
                new_angles = await tx_queue.get()

                # Threshold the values to reduce noise when hand is at rest
                # Movement of more than specified deadband is required to move the joints
                joint_angles = update_values(previous_angles, new_angles, JOINT_DEADBAND)
                previous_angles = joint_angles
                

                # Encode the joint angles into 8-bit integers. We scale everything to 0-255
                # to represent -180 to 180 degrees with 128 as the zero point.
                clipped = np.clip(joint_angles, -180, 180)
                scaled = np.interp(clipped, (-180, 180), (0, 255))
                encoded = scaled.astype(np.uint8)

                data = bytearray(encoded)

                # Add a checksum to the data
                checksum = sum(data) % 256
                data.append(checksum)
                
                # Send the joint angles to the hand without response as it's faster
                await client.write_gatt_char(dof_char, data)

                # Yield
                await asyncio.sleep(0)
        
        except asyncio.CancelledError:
            print('Communication task has been cancelled.')

            hand_connected = False
             
            # Disconnect from the hand
            if (client.is_connected):
                await client.disconnect()

            


async def main():

    # Create the transmit queue
    ble_tx_queue = asyncio.Queue()
    
    # Create the tasks
    hand_tracking_task = asyncio.create_task(update_angles(ble_tx_queue))
    ble_communication_task = asyncio.create_task(ble_communication(ble_tx_queue))

    # Run the tasks concurrently
    try:
        await asyncio.gather(hand_tracking_task, ble_communication_task, return_exceptions=True)
    except asyncio.exceptions.CancelledError:
        print('Sub tasks have been cancelled. Shutting down.')

asyncio.run(main())