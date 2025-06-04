# Camera Calibration Using OpenCV

This project performs camera calibration using OpenCV in both C++ and Python. It computes the camera matrix and distortion coefficients from a set of calibration images and applies these parameters to undistort images. Based on the original [LearnOpenCV project](https://github.com/spmallick/learnopencv/tree/master/CameraCalibration), this version has been enhanced with modern features and improved usability.

## Features
- Compute camera calibration parameters from chessboard images
- Undistort images using computed calibration
- C++ and Python implementations with consistent APIs
- Command-line argument parsing for flexible usage
- JSON output for calibration results
- Support for multiple image formats (jpg, jpeg, png, bmp, tiff)
- Robust error handling and validation
- Progress reporting during calibration
- Automatic directory creation
- Reprojection error calculation
- Optional display functionality

## C++ Usage

### Compilation

Option 1 - Direct compilation:
```bash
g++ -o cameraCalibration cameraCalibration.cpp `pkg-config --cflags --libs opencv4` -std=c++17
g++ -o cameraCalibrationWithUndistortion cameraCalibrationWithUndistortion.cpp `pkg-config --cflags --libs opencv4` -std=c++17
```

Option 2 - CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Execution

Basic usage:
```bash
./cameraCalibration
./cameraCalibrationWithUndistortion
```

With command-line options:
```bash
./cameraCalibration -i ./images -o calibration_results.json -cw 7 -ch 10
./cameraCalibrationWithUndistortion -i ./images -o results.json --no-display
```

### C++ Command-line Options
- `-i, --image_dir <dir>`: Directory containing checkerboard images (default: ./images)
- `-o, --output_file <file>`: Path to output JSON file (default: calibration_results.json)
- `-cw, --checkerboard_width <width>`: Number of inner corners along width (default: 7)
- `-ch, --checkerboard_height <height>`: Number of inner corners along height (default: 10)
- `--no-display`: Skip displaying undistorted image (undistortion version only)
- `-h, --help`: Show help message

## Python Usage

### Setup

Use Python 3.13 in a virtual environment:

```bash
python3.13 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Execution

Basic usage:
```bash
python3 cameraCalibration.py
python3 cameraCalibrationWithUndistortion.py
```

With command-line options:
```bash
python3 cameraCalibration.py -i ./images -o calibration_results.json -cw 7 -ch 10
python3 cameraCalibrationWithUndistortion.py -i ./images -o results.json --no-display
```

### Python Command-line Options
- `-i, --image_dir <dir>`: Directory containing checkerboard images (default: ./images)
- `-o, --output_file <file>`: Path to output JSON file (default: calibration_results.json)
- `-cw, --checkerboard_width <width>`: Number of inner corners along width (default: 7)
- `-ch, --checkerboard_height <height>`: Number of inner corners along height (default: 10)
- `--no-display`: Skip displaying undistorted image (undistortion version only)
- `-h, --help`: Show help message

## Output Format

The calibration results are saved in JSON format with the following structure:
```json
{
    "camera_matrix": [[fx, 0, cx], [0, fy, cy], [0, 0, 1]],
    "distortion_coefficients": [k1, k2, p1, p2, k3],
    "rotation_vectors": [...],
    "translation_vectors": [...],
    "calibration_success": true,
    "image_dimensions_wh": [width, height],
    "checkerboard_dimensions_wh": [width, height],
    "num_images_used": 34,
    "mean_reprojection_error": 0.329
}
```

## Notes

* Input images must show a **chessboard pattern** with clearly visible corners
* The program automatically creates the `./images` directory if it doesn't exist
* Supports multiple image formats: jpg, jpeg, png, bmp, tiff
* The default checkerboard size is 7x10 (inner corners), adjust if your pattern differs
* Images where checkerboards cannot be detected are automatically skipped
* Calibration requires at least one successful checkerboard detection
* Lower reprojection error indicates better calibration quality (typically < 1.0 pixel)
* Results include camera matrix, distortion coefficients, and quality metrics
