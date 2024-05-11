import cv2
import os

# Define the path to the cascade.xml file
cascade_path = r'C:\Users\mosim\trdec\cascadee.xml'


objectName = 'TRASH'
color = (255, 0, 255)
font = cv2.FONT_HERSHEY_SIMPLEX
fontScale = 1
fontColor = (0, 0, 255)
lineType = 2

def empty(a):
    pass

cv2.namedWindow("Result")


# Add error handling to load the cascade classifier
try:
    cascade = cv2.CascadeClassifier(cascade_path)
except cv2.error as e:
    print("Error: Unable to load cascade classifier file.")
    print(e)
    exit()

def detect_trash(image):
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    scaleVal = 1.2  # Adjust as needed
    neig = 5  # Adjust as needed
    objects = cascade.detectMultiScale(gray, scaleVal, neig)

    if len(objects) == 0:
        return 0, "Not trash detected", image  # No trash detected, return original image
    
    for (x, y, w, h) in objects:
        area = w * h
        minArea = 100  # Adjust as needed
        if area > minArea:
            cv2.rectangle(image, (x, y), (x + w, y + h), color, 3)
            cv2.putText(image, objectName, (x, y - 5), cv2.FONT_HERSHEY_COMPLEX_SMALL, 1, color, 2)
            return 1, "Trash detected", image  # Trash detected, return original image
        
    return 0, "Not trash detected", image  # No trash detected, return original image

# Example usage:
image_dir = r'C:\Users\mosim\OneDrive\Desktop\images to detect'  # Provide the absolute path to your input image directory
image_files = os.listdir(image_dir)

for filename in image_files:
    img = cv2.imread(os.path.join(image_dir, filename))
    if img is None:
        print(f"Error: Image '{filename}' not found or cannot be read.")
    else:
        result, message, image_with_detection = detect_trash(img)
        print(f"{message}: {result}")
        cv2.imshow("Result", image_with_detection)
        cv2.waitKey(0)
        cv2.destroyAllWindows()