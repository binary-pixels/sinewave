import cv2

# Load the image
image = cv2.imread("1.bmp", 0)

blurred_image = image.copy()
# Apply Gaussian Blur
for i in range(14):
    blurred_image = cv2.GaussianBlur(blurred_image, (5, 5), 0)  # (kernel_size, sigmaX)

# Save or Display the result
cv2.imwrite("111.bmp", blurred_image)
cv2.imshow("Blurred Image", blurred_image)
cv2.waitKey(0)
cv2.destroyAllWindows()
