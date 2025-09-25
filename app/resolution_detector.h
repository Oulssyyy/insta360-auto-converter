#pragma once
#include <string>
#include <utility>

// Structure to store resolution information
struct ResolutionInfo {
    int width;
    int height;
    std::string model_name;
};

/**
 * Automatically detects optimal resolution according to camera model
 * by analyzing EXIF metadata from source file
 */
ResolutionInfo detectOptimalResolution(const std::string& input_file_path);

/**
 * Gets default resolution for a given model
 */
ResolutionInfo getResolutionForModel(const std::string& model_name);

/**
 * Extracts camera model from EXIF metadata
 */
std::string extractCameraModel(const std::string& file_path);