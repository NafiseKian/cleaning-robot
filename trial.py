import cv2
import os
import torch
import numpy as np
import socket
from models.experimental import attempt_load
from utils.general import non_max_suppression

SOCKET_PATH = "/tmp/unix_socket_example"

def detect_trash(image_path, model, classNames, device):
    trash_detected = False

    # Load the image
    print(f"Loading image: {image_path}")
    img = cv2.imread(image_path)
    if img is None:
        print(f"Error: Unable to load image at {image_path}")
        return False

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

    # Apply NMS
    pred = non_max_suppression(pred, 0.25, 0.45, agnostic=False)

    # Process detections
    for det in pred:
        if len(det):
            # Process each detected object
            for *xyxy, _, cls in det:
                # Determine if it's trash or not
                if classNames[int(cls)] == "trash":
                    trash_detected = True
                    break

    return trash_detected

def main():
    # Load the YOLOv7 model with the specified weights file
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    weights_path = r"/home/ciuteam/cleaningrobot/cleaning-robot/epoch_054.pt"
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

            # Run the detection
            trash_detected = detect_trash(image_path, model, classNames, device)

            # Send the result back to C++ process
            result = "1" if trash_detected else "0"
            print("the value of result is "+result)
            connection.sendall(result.encode('utf-8'))

    finally:
        # Clean up the connection
        connection.close()
        server_socket.close()
        os.unlink(SOCKET_PATH)

if __name__ == "__main__":
    main()
