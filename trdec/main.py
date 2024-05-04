import cv2
import os

path = 'cascade.xml'
objectName = 'TRASH'
color = (255, 0, 255)
font = cv2.FONT_HERSHEY_SIMPLEX
fontScale = 1
fontColor = (0, 0, 255)
lineType = 2

def empty(a):
    pass

cv2.namedWindow("Result")

cascade = cv2.CascadeClassifier(path)

def detect_trash(image):
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    scaleVal = 1.2  # Adjust as needed
    neig = 5  # Adjust as needed
    objects = cascade.detectMultiScale(gray, scaleVal, neig)

    if len(objects) == 0:
        return "No trash detected"
    
    for (x, y, w, h) in objects:
        area = w * h
        minArea = 100  # Adjust as needed
        if area > minArea:
            cv2.rectangle(image, (x, y), (x + w, y + h), color, 3)
            cv2.putText(image, objectName, (x, y - 5), cv2.FONT_HERSHEY_COMPLEX_SMALL, 1, color, 2)
        
    return "Trash detected"

# Example usage:
image_dir = r'C:\Users\mosim\OneDrive\Desktop\images to detect'  # Provide the absolute path to your input image directory
image_files = os.listdir(image_dir)

for filename in image_files:
    img = cv2.imread(os.path.join(image_dir, filename))
    if img is None:
        print(f"Error: Image '{filename}' not found or cannot be read.")
    else:
        result = detect_trash(img)
        cv2.putText(img, result, (50, 50), font, fontScale, fontColor, lineType)
        cv2.imshow("Result", img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
