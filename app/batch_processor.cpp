#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <regex>
#include <queue>
#include <mutex>
#include <json/json.h>

// Include SDK headers
#include "ins_stitcher.h"
#include "ins_common.h"
#include "exif_metadata.h"  // For adding 360° EXIF metadata

namespace fs = std::filesystem;

struct ConversionJob {
    std::string inputPath;
    std::string outputPath;
    std::string fileType;
    std::chrono::system_clock::time_point createdAt;
};

class Insta360BatchProcessor {
private:
    std::string inputDir;
    std::string outputDir;
    std::string configFile;
    std::queue<ConversionJob> jobQueue;
    std::mutex queueMutex;
    bool running;
    
    // Configuration
    bool enableGPU = false;
    int outputWidth = 11904;  // Maximum native resolution for Insta360 X4
    int outputHeight = 5952;  // Maximum native resolution (2:1 ratio)
    int bitrate = 50000000; // 50 Mbps
    int maxConcurrentJobs = 1;
    int watchInterval = 30; // seconds
    bool watchMode = false; // Watch mode: continuously monitor for new files
    
public:
    Insta360BatchProcessor(const std::string& input, const std::string& output, const std::string& config) 
        : inputDir(input), outputDir(output), configFile(config), running(false) {
        
        // Ensure directories exist
        fs::create_directories(outputDir);
        
        // Load configuration
        loadConfiguration();
        
        // Initialize SDK
        ins::InitEnv();
        ins::SetLogLevel(ins::InsLogLevel::INFO);
        
        std::cout << "Insta360 Batch Processor initialized" << std::endl;
        std::cout << "Input directory: " << inputDir << std::endl;
        std::cout << "Output directory: " << outputDir << std::endl;
    }
    
    void setWatchMode(bool enabled) {
        watchMode = enabled;
        std::cout << "Watch mode " << (enabled ? "ENABLED" : "DISABLED") << " via command line" << std::endl;
    }
    
    void loadConfiguration() {
        if (!fs::exists(configFile)) {
            createDefaultConfig();
            return;
        }
        
        try {
            std::ifstream file(configFile);
            Json::Value config;
            file >> config;
            
            if (config.isMember("enableGPU")) enableGPU = config["enableGPU"].asBool();
            if (config.isMember("outputWidth")) outputWidth = config["outputWidth"].asInt();
            if (config.isMember("outputHeight")) outputHeight = config["outputHeight"].asInt();
            if (config.isMember("bitrate")) bitrate = config["bitrate"].asInt();
            if (config.isMember("maxConcurrentJobs")) maxConcurrentJobs = config["maxConcurrentJobs"].asInt();
            if (config.isMember("watchInterval")) watchInterval = config["watchInterval"].asInt();
            if (config.isMember("watchMode")) watchMode = config["watchMode"].asBool();
            
            std::cout << "Configuration loaded from: " << configFile << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error loading configuration: " << e.what() << std::endl;
            std::cerr << "Using default configuration" << std::endl;
        }
    }
    
    void createDefaultConfig() {
        Json::Value config;
        config["enableGPU"] = false;
        config["outputWidth"] = 11904;  // Maximum native resolution for Insta360 X4
        config["outputHeight"] = 5952;  // Maximum native resolution (2:1 ratio)
        config["bitrate"] = 50000000;
        config["maxConcurrentJobs"] = 1;
        config["watchInterval"] = 30;
        config["watchMode"] = false;  // Set to true for continuous monitoring
        config["comment"] = "Insta360 Batch Processor Configuration - Set watchMode=true for continuous monitoring";
        
        std::ofstream file(configFile);
        file << config;
        file.close();
        
        std::cout << "Default configuration created: " << configFile << std::endl;
    }
    
    // Check if a file has already been converted by looking in the output directory
    bool isAlreadyConverted(const fs::path& inputPath) {
        try {
            std::string inputFilename = inputPath.stem().string();
            std::string inputExt = inputPath.extension().string();
            
            // Determine expected output extension
            std::string outputExt;
            if (inputExt == ".insv") {
                outputExt = ".mp4";
            } else if (inputExt == ".insp" || inputExt == ".jpg") {
                outputExt = ".jpg";
            } else {
                return false; // Unsupported format
            }
            
            // Get relative path from input directory to maintain folder structure
            fs::path relativePath = fs::relative(inputPath, inputDir);
            fs::path outputPath = fs::path(outputDir) / relativePath.parent_path() / (inputFilename + outputExt);
            
            bool exists = fs::exists(outputPath);
            if (exists) {
                std::cout << "File already converted: " << inputPath.filename() << " -> " << outputPath.filename() << std::endl;
            }
            return exists;
            
        } catch (const std::exception& e) {
            std::cerr << "Error checking conversion status for " << inputPath << ": " << e.what() << std::endl;
            return false; // If we can't check, assume not converted to be safe
        }
    }
    
    void scanForFiles() {
        if (!fs::exists(inputDir)) {
            std::cerr << "Input directory does not exist: " << inputDir << std::endl;
            return;
        }
        
        try {
            for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
                if (!entry.is_regular_file()) continue;
                
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                
                if (extension == ".insv" || extension == ".insp") {
                    // Check if already converted
                    if (isAlreadyConverted(entry.path())) {
                        continue; // Already converted, skip
                    }
                    
                    // Create conversion job
                    ConversionJob job;
                    job.inputPath = entry.path().string();
                    job.fileType = extension;
                    job.createdAt = std::chrono::system_clock::now();
                    
                    // Generate output path
                    std::string outputFileName;
                    if (extension == ".insv") {
                        outputFileName = entry.path().stem().string() + ".mp4";
                    } else {
                        outputFileName = entry.path().stem().string() + ".jpg";
                    }
                    
                    job.outputPath = (fs::path(outputDir) / outputFileName).string();
                    
                    // Add to queue
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        jobQueue.push(job);
                    }
                    
                    std::cout << "Added to queue: " << entry.path().filename() << " (" << extension << ")" << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error scanning directory: " << e.what() << std::endl;
        }
    }
    
    bool processVideo(const ConversionJob& job) {
        std::cout << "Processing video: " << fs::path(job.inputPath).filename() << std::endl;
        
        try {
            auto videoStitcher = std::make_shared<ins::VideoStitcher>();
            
            std::vector<std::string> inputs = { job.inputPath };
            videoStitcher->SetInputPath(inputs);
            videoStitcher->SetOutputPath(job.outputPath);
            
            // Configure for NAS environment
            videoStitcher->EnableCuda(enableGPU);
            videoStitcher->EnableFlowState(true);
            videoStitcher->EnableDirectionLock(true);
            videoStitcher->EnableH265Encoder();
            videoStitcher->SetOutputBitRate(bitrate);
            videoStitcher->SetOutputSize(outputWidth, outputHeight);
            videoStitcher->SetStitchType(ins::STITCH_TYPE::TEMPLATE); // Use template for reliability
            
            // Set up progress callback
            videoStitcher->SetStitchProgressCallback([](int progress, int error) {
                if (error != 0) {
                    std::cerr << "Stitching error: " << error << std::endl;
                } else {
                    std::cout << "Progress: " << progress << "%" << std::endl;
                }
            });
            
            videoStitcher->StartStitch();
            
            if (fs::exists(job.outputPath)) {
                std::cout << "Video conversion completed: " << fs::path(job.outputPath).filename() << std::endl;
                return true;
            } else {
                std::cerr << "Video conversion failed - output file not created" << std::endl;
                return false;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error processing video: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool processImage(const ConversionJob& job) {
        std::cout << "Processing image: " << fs::path(job.inputPath).filename() << std::endl;
        
        try {
            auto imageStitcher = std::make_shared<ins::ImageStitcher>();
            
            std::vector<std::string> inputs = { job.inputPath };
            imageStitcher->SetInputPath(inputs);
            imageStitcher->SetOutputPath(job.outputPath);
            
            // Configure for NAS environment
            imageStitcher->EnableCuda(enableGPU);
            imageStitcher->SetImageProcessingAccelType(ins::ImageProcessingAccel::kCPU);
            imageStitcher->SetStitchType(ins::STITCH_TYPE::TEMPLATE);
            imageStitcher->EnableStitchFusion(false); // Disable to avoid OpenCV issues
            imageStitcher->SetOutputSize(outputWidth, outputHeight);
            
            bool success = imageStitcher->Stitch();
            
            if (success && fs::exists(job.outputPath)) {
                std::cout << "Image conversion completed: " << fs::path(job.outputPath).filename() << std::endl;
                
                // Add 360° EXIF metadata to make the image recognizable as a panorama
                std::cout << "Adding 360° EXIF metadata..." << std::endl;
                if (add360ExifMetadata(job.outputPath, job.inputPath, outputWidth, outputHeight)) {
                    std::cout << "Successfully added 360° EXIF metadata to " << fs::path(job.outputPath).filename() << std::endl;
                } else {
                    std::cerr << "Warning: Failed to add 360° EXIF metadata to " << fs::path(job.outputPath).filename() << std::endl;
                }
                
                return true;
            } else {
                std::cerr << "Image conversion failed" << std::endl;
                return false;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error processing image: " << e.what() << std::endl;
            return false;
        }
    }
    
    // markAsProcessed function removed - we now detect processed files by checking output directory
    
    void processJobs() {
        while (running) {
            ConversionJob job;
            bool hasJob = false;
            
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                if (!jobQueue.empty()) {
                    job = jobQueue.front();
                    jobQueue.pop();
                    hasJob = true;
                }
            }
            
            if (hasJob) {
                bool success = false;
                
                if (job.fileType == ".insv") {
                    success = processVideo(job);
                } else if (job.fileType == ".insp") {
                    success = processImage(job);
                }
                
                if (success) {
                    std::cout << "Job completed successfully: " << fs::path(job.inputPath).filename() << std::endl;
                } else {
                    std::cerr << "Job failed: " << fs::path(job.inputPath).filename() << std::endl;
                }
            } else {
                // No jobs, wait a bit
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    
    void start() {
        running = true;
        
        if (watchMode) {
            std::cout << "Starting batch processor in WATCH MODE (continuous monitoring)..." << std::endl;
            std::cout << "The processor will continuously monitor for new files and convert them automatically." << std::endl;
            std::cout << "Press Ctrl+C to stop." << std::endl;
        } else {
            std::cout << "Starting batch processor in SINGLE RUN MODE..." << std::endl;
            std::cout << "The processor will scan once, convert all found files, and exit." << std::endl;
        }
        
        // Start worker thread
        std::thread worker(&Insta360BatchProcessor::processJobs, this);
        
        if (watchMode) {
            // Watch mode: continuous monitoring
            while (running) {
                scanForFiles();
                
                // Display queue status
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    if (!jobQueue.empty()) {
                        std::cout << "Jobs in queue: " << jobQueue.size() << std::endl;
                    }
                }
                
                // Wait before next scan
                std::this_thread::sleep_for(std::chrono::seconds(watchInterval));
            }
        } else {
            // Single run mode: scan once and wait for completion
            scanForFiles();
            
            // Display initial queue status
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                std::cout << "Jobs in queue: " << jobQueue.size() << std::endl;
            }
            
            // Wait for all jobs to complete
            bool allJobsCompleted = false;
            while (!allJobsCompleted && running) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                
                std::lock_guard<std::mutex> lock(queueMutex);
                allJobsCompleted = jobQueue.empty();
            }
            
            // Stop the processor after completion
            running = false;
            std::cout << "All jobs completed. Exiting single run mode." << std::endl;
        }
        
        worker.join();
    }
    
    void stop() {
        running = false;
        std::cout << "Stopping batch processor..." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_dir> <output_dir> [config_file] [--watch]" << std::endl;
        std::cerr << "Example (single run): " << argv[0] << " /data/input /data/output /data/config.json" << std::endl;
        std::cerr << "Example (watch mode): " << argv[0] << " /data/input /data/output /data/config.json --watch" << std::endl;
        std::cerr << "" << std::endl;
        std::cerr << "Modes:" << std::endl;
        std::cerr << "  Single run (default): Process all files once and exit" << std::endl;
        std::cerr << "  Watch mode (--watch): Continuously monitor for new files" << std::endl;
        std::cerr << "Note: Converted files detection is done by checking the output directory" << std::endl;
        return 1;
    }
    
    std::string inputDir = argv[1];
    std::string outputDir = argv[2];
    std::string configFile = argc > 3 ? argv[3] : "/data/config.json";
    
    // Check for --watch parameter
    bool forceWatchMode = false;
    for (int i = 3; i < argc; i++) {
        if (std::string(argv[i]) == "--watch") {
            forceWatchMode = true;
            break;
        }
    }
    
    std::cout << "Insta360 Batch Processor for Synology NAS" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "Detection method: Output directory comparison (no processed folder duplication)" << std::endl;
    
    try {
        Insta360BatchProcessor processor(inputDir, outputDir, configFile);
        
        // Override watch mode if specified via command line
        if (forceWatchMode) {
            processor.setWatchMode(true);
        }
        
        // Handle shutdown gracefully
        processor.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
