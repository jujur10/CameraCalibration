#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

struct CalibrationResults {
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    std::vector<cv::Mat> rvecs;
    std::vector<cv::Mat> tvecs;
    bool success;
    cv::Size imageSize;
    cv::Size checkerboardSize;
    int numImagesUsed;
    double meanReprojectionError;
};

void saveCalibrationResultsToJSON(const CalibrationResults& results, const std::string& outputFile) {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to output file " << outputFile << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"camera_matrix\": [\n";
    for (int i = 0; i < 3; i++) {
        file << "    [";
        for (int j = 0; j < 3; j++) {
            file << results.cameraMatrix.at<double>(i, j);
            if (j < 2) file << ", ";
        }
        file << "]";
        if (i < 2) file << ",";
        file << "\n";
    }
    file << "  ],\n";
    
    file << "  \"distortion_coefficients\": [";
    for (int i = 0; i < results.distCoeffs.rows; i++) {
        file << results.distCoeffs.at<double>(i, 0);
        if (i < results.distCoeffs.rows - 1) file << ", ";
    }
    file << "],\n";
    
    file << "  \"rotation_vectors\": [\n";
    for (size_t i = 0; i < results.rvecs.size(); i++) {
        file << "    [";
        for (int j = 0; j < 3; j++) {
            file << results.rvecs[i].at<double>(j, 0);
            if (j < 2) file << ", ";
        }
        file << "]";
        if (i < results.rvecs.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ],\n";
    
    file << "  \"translation_vectors\": [\n";
    for (size_t i = 0; i < results.tvecs.size(); i++) {
        file << "    [";
        for (int j = 0; j < 3; j++) {
            file << results.tvecs[i].at<double>(j, 0);
            if (j < 2) file << ", ";
        }
        file << "]";
        if (i < results.tvecs.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ],\n";
    
    file << "  \"calibration_success\": " << (results.success ? "true" : "false") << ",\n";
    file << "  \"image_dimensions_wh\": [" << results.imageSize.width << ", " << results.imageSize.height << "],\n";
    file << "  \"checkerboard_dimensions_wh\": [" << results.checkerboardSize.width << ", " << results.checkerboardSize.height << "],\n";
    file << "  \"num_images_used\": " << results.numImagesUsed << ",\n";
    file << "  \"mean_reprojection_error\": " << results.meanReprojectionError << "\n";
    file << "}\n";
    
    file.close();
    std::cout << "\nCalibration results successfully saved to: " << outputFile << std::endl;
}

CalibrationResults calibrateCamera(const std::string& imageDir, const cv::Size& checkerboardSize) {
    std::cout << "Starting camera calibration..." << std::endl;
    std::cout << "Image directory: " << imageDir << std::endl;
    std::cout << "Checkerboard size: " << checkerboardSize.width << "x" << checkerboardSize.height << std::endl;

    CalibrationResults results;
    results.checkerboardSize = checkerboardSize;
    results.success = false;
    results.numImagesUsed = 0;
    results.meanReprojectionError = 0.0;

    // Creating vector to store vectors of 3D points for each checkerboard image
    std::vector<std::vector<cv::Point3f>> objpoints;
    // Creating vector to store vectors of 2D points for each checkerboard image
    std::vector<std::vector<cv::Point2f>> imgpoints;

    // Defining the world coordinates for 3D points
    std::vector<cv::Point3f> objp;
    for (int i = 0; i < checkerboardSize.height; i++) {
        for (int j = 0; j < checkerboardSize.width; j++) {
            objp.push_back(cv::Point3f(j, i, 0));
        }
    }

    // Extracting path of individual image stored in a given directory
    std::vector<cv::String> images;
    std::vector<std::string> patterns = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tiff"};
    
    for (const auto& pattern : patterns) {
        std::vector<cv::String> temp;
        cv::glob(imageDir + "/" + pattern, temp);
        images.insert(images.end(), temp.begin(), temp.end());
    }

    if (images.empty()) {
        std::cerr << "Error: No images found in directory '" << imageDir << "' with supported patterns." << std::endl;
        return results;
    }

    std::cout << "Found " << images.size() << " images." << std::endl;

    cv::Mat frame, gray;
    std::vector<cv::Point2f> corner_pts;
    bool success;
    cv::Size imageSize;
    bool imageSizeSet = false;

    // Looping over all the images in the directory
    for (size_t i = 0; i < images.size(); i++) {
        std::cout << "Processing image " << (i + 1) << "/" << images.size() << ": " 
                  << std::filesystem::path(images[i]).filename() << "..." << std::endl;
        
        frame = cv::imread(images[i]);
        if (frame.empty()) {
            std::cout << "Warning: Could not read image " << images[i] << ". Skipping." << std::endl;
            continue;
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        if (!imageSizeSet) {
            imageSize = cv::Size(gray.cols, gray.rows);
            imageSizeSet = true;
        }

        // Finding checker board corners
        success = cv::findChessboardCorners(gray, checkerboardSize, corner_pts, 
                                          cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

        if (success) {
            cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001);
            cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

            objpoints.push_back(objp);
            imgpoints.push_back(corner_pts);
            
            std::cout << "  -> Checkerboard found and corners refined for " 
                      << std::filesystem::path(images[i]).filename() << std::endl;
        } else {
            std::cout << "  -> Checkerboard not found in " 
                      << std::filesystem::path(images[i]).filename() << std::endl;
        }
    }

    if (objpoints.empty() || imgpoints.empty()) {
        std::cerr << "Error: No checkerboard corners were detected in any of the images. Calibration cannot proceed." << std::endl;
        return results;
    }

    if (!imageSizeSet) {
        std::cerr << "Error: Could not determine image dimensions for calibration." << std::endl;
        return results;
    }

    std::cout << "\nPerforming camera calibration with " << objpoints.size() << " image(s) where corners were found..." << std::endl;

    results.success = cv::calibrateCamera(objpoints, imgpoints, imageSize, 
                                        results.cameraMatrix, results.distCoeffs, 
                                        results.rvecs, results.tvecs);

    if (!results.success) {
        std::cerr << "Error: Camera calibration failed." << std::endl;
        return results;
    }

    results.imageSize = imageSize;
    results.numImagesUsed = objpoints.size();

    std::cout << "\nCalibration successful!" << std::endl;
    std::cout << "Camera matrix:" << std::endl << results.cameraMatrix << std::endl;
    std::cout << "\nDistortion coefficients:" << std::endl << results.distCoeffs << std::endl;

    // Calculate mean reprojection error
    double totalError = 0.0;
    for (size_t i = 0; i < objpoints.size(); i++) {
        std::vector<cv::Point2f> imgpoints2;
        cv::projectPoints(objpoints[i], results.rvecs[i], results.tvecs[i], 
                         results.cameraMatrix, results.distCoeffs, imgpoints2);
        double error = cv::norm(imgpoints[i], imgpoints2, cv::NORM_L2) / imgpoints2.size();
        totalError += error;
    }
    results.meanReprojectionError = totalError / objpoints.size();
    std::cout << "\nTotal (Mean) Reprojection Error: " << results.meanReprojectionError << std::endl;

    return results;
}

void showUndistortedImage(const CalibrationResults& results, const std::string& imageDir) {
    if (!results.success) {
        std::cerr << "Cannot show undistorted image: calibration was not successful." << std::endl;
        return;
    }

    // Find first image to use for undistortion demonstration
    std::vector<cv::String> images;
    std::vector<std::string> patterns = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tiff"};
    
    for (const auto& pattern : patterns) {
        std::vector<cv::String> temp;
        cv::glob(imageDir + "/" + pattern, temp);
        images.insert(images.end(), temp.begin(), temp.end());
    }

    if (images.empty()) {
        std::cerr << "No images found for undistortion demonstration." << std::endl;
        return;
    }

    cv::Mat img = cv::imread(images[0]);
    if (img.empty()) {
        std::cerr << "Could not read image for undistortion demonstration." << std::endl;
        return;
    }

    cv::Mat dst, newCameraMatrix;
    cv::Size imageSize(img.cols, img.rows);

    // Refining the camera matrix using parameters obtained by calibration
    newCameraMatrix = cv::getOptimalNewCameraMatrix(results.cameraMatrix, results.distCoeffs, imageSize, 1, imageSize, 0);

    // Method 1 to undistort the image
    cv::undistort(img, dst, results.cameraMatrix, results.distCoeffs, newCameraMatrix);

    // Display the undistorted image
    std::cout << "\nDisplaying undistorted image. Press any key to continue..." << std::endl;
    cv::imshow("Original Image", img);
    cv::imshow("Undistorted Image", dst);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --image_dir <dir>        Directory containing checkerboard images (default: ./images)\n";
    std::cout << "  -o, --output_file <file>     Path to output JSON file (default: calibration_results.json)\n";
    std::cout << "  -cw, --checkerboard_width <width>   Number of inner corners along width (default: 7)\n";
    std::cout << "  -ch, --checkerboard_height <height> Number of inner corners along height (default: 10)\n";
    std::cout << "  --no-display                 Skip displaying undistorted image\n";
    std::cout << "  -h, --help                   Show this help message\n";
}

int main(int argc, char* argv[]) {
    std::string imageDir = "./images";
    std::string outputFile = "calibration_results.json";
    int checkerboardWidth = 7;
    int checkerboardHeight = 10;
    bool showDisplay = true;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if ((arg == "-i" || arg == "--image_dir") && i + 1 < argc) {
            imageDir = argv[++i];
        } else if ((arg == "-o" || arg == "--output_file") && i + 1 < argc) {
            outputFile = argv[++i];
        } else if ((arg == "-cw" || arg == "--checkerboard_width") && i + 1 < argc) {
            checkerboardWidth = std::stoi(argv[++i]);
        } else if ((arg == "-ch" || arg == "--checkerboard_height") && i + 1 < argc) {
            checkerboardHeight = std::stoi(argv[++i]);
        } else if (arg == "--no-display") {
            showDisplay = false;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Create image directory if it doesn't exist
    if (!std::filesystem::exists(imageDir)) {
        try {
            std::filesystem::create_directories(imageDir);
            std::cout << "Created image directory: " << imageDir << std::endl;
            std::cout << "Please place your checkerboard images in this directory and run the script again." << std::endl;
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: Could not create image directory " << imageDir << ". " << e.what() << std::endl;
            return 1;
        }
    }

    cv::Size checkerboardSize(checkerboardWidth, checkerboardHeight);
    CalibrationResults results = calibrateCamera(imageDir, checkerboardSize);

    if (results.success) {
        saveCalibrationResultsToJSON(results, outputFile);
        
        if (showDisplay) {
            showUndistortedImage(results, imageDir);
        }
    }

    return results.success ? 0 : 1;
}