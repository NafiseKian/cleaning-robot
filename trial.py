import cv2
import os
import torch
import numpy as np
import socket
import math
import cvzone
from models.experimental import attempt_load
from utils.general import non_max_suppression, scale_coords

SOCKET_PATH = "/tmp/unix_socket_example"


def detect_trash(image_path, model, classNames, device, conf_thresh=0.5):
    trash_detected = False
    direction = "center"
    angle = 0

    # Load the image
    print(f"Loading image: {image_path}")
    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Unable to load image at {image_path}")
        return False, direction, angle

    img0 = img.copy()  # Copy for drawing

    # Prepare image for inference
    img = cv2.resize(img, (640, 640))
    img = img[:, :, ::-1].transpose(2, 0, 1)  # BGR to RGB, to 3x640x640
    img = np.ascontiguousarray(img)

    img = torch.from_numpy(img).to(device)
    img = img.float()  # uint8 to fp16/32
    img /= 255.0  # 0 - 255 to 0.0 - 1.0

    # Ensure img has the right dimensions
    if img.ndimension() == 3:
        img = img.unsqueeze(0)

    # Perform object detection
    with torch.no_grad():
        pred = model(img)[0]

    # Apply NMS with confidence threshold
    pred = non_max_suppression(pred, conf_thresh, 0.45, agnostic=False)

    # Process detections
    for det in pred:
        if len(det):
            det[:, :4] = scale_coords(img.shape[2:], det[:, :4], img0.shape).round()

            for *xyxy, _, cls in det:
                x1, y1, x2, y2 = map(int, xyxy)
                w, h = x2 - x1, y2 - y1

                # Determine if it's trash or not
                if classNames[int(cls)] == "trash":
                    trash_detected = True

                    # Draw bounding box
                    cvzone.cornerRect(img0, (x1, y1, w, h))

                    # Calculate the horizontal center of the bounding box
                    center_x = (x1 + x2) / 2
                    image_center_x = img0.shape[1] / 2

                    # Calculate the angle based on the position
                    angle = (center_x / img0.shape[1]) * 180

                    # Determine the direction based on the angle
                    if 0 <= angle <= 45:
                        direction = "turn left"
                    elif 135 <= angle <= 180:
                        direction = "turn right"
                    else:
                        direction = "center"

                    # Print direction and angle
                    print(f"Direction: {direction}, Angle: {angle}")
                    break

    return trash_detected, direction, angle

def main():
    # Load the YOLOv7 model with the specified weights file
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    weights_path = r"/home/ciuteam/cleaningrobot/cleaning-robot/trained.pt"
    model = attempt_load(weights_path, map_location=device)  # Load the model
    model.eval()

    classNames = ["not trash", "trash"]

    # Create a Unix domain socket
    server_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    # Ensure the socket does not already exist
    try:
        os.unlink(SOCKET_PATH)
    except OSError:
        if os.path.exists(SOCKET_PATH):
            raise

    # Bind the socket to the address
    server_socket.bind(SOCKET_PATH)

    # Listen for incoming connections
    server_socket.listen(1)

    print("Waiting for a connection...")
    connection, client_address = server_socket.accept()

    try:
        print("Connection established")

        while True:
            # Receive the data
            data = connection.recv(1024)
            if not data:
                break

            image_path = data.decode('utf-8')
            print(f"Received image path: {image_path}")

            # Run the detection with a confidence threshold of 0.5
            trash_detected, direction, angle = detect_trash(image_path, model, classNames, device, conf_thresh=0.5)

            # Send the result back to C++ process
            result = f"{int(trash_detected)}|{direction}|{angle}"
            print(f"Sending result: {result}")
            connection.sendall(result.encode('utf-8'))

    finally:
        # Clean up the connection
        connection.close()
        server_socket.close()
        os.unlink(SOCKET_PATH)

if _name_ == "_main_":
    main()
