import cv2
import numpy as np
import os
import glob
import json  # For JSON output
import argparse  # For command-line arguments


def calibrate_camera(image_dir, output_file, checkerboard_size=(7, 10)):
    """
    Performs camera calibration using checkerboard images and saves results to a JSON file.

    Args:
        image_dir (str): Path to the directory containing checkerboard images (e.g., './images').
        output_file (str): Path to the output JSON file for storing calibration results.
        checkerboard_size (tuple): Dimensions of the checkerboard (corners_x, corners_y).
    """
    print(f"Starting camera calibration...")
    print(f"Image directory: {image_dir}")
    print(f"Output file: {output_file}")
    print(f"Checkerboard size: {checkerboard_size}")

    # Defining the dimensions of checkerboard
    CHECKERBOARD = checkerboard_size
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    # Creating vector to store vectors of 3D points for each checkerboard image
    objpoints = []
    # Creating vector to store vectors of 2D points for each checkerboard image
    imgpoints = []

    # Defining the world coordinates for 3D points
    objp = np.zeros((1, CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32)
    objp[0, :, :2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2)

    # Extracting path of individual image stored in a given directory
    # Supporting multiple image formats
    image_patterns = ['*.jpg', '*.jpeg', '*.png', '*.bmp', '*.tiff']
    images = []
    for pattern in image_patterns:
        images.extend(glob.glob(os.path.join(image_dir, pattern)))

    if not images:
        print(f"Error: No images found in directory '{image_dir}' with patterns {image_patterns}.")
        return

    print(f"Found {len(images)} images.")

    # To store the shape of the first valid gray image for calibrateCamera
    gray_shape_for_calibration = None

    for i, fname in enumerate(images):
        print(f"Processing image {i + 1}/{len(images)}: {os.path.basename(fname)}...")
        img = cv2.imread(fname)
        if img is None:
            print(f"Warning: Could not read image {fname}. Skipping.")
            continue

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        if gray_shape_for_calibration is None:  # Store shape from the first valid image
            gray_shape_for_calibration = gray.shape[::-1]  # (width, height)

        # Find the chess board corners
        # If desired number of corners are found in the image then ret = true
        ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD,
                                                 cv2.CALIB_CB_ADAPTIVE_THRESH +
                                                 cv2.CALIB_CB_FAST_CHECK +
                                                 cv2.CALIB_CB_NORMALIZE_IMAGE)

        """
        If desired number of corner are detected,
        we refine the pixel coordinates and display 
        them on the images of checker board
        """
        if ret == True:
            objpoints.append(objp)
            # refining pixel coordinates for given 2d points.
            corners2 = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
            imgpoints.append(corners2)
            print(f"  -> Checkerboard found and corners refined for {os.path.basename(fname)}")

            # Optional: Draw and display the corners (can be slow for many images)
            # img_with_corners = cv2.drawChessboardCorners(img.copy(), CHECKERBOARD, corners2, ret)
            # cv2.imshow('img',img_with_corners)
            # cv2.waitKey(500) # Display for 0.5 seconds
        else:
            print(f"  -> Checkerboard not found in {os.path.basename(fname)}")

    # cv2.destroyAllWindows() # Close any display windows if they were opened

    if not objpoints or not imgpoints:
        print("Error: No checkerboard corners were detected in any of the images. Calibration cannot proceed.")
        return

    if gray_shape_for_calibration is None:
        print("Error: Could not determine image dimensions for calibration.")
        return

    print(f"\nPerforming camera calibration with {len(objpoints)} image(s) where corners were found...")
    """
    Performing camera calibration by 
    passing the value of known 3D points (objpoints)
    and corresponding pixel coordinates of the 
    detected corners (imgpoints)
    """
    # gray.shape[::-1] gives (width, height)
    ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray_shape_for_calibration, None, None)

    if not ret:
        print("Error: Camera calibration failed.")
        return

    print("\nCalibration successful!")
    print("Camera matrix (mtx):")
    print(mtx)
    print("\nDistortion coefficients (dist):")
    print(dist)
    # print("\nRotation vectors (rvecs):") # These can be very long
    # for i, rvec in enumerate(rvecs):
    #     print(f"  Image {i+1}: {rvec.ravel()}")
    # print("\nTranslation vectors (tvecs):") # These can also be very long
    # for i, tvec in enumerate(tvecs):
    #     print(f"  Image {i+1}: {tvec.ravel()}")

    mean_error = 0
    for i in range(len(objpoints)):
        imgpoints2, _ = cv2.projectPoints(objpoints[i], rvecs[i], tvecs[i], mtx, dist)
        error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
        mean_error += error
    print(f"\nTotal (Mean) Reprojection Error: {mean_error / len(objpoints)}")

    # Prepare data for JSON
    # NumPy arrays are not directly serializable to JSON, convert to lists
    calibration_results = {
        "camera_matrix": mtx.tolist() if mtx is not None else None,
        "distortion_coefficients": dist.tolist() if dist is not None else None,
        "rotation_vectors": [r.tolist() for r in rvecs] if rvecs is not None else None,
        "translation_vectors": [t.tolist() for t in tvecs] if tvecs is not None else None,
        "calibration_success": bool(ret),
        "image_dimensions_wh": gray_shape_for_calibration,
        "checkerboard_dimensions_wh": CHECKERBOARD,
        "num_images_used": len(objpoints),
        "mean_reprojection_error": mean_error / len(objpoints) if objpoints else float('nan')
    }

    # Write to JSON file
    try:
        with open(output_file, 'w') as f:
            json.dump(calibration_results, f, indent=4)
        print(f"\nCalibration results successfully saved to: {output_file}")
    except IOError as e:
        print(f"Error: Could not write to output file {output_file}. {e}")
    except Exception as e:
        print(f"An unexpected error occurred while saving JSON: {e}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Camera Calibration Script")
    parser.add_argument(
        "-i", "--image_dir",
        type=str,
        default="./images",
        help="Directory containing checkerboard images (default: ./images)"
    )
    parser.add_argument(
        "-o", "--output_file",
        type=str,
        default="calibration_results.json",
        help="Path to the output JSON file for calibration results (default: calibration_results.json)"
    )
    parser.add_argument(
        "-cw", "--checkerboard_width",
        type=int,
        default=7,
        help="Number of inner corners along the width of the checkerboard (default: 7)"
    )
    parser.add_argument(
        "-ch", "--checkerboard_height",
        type=int,
        default=10,
        help="Number of inner corners along the height of the checkerboard (default: 10)"
    )

    args = parser.parse_args()

    # Create image directory if it doesn't exist (useful for first-time users)
    if not os.path.isdir(args.image_dir):
        try:
            os.makedirs(args.image_dir)
            print(f"Created image directory: {args.image_dir}")
            print(f"Please place your checkerboard images in this directory and run the script again.")
            exit()  # Exit so the user can add images
        except OSError as e:
            print(f"Error: Could not create image directory {args.image_dir}. {e}")
            exit()

    checkerboard_dims = (args.checkerboard_width, args.checkerboard_height)
    calibrate_camera(args.image_dir, args.output_file, checkerboard_dims)