import argparse
import struct
import time

import cv2
import serial


FRAME_WIDTH = 128
FRAME_HEIGHT = 64
FRAME_SIZE = FRAME_WIDTH * FRAME_HEIGHT * 2
MAGIC = b"\xA5\x5A"
ACK = b"K"


def parse_args():
    parser = argparse.ArgumentParser(
        description="Capture PC camera frames and send 128x64 RGB565 color data to STM32 USART1."
    )
    parser.add_argument("--port", required=True, help="Serial port, for example COM3.")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate.")
    parser.add_argument("--camera", type=int, default=0, help="OpenCV camera index.")
    parser.add_argument("--fps", type=float, default=1.0, help="Max frames per second.")
    parser.add_argument("--preview", action="store_true", help="Show the 128x64 color preview window.")
    return parser.parse_args()


def pack_rgb565(frame_bgr):
    data = bytearray(FRAME_SIZE)
    index = 0

    for y in range(FRAME_HEIGHT):
        for x in range(FRAME_WIDTH):
            b, g, r = frame_bgr[y, x]
            color = ((int(r) & 0xF8) << 8) | ((int(g) & 0xFC) << 3) | (int(b) >> 3)
            data[index] = color >> 8
            data[index + 1] = color & 0xFF
            index += 2

    return bytes(data)


def crop_to_frame_aspect(image):
    height, width = image.shape[:2]
    target_aspect = FRAME_WIDTH / FRAME_HEIGHT
    current_aspect = width / height

    if current_aspect > target_aspect:
        new_width = int(height * target_aspect)
        left = (width - new_width) // 2
        return image[:, left:left + new_width]

    new_height = int(width / target_aspect)
    top = (height - new_height) // 2
    return image[top:top + new_height, :]


def frame_to_rgb565_data(frame):
    cropped = crop_to_frame_aspect(frame)
    resized = cv2.resize(cropped, (FRAME_WIDTH, FRAME_HEIGHT), interpolation=cv2.INTER_AREA)

    return pack_rgb565(resized), resized


def build_packet(payload):
    checksum = 0
    for byte in payload:
        checksum ^= byte

    return MAGIC + struct.pack("<H", len(payload)) + payload + bytes([checksum])


def wait_for_ack(ser, timeout):
    deadline = time.monotonic() + timeout
    ignored = bytearray()

    while time.monotonic() < deadline:
        data = ser.read(1)
        if not data:
            continue
        if data == ACK:
            if ignored:
                text = bytes(ignored).decode(errors="ignore").strip()
                if text:
                    print(f"STM32 says: {text}")
            return True
        ignored.extend(data)

    if ignored:
        text = bytes(ignored).decode(errors="ignore").strip()
        if text:
            print(f"Ignored serial text before ACK: {text}")
    return False


def drain_startup_text(ser, timeout):
    deadline = time.monotonic() + timeout
    data = bytearray()

    while time.monotonic() < deadline:
        chunk = ser.read(128)
        if chunk:
            data.extend(chunk)
            deadline = time.monotonic() + 0.2

    if data:
        text = bytes(data).decode(errors="ignore").strip()
        if text:
            print(f"STM32 startup: {text}")


def main():
    args = parse_args()
    frame_interval = 1.0 / args.fps if args.fps > 0 else 0
    timeout_count = 0

    cap = cv2.VideoCapture(args.camera, cv2.CAP_DSHOW)
    if not cap.isOpened():
        raise RuntimeError("Could not open camera. Try another --camera index.")

    with serial.Serial(args.port, args.baud, timeout=0.02, write_timeout=5) as ser:
        time.sleep(2.0)
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        drain_startup_text(ser, timeout=0.5)
        print(f"Sending camera {args.camera} to STM32 display on {args.port} at {args.baud} baud. Press Ctrl+C to stop.")

        try:
            while True:
                start = time.monotonic()
                ok, frame = cap.read()
                if not ok:
                    print("Camera frame read failed, retrying...")
                    time.sleep(0.2)
                    continue

                payload, preview = frame_to_rgb565_data(frame)
                ser.write(build_packet(payload))
                ser.flush()

                if wait_for_ack(ser, timeout=2.5):
                    timeout_count = 0
                else:
                    timeout_count += 1
                    if timeout_count % 5 == 0:
                        print("No ACK from STM32 yet. Check baud rate, wiring, and firmware.")

                if args.preview:
                    cv2.imshow("128x64 color preview", cv2.resize(preview, (512, 256), interpolation=cv2.INTER_NEAREST))
                    if cv2.waitKey(1) & 0xFF == 27:
                        break

                elapsed = time.monotonic() - start
                if elapsed < frame_interval:
                    time.sleep(frame_interval - elapsed)

        except KeyboardInterrupt:
            pass
        finally:
            cap.release()
            if args.preview:
                cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
