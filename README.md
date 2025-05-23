# Camera Calibration Using OpenCV

This repository is a modified version of https://github.com/spmallick/learnopencv/tree/master/CameraCalibration without visualization features.

## Using the C++ code
### Compilation
To compile the `cameraCalibration.cpp`  and `cameraCalibrationWithUndistortion.cpp` code files, use the following:
```shell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
## Usage

### Using the C++ code

Refer to the following to use the compiled files:

```shell
./build/cameraCalibration
./build/cameraCalibrationWithUndistortion
```

### Using the python code

Create a virtual environment for Python 3.13.
- `python3.13 -m venv .venv`
- `source .venv/bin/activate`
- `pip install -r requirements.txt`

Refer to the following to use the `cameraCalibration.py` and `cameraCalibrationWithUndistortion.py` files respectively:

```shell
python3 cameraCalibration.py
python3 cameraCalibrationWithUndistortion.py
```
