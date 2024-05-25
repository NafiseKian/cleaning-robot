import cv2
import os
import torch
import numpy as np
from models.experimental import attempt_load
from utils.general import non_max_suppression

def detect_trash(image_dir, weights_path, classNames):
    trash_detected = False

    # Load the YOLOv7 model with the specified weights file
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    model = attempt_load(weights_path, map_location=device)  # Load the model
    model.eval()

    # Initialize a list to store detection results for all images
    all_results = []

    # Recursively process each image in the directory and its subdirectories
    for root, dirs, files in os.walk(image_dir):
        for filename in files:
            if filename.lower().endswith(('.png', '.jpg', '.jpeg')):
                # Read the image
                img_path = os.path.join(root, filename)
                img = cv2.imread(img_path)

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
                            is_trash = classNames[int(cls)] == "trash"

                            # Append result to the list
                            all_results.append((filename, is_trash))
                            if is_trash:
                                trash_detected = True

    return trash_detected, all_results

# List of class names, assuming two classes: "not trash" and "trash"
classNames = ["not trash", "trash"]

# Call the function with your desired parameters
image_dir = r"/home/ciuteam/cleaningrobot/cleaning-robot/images"
weights_path = r"/home/ciuteam/cleaningrobot/cleaning-robot/epoch_054.pt"
trash_detected, detection_results = detect_trash(image_dir, weights_path, classNames)

print("Trash Detected:", trash_detected)
print("Detection Results:", detection_results)
