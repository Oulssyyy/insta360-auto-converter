#include "exif_metadata.h"
#include <exiv2/exiv2.hpp>
#include <iostream>
#include <sstream>

bool add360ExifMetadata(const std::string& imagePath, const std::string& originalPath, int width, int height) {
    try {
        // First, copy metadata from original file to converted file
        std::cout << "Copying metadata from original file: " << originalPath << std::endl;
        
        // Open the original file to read its metadata
        auto originalImage = Exiv2::ImageFactory::open(originalPath);
        if (!originalImage.get()) {
            std::cerr << "Warning: Cannot open original file: " << originalPath << std::endl;
            std::cerr << "Proceeding without copying original metadata..." << std::endl;
        } else {
            originalImage->readMetadata();
        }
        
        // Open the converted image file
        auto image = Exiv2::ImageFactory::open(imagePath);
        if (!image.get()) {
            std::cerr << "Error: Cannot open converted image file: " << imagePath << std::endl;
            return false;
        }

        // Read existing metadata (should be minimal for converted files)
        image->readMetadata();

        // Get EXIF data from converted image
        Exiv2::ExifData& exifData = image->exifData();
        
        // Copy important metadata from original file if available
        if (originalImage.get()) {
            const Exiv2::ExifData& originalExifData = originalImage->exifData();
            
            // Copy essential camera information
            auto copyIfExists = [&](const std::string& key) {
                try {
                    auto iter = originalExifData.findKey(Exiv2::ExifKey(key));
                    if (iter != originalExifData.end()) {
                        exifData[key] = iter->value();
                        std::cout << "  Copied " << key << ": " << iter->value() << std::endl;
                    } else {
                        std::cout << "  Key not found in original: " << key << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cout << "  Error copying " << key << ": " << e.what() << std::endl;
                }
            };
            
            // Copy important EXIF tags (using exact tags from .insp file)
            copyIfExists("Exif.Image.Make");
            copyIfExists("Exif.Image.Model");  // This becomes "Camera Model Name"
            copyIfExists("Exif.Image.DateTime");
            copyIfExists("Exif.Image.Software");
            copyIfExists("Exif.Photo.DateTimeOriginal");
            copyIfExists("Exif.Photo.DateTimeDigitized");
            copyIfExists("Exif.Photo.ExposureTime");
            copyIfExists("Exif.Photo.FNumber");
            copyIfExists("Exif.Photo.ISOSpeedRatings");  // Correct EXIF tag for ISO
            copyIfExists("Exif.Photo.WhiteBalance");
            copyIfExists("Exif.Photo.Flash");
            copyIfExists("Exif.Photo.ExposureProgram");
            copyIfExists("Exif.Photo.MeteringMode");
            copyIfExists("Exif.Photo.FocalLength");
            copyIfExists("Exif.GPS.GPSVersionID");
            copyIfExists("Exif.GPS.GPSLatitude");
            copyIfExists("Exif.GPS.GPSLatitudeRef");
            copyIfExists("Exif.GPS.GPSLongitude");
            copyIfExists("Exif.GPS.GPSLongitudeRef");
            copyIfExists("Exif.GPS.GPSAltitude");
            copyIfExists("Exif.GPS.GPSAltitudeRef");
        }

        // Print some existing metadata for debugging
        std::cout << "Preserving existing EXIF data..." << std::endl;
        if (!exifData.empty()) {
            // Check if we have existing camera info
            auto makeIter = exifData.findKey(Exiv2::ExifKey("Exif.Image.Make"));
            auto modelIter = exifData.findKey(Exiv2::ExifKey("Exif.Image.Model"));
            auto datetimeIter = exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTime"));
            
            if (makeIter != exifData.end()) {
                std::cout << "  Preserving Make: " << makeIter->value() << std::endl;
            }
            if (modelIter != exifData.end()) {
                std::cout << "  Preserving Model: " << modelIter->value() << std::endl;
            }
            if (datetimeIter != exifData.end()) {
                std::cout << "  Preserving DateTime: " << datetimeIter->value() << std::endl;
            }
        }

        // Add ONLY the 360° panorama metadata (without overwriting existing data)
        std::cout << "Adding 360° panorama metadata..." << std::endl;
        
        // Method 1: XMP metadata (Google Photo Sphere standard)
        Exiv2::XmpData& xmpData = image->xmpData();
        
        // Register GPano namespace if not already registered
        try {
            Exiv2::XmpProperties::registerNs("http://ns.google.com/photos/1.0/panorama/", "GPano");
        } catch (...) {
            // Namespace might already be registered, ignore error
        }
        
        // Set XMP GPano tags for Google Photo Sphere standard
        xmpData["Xmp.GPano.ProjectionType"] = "equirectangular";
        xmpData["Xmp.GPano.UsePanoramaViewer"] = "True";
        xmpData["Xmp.GPano.StitchingSoftware"] = "Insta360 SDK";
        xmpData["Xmp.GPano.FullPanoWidthPixels"] = std::to_string(width);
        xmpData["Xmp.GPano.FullPanoHeightPixels"] = std::to_string(height);
        xmpData["Xmp.GPano.CroppedAreaImageWidthPixels"] = std::to_string(width);
        xmpData["Xmp.GPano.CroppedAreaImageHeightPixels"] = std::to_string(height);
        xmpData["Xmp.GPano.CroppedAreaLeftPixels"] = "0";
        xmpData["Xmp.GPano.CroppedAreaTopPixels"] = "0";
        
        std::cout << "  Added XMP GPano tags for 360° recognition" << std::endl;
        
        // Method 2: Add standard EXIF tags for panorama recognition
        exifData["Exif.Image.Orientation"] = static_cast<uint16_t>(1); // Normal orientation
        exifData["Exif.Photo.SceneCaptureType"] = static_cast<uint16_t>(4); // Other (can indicate panorama)
        
        // Method 3: Add custom EXIF tags that some viewers recognize
        exifData["Exif.Image.ImageDescription"] = "360 degree panorama";
        exifData["Exif.Photo.WhiteBalance"] = static_cast<uint16_t>(0); // Auto white balance
        
        // DO NOT override existing Make/Model/DateTime if they exist
        // Only set these if they don't already exist
        auto makeIter = exifData.findKey(Exiv2::ExifKey("Exif.Image.Make"));
        if (makeIter == exifData.end()) {
            exifData["Exif.Image.Make"] = "Insta360";
        }
        
        // Always update software to indicate processing
        exifData["Exif.Image.Software"] = "Insta360 Auto Converter";
        
        // Write the metadata back to the file (this preserves existing data and adds new)
        image->writeMetadata();
        
        std::cout << "Successfully added 360° EXIF metadata while preserving existing data" << std::endl;
        return true;
        
    } catch (const Exiv2::Error& e) {
        std::cerr << "Exiv2 error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error adding EXIF metadata: " << e.what() << std::endl;
        return false;
    }
}