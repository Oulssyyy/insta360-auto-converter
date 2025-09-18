#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

// Inclure les headers du SDK
#include "ins_media.h"   // selon le nom exact dans include du SDK
#include "ins_video_stitcher.h"
#include "ins_photo_stitcher.h"

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

        // Paramètres (optionnels)
        videoStitcher->EnableFlowState(true);      // stabilisation
        videoStitcher->EnableDirectionLock(true);  // direction lock
        videoStitcher->EnableH265Encoder();        // H.265 si dispo
        videoStitcher->SetOutputBitRate(60LL * 1000 * 1000); // 60 Mbps
        videoStitcher->SetOutputSize(3840, 1920);  // 4K (ratio 2:1)

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

        auto photoStitcher = std::make_shared<ins::PhotoStitcher>();

        std::vector<std::string> inputs = { input };
        photoStitcher->SetInputPath(inputs);

        std::string output_path = (fs::path(output_dir) / (fs::path(input).stem().string() + ".jpg")).string();
        photoStitcher->SetOutputPath(output_path);

        photoStitcher->EnableStitchFusion(true);

        photoStitcher->StartStitch();
        std::cout << "Export finished: " << output_path << std::endl;
    } else {
        std::cerr << "Unsupported file type: " << ext << "\n";
        return 1;
    }

    return 0;
}
