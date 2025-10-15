import asyncio
from bleak import BleakClient, BleakScanner
from pynput import keyboard
import time

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

# Motor/weapon values
PWM_MAX = 255
PWM_MIN = 0
WEAPON_ON = 255 #1500
WEAPON_OFF = 0

current_keys = set()

def encode_combo():
    # Map key combos to 0-15
    w = keyboard.KeyCode(char='w') in current_keys
    a = keyboard.KeyCode(char='a') in current_keys
    s = keyboard.KeyCode(char='s') in current_keys
    d = keyboard.KeyCode(char='d') in current_keys
    space = keyboard.Key.space in current_keys
    boost = keyboard.KeyCode(char='j') in current_keys

    # 4 bits for WASD, 1 bit for space
    combo = (w << 3) | (a << 2) | (s << 1) | (d << 0)
    if space:
        combo |= 0b10000

    # Map to 0-15 for each unique combo
    # combos without space: 0-7, with space: 8-15
    # But you want 16 unique combos, so:
    # 0: w
    # 1: a
    # 2: s
    # 3: d
    # 4: wa
    # 5: wd
    # 6: sa
    # 7: sd
    # 8: w+space
    # 9: a+space
    # 10: s+space
    # 11: d+space
    # 12: wa+space
    # 13: wd+space
    # 14: sa+space
    # 15: sd+space

    # Count movement keys pressed
    movement_keys = [w, a, s, d]
    num_pressed = sum(movement_keys)

    # Mapping:
    # 0: no movement, no space
    # 1: w
    # 2: a
    # 3: s
    # 4: d
    # 5: wa
    # 6: wd
    # 7: sa
    # 8: sd
    # 9: space only
    # 10: w+space
    # 11: a+space
    # 12: s+space
    # 13: d+space
    # 14: wa+space
    # 15: wd+space
    # 16: sa+space
    # 17: sd+space

    # Mapping:
    # 0: no keys, no space, no boost
    # 1: no keys, space, no boost
    # 2-9: movement (single/double), no space, no boost
    # 10-17: movement (single/double), space, no boost
    # 18: no keys, no space, boost
    # 19: no keys, space, boost
    # 20-27: movement (single/double), no space, boost
    # 28-35: movement (single/double), space, boost

    base = 0
    # If no movement keys, treat boost as not pressed
    if num_pressed == 0:
        return 1 if space else 0
    # If more than 2 movement keys, treat as no movement
    if num_pressed > 2:
        return 1 if space else 0
    # Single or double movement keys
    combos = [
        (w and not a and not s and not d),           # w
        (a and not w and not s and not d),           # a
        (s and not w and not a and not d),           # s
        (d and not w and not a and not s),           # d
        (w and a and not s and not d),               # wa
        (w and d and not a and not s),               # wd
        (s and a and not w and not d),               # sa
        (s and d and not w and not a),               # sd
    ]
    idx = None
    for i, c in enumerate(combos):
        if c:
            idx = i + 2  # start at 2
            break
    if idx is None:
        idx = 0  # fallback to no movement
    if space and idx != 0:
        idx += 8  # offset for space combos
    # If boost is pressed, add 16 to movement combos only
    if boost and idx != 0:
        idx += 16
    return idx

def on_press(key):
    current_keys.add(key)
    print(f"Keys pressed: {[str(k) for k in current_keys]}")

def on_release(key):
    current_keys.discard(key)

async def main():
    print("Searching for HELLCOPTER-BLE...")
    devices = await BleakScanner.discover()
    target = None
    for d in devices:
        if d.name == "Hellcopter":
            target = d
            break
    if not target:
        print("Device not found!")
        return

    async with BleakClient(target) as client:
        print("Connected! Use WASD for drive, SPACE for weapon. Press ESC to quit.")
        listener = keyboard.Listener(on_press=on_press, on_release=on_release)
        listener.start()
        try:
            while True:
                if keyboard.Key.esc in current_keys:
                    print("Exiting...")
                    break
                combo_num = encode_combo()
                packet = bytes([combo_num])
                await client.write_gatt_char(CHARACTERISTIC_UUID, packet, response=False)
                print(f"Combo: {combo_num}, Packet: {list(packet)}")
                time.sleep(0.05)  # 20Hz update
        finally:
            listener.stop()
            # Debugging output
            print(f"Packet sent: {packet.hex()}")
            # Uncomment the next line to see the raw packet sent
            print(f"Raw packet: {list(packet)}")

if __name__ == "__main__":
    asyncio.run(main())
