#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

// Inclure les headers du SDK
#include "ins_stitcher.h"   // Contains VideoStitcher and ImageStitcher classes
#include "ins_common.h"     // Contains common types and enums
#include "exif_metadata.h"  // For adding 360° EXIF metadata
#include "resolution_detector.h"  // For dynamic resolution detection

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.insv|insp> <output_dir>\n";
        return 1;
    }

    std::string input = argv[1];
    std::string output_dir = argv[2];

    if (!fs::exists(input)) {
        std::cerr << "Input file not found: " << input << "\n";
        return 1;
    }
    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }

    ins::InitEnv();

    std::string ext = fs::path(input).extension().string();
    if (ext == ".insv") {
        std::cout << "Converting video: " << input << std::endl;

        auto videoStitcher = std::make_shared<ins::VideoStitcher>();

        std::vector<std::string> inputs = { input };
        videoStitcher->SetInputPath(inputs);

        std::string output_path = (fs::path(output_dir) / (fs::path(input).stem().string() + ".mp4")).string();
        videoStitcher->SetOutputPath(output_path);

        // Parameters (optional)
        videoStitcher->EnableFlowState(true);      // stabilization
        videoStitcher->EnableDirectionLock(true);  // direction lock
        videoStitcher->EnableH265Encoder();        // H.265 if available
        videoStitcher->SetOutputBitRate(60LL * 1000 * 1000); // 60 Mbps
        videoStitcher->SetOutputSize(3840, 1920);  // 4K (2:1 ratio)

        videoStitcher->SetStitchProgressCallback([&](int progress, int error) {
            std::cout << "\rProgress: " << progress << "%" << std::flush;
            if (error != 0) {
                std::cerr << "\nError during stitch: code " << error << std::endl;
            }
        });

        videoStitcher->StartStitch();
        std::cout << "\nExport finished: " << output_path << std::endl;

    } else if (ext == ".insp" || ext == ".jpg") {
        std::cout << "Converting photo: " << input << std::endl;

        auto imageStitcher = std::make_shared<ins::ImageStitcher>();

        std::vector<std::string> inputs = { input };
        imageStitcher->SetInputPath(inputs);

        std::string output_path = (fs::path(output_dir) / (fs::path(input).stem().string() + ".jpg")).string();
        imageStitcher->SetOutputPath(output_path);

        // 🔍 DYNAMIC RESOLUTION DETECTION 
        // Automatically detect optimal resolution based on camera model
        ResolutionInfo resolution = detectOptimalResolution(input);
        
        // Configure for CPU-only processing in containerized environment
        imageStitcher->EnableCuda(false);
        imageStitcher->SetImageProcessingAccelType(ins::ImageProcessingAccel::kCPU);
        
        // 📐 Set optimal resolution dynamically based on detected camera model
        imageStitcher->SetOutputSize(resolution.width, resolution.height);
        
        // ✨ KEY OPTIMIZATION FOR PERFECT JUNCTIONS ✨
        // Use OPTFLOW instead of TEMPLATE for superior seam blending
        // This is the same algorithm used by the official Insta360 Studio
        imageStitcher->SetStitchType(ins::STITCH_TYPE::OPTFLOW);
        
        // ✨ CRITICAL: Enable advanced stitching fusion ✨
        // This enables sophisticated blending at image boundaries
        // Disabled previously to avoid crashes, but essential for quality
        imageStitcher->EnableStitchFusion(true);

        std::cout << "Starting image stitching..." << std::endl;
        bool success = imageStitcher->Stitch();
        std::cout << "Stitching completed with result: " << (success ? "SUCCESS" : "FAILED") << std::endl;
        if (success) {
            std::cout << "Export finished: " << output_path << std::endl;
            
            // Add 360° EXIF metadata to make the image recognizable as a panorama
            // Use the dynamically detected resolution for metadata accuracy
            const int pano_width = resolution.width;   // Dynamic resolution width
            const int pano_height = resolution.height; // Dynamic resolution height
            
            std::cout << "Adding 360° EXIF metadata..." << std::endl;
            if (add360ExifMetadata(output_path, input, pano_width, pano_height)) {
                std::cout << "Successfully added 360° EXIF metadata to " << output_path << std::endl;
            } else {
                std::cerr << "Warning: Failed to add 360° EXIF metadata to " << output_path << std::endl;
            }
        } else {
            std::cerr << "Error during image stitching" << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Unsupported file type: " << ext << "\n";
        return 1;
    }

    return 0;
}
