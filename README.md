# Camera Calibration Using OpenCV

This project performs camera calibration using OpenCV in both C++ and Python. It computes the camera matrix and distortion coefficients from a set of calibration images and applies these parameters to undistort images. Based on the original [LearnOpenCV project](https://github.com/spmallick/learnopencv/tree/master/CameraCalibration), this version removes all visualization features.

## Features
- Compute camera calibration parameters from chessboard images.
- Undistort images using computed calibration.
- C++ and Python implementations.

## C++ Usage

### Compilation
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
````

### Execution

```bash
./cameraCalibration
./cameraCalibrationWithUndistortion
```

## Python Usage

### Setup

Use Python 3.13 in a virtual environment:

```bash
python3.13 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Execution

Place calibration images in the `./images` directory.

Run:

```bash
python3 cameraCalibration.py
python3 cameraCalibrationWithUndistortion.py
```

## Notes

* Input images must show a chessboard pattern.
* Results include saved calibration parameters and undistorted output.
