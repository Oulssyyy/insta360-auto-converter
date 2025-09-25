#include "resolution_detector.h"
#include <exiv2/exiv2.hpp>
#include <iostream>
#include <algorithm>
#include <map>
#include <iomanip>

// Resolution database by Insta360 model
std::map<std::string, ResolutionInfo> MODEL_RESOLUTIONS = {
    // Insta360 X4 - Latest model (8K)
    {"Insta360 X4", {11904, 5952, "Insta360 X4"}},
    {"X4", {11904, 5952, "Insta360 X4"}},
    
    // Insta360 X3 - Popular model (5.7K)
    {"Insta360 X3", {11520, 5760, "Insta360 X3"}},
    {"X3", {11520, 5760, "Insta360 X3"}},
    
    // Insta360 X2 - Previous generation (5.7K)
    {"Insta360 ONE X2", {11520, 5760, "Insta360 ONE X2"}},
    {"ONE X2", {11520, 5760, "Insta360 ONE X2"}},
    {"X2", {11520, 5760, "Insta360 ONE X2"}},
    
    // Insta360 ONE X - First generation (5.7K)
    {"Insta360 ONE X", {11520, 5760, "Insta360 ONE X"}},
    {"ONE X", {11520, 5760, "Insta360 ONE X"}},
    
    // Insta360 ONE R - Modular (5.3K)
    {"Insta360 ONE R", {10560, 5280, "Insta360 ONE R"}},
    {"ONE R", {10560, 5280, "Insta360 ONE R"}},
    
    // Insta360 ONE RS - Enhanced version (6K)
    {"Insta360 ONE RS", {12000, 6000, "Insta360 ONE RS"}},
    {"ONE RS", {12000, 6000, "Insta360 ONE RS"}},
    
    // Older models
    {"Insta360 ONE", {7680, 3840, "Insta360 ONE"}},
    {"ONE", {7680, 3840, "Insta360 ONE"}},
    
    // Default fallback (X4 as reference)
    {"Unknown", {11904, 5952, "Unknown Model (X4 Default)"}},
};

std::string extractCameraModel(const std::string& file_path) {
    try {
        auto image = Exiv2::ImageFactory::open(file_path);
        if (!image.get()) {
            std::cerr << "Warning: Cannot open image for model detection: " << file_path << std::endl;
            return "Unknown";
        }

        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        
        // Search for model in different EXIF fields
        std::vector<std::string> model_tags = {
            "Exif.Image.Model",
            "Exif.Image.Make", 
            "Exif.Photo.CameraOwnerName",
            "Exif.Iop.InteroperabilityIndex"
        };
        
        for (const auto& tag : model_tags) {
            auto it = exifData.findKey(Exiv2::ExifKey(tag));
            if (it != exifData.end()) {
                std::string model = it->print();
                
                // Clean and normalize model name
                std::transform(model.begin(), model.end(), model.begin(), ::tolower);
                
                // Detection by keywords
                if (model.find("x4") != std::string::npos) return "Insta360 X4";
                if (model.find("x3") != std::string::npos) return "Insta360 X3";
                if (model.find("x2") != std::string::npos || model.find("one x2") != std::string::npos) return "Insta360 ONE X2";
                if (model.find("one x") != std::string::npos && model.find("x2") == std::string::npos) return "Insta360 ONE X";
                if (model.find("one r") != std::string::npos && model.find("rs") == std::string::npos) return "Insta360 ONE R";
                if (model.find("one rs") != std::string::npos || model.find("oners") != std::string::npos) return "Insta360 ONE RS";
                if (model.find("one") != std::string::npos && model.find("x") == std::string::npos && model.find("r") == std::string::npos) return "Insta360 ONE";
                
                // If we find "insta360", return the complete model
                if (model.find("insta360") != std::string::npos) {
                    return it->print(); // Return original name
                }
            }
        }
        
        // Try to detect via metadata resolution
        auto width_it = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        auto height_it = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        
        if (width_it != exifData.end() && height_it != exifData.end()) {
            int width = width_it->toLong();
            int height = height_it->toLong();
            
            // Deduction by resolution
            if (width >= 11900) return "Insta360 X4";      // 8K
            if (width >= 11500) return "Insta360 X3";      // 5.7K
            if (width >= 12000 && height >= 6000) return "Insta360 ONE RS"; // 6K
            if (width >= 10500) return "Insta360 ONE R";   // 5.3K
            if (width >= 7680) return "Insta360 ONE";      // 4K
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Warning: EXIF reading error: " << e.what() << std::endl;
    }
    
    return "Unknown";
}

ResolutionInfo getResolutionForModel(const std::string& model_name) {
    auto it = MODEL_RESOLUTIONS.find(model_name);
    if (it != MODEL_RESOLUTIONS.end()) {
        return it->second;
    }
    
    // Fuzzy search for name variants
    for (const auto& [key, resolution] : MODEL_RESOLUTIONS) {
        if (model_name.find(key) != std::string::npos || key.find(model_name) != std::string::npos) {
            return resolution;
        }
    }
    
    // Fallback to X4 (highest resolution)
    std::cout << "Warning: Unknown model '" << model_name << "', using X4 default resolution" << std::endl;
    return MODEL_RESOLUTIONS["Unknown"];
}

ResolutionInfo detectOptimalResolution(const std::string& input_file_path) {
    std::cout << "🔍 Detecting camera model and optimal resolution..." << std::endl;
    
    std::string detected_model = extractCameraModel(input_file_path);
    ResolutionInfo resolution = getResolutionForModel(detected_model);
    
    std::cout << "📷 Detected Model: " << resolution.model_name << std::endl;
    std::cout << "📐 Optimal Resolution: " << resolution.width << "x" << resolution.height << std::endl;
    
    // Calculate megapixels for information
    double megapixels = (resolution.width * resolution.height) / 1000000.0;
    std::cout << "🎯 Output Quality: " << std::fixed << std::setprecision(1) << megapixels << " MP" << std::endl;
    
    return resolution;
}