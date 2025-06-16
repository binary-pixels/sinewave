import cv2

# Load an RGB image
image = cv2.imread("1.jpg")  # Replace with your image path

# Convert to grayscale
gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

# Save or display the grayscale image
cv2.imwrite("4.jpeg", gray_image)  # Save the output
cv2.imshow("Grayscale Image", gray_image)  # Show the image
cv2.waitKey(0)
cv2.destroyAllWindows()

