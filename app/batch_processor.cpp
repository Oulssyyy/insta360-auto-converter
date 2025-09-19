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
    std::string processedDir;
    std::string configFile;
    std::queue<ConversionJob> jobQueue;
    std::mutex queueMutex;
    bool running;
    
    // Configuration
    bool enableGPU = false;
    int outputWidth = 5760;
    int outputHeight = 2880;
    int bitrate = 50000000; // 50 Mbps
    int maxConcurrentJobs = 1;
    int watchInterval = 30; // seconds
    
public:
    Insta360BatchProcessor(const std::string& input, const std::string& output, const std::string& processed, const std::string& config) 
        : inputDir(input), outputDir(output), processedDir(processed), configFile(config), running(false) {
        
        // Ensure directories exist
        fs::create_directories(outputDir);
        fs::create_directories(processedDir);
        
        // Load configuration
        loadConfiguration();
        
        // Initialize SDK
        ins::InitEnv();
        ins::SetLogLevel(ins::InsLogLevel::INFO);
        
        std::cout << "Insta360 Batch Processor initialized" << std::endl;
        std::cout << "Input directory: " << inputDir << std::endl;
        std::cout << "Output directory: " << outputDir << std::endl;
        std::cout << "Processed directory: " << processedDir << std::endl;
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
            
            std::cout << "Configuration loaded from: " << configFile << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error loading configuration: " << e.what() << std::endl;
            std::cerr << "Using default configuration" << std::endl;
        }
    }
    
    void createDefaultConfig() {
        Json::Value config;
        config["enableGPU"] = false;
        config["outputWidth"] = 5760;
        config["outputHeight"] = 2880;
        config["bitrate"] = 50000000;
        config["maxConcurrentJobs"] = 1;
        config["watchInterval"] = 30;
        config["comment"] = "Insta360 Batch Processor Configuration";
        
        std::ofstream file(configFile);
        file << config;
        file.close();
        
        std::cout << "Default configuration created: " << configFile << std::endl;
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
                    // Check if already processed
                    std::string relativePath = fs::relative(entry.path(), inputDir).string();
                    std::string processedPath = (fs::path(processedDir) / relativePath).string();
                    
                    if (fs::exists(processedPath)) {
                        continue; // Already processed
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
    
    void markAsProcessed(const ConversionJob& job) {
        try {
            // Create marker file in processed directory
            std::string relativePath = fs::relative(job.inputPath, inputDir).string();
            std::string processedPath = (fs::path(processedDir) / relativePath).string();
            
            // Create directory structure
            fs::create_directories(fs::path(processedPath).parent_path());
            
            // Create marker file with metadata
            std::ofstream markerFile(processedPath);
            markerFile << "Processed: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
            markerFile << "Input: " << job.inputPath << std::endl;
            markerFile << "Output: " << job.outputPath << std::endl;
            markerFile.close();
            
        } catch (const std::exception& e) {
            std::cerr << "Error marking as processed: " << e.what() << std::endl;
        }
    }
    
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
                    markAsProcessed(job);
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
        std::cout << "Starting batch processor..." << std::endl;
        
        // Start worker thread
        std::thread worker(&Insta360BatchProcessor::processJobs, this);
        
        // Main loop - scan for new files periodically
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
        
        worker.join();
    }
    
    void stop() {
        running = false;
        std::cout << "Stopping batch processor..." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input_dir> <output_dir> <processed_dir> [config_file]" << std::endl;
        std::cerr << "Example: " << argv[0] << " /data/input /data/output /data/processed /data/config.json" << std::endl;
        return 1;
    }
    
    std::string inputDir = argv[1];
    std::string outputDir = argv[2];
    std::string processedDir = argv[3];
    std::string configFile = argc > 4 ? argv[4] : "/data/config.json";
    
    std::cout << "Insta360 Batch Processor for Synology NAS" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    try {
        Insta360BatchProcessor processor(inputDir, outputDir, processedDir, configFile);
        
        // Handle shutdown gracefully
        processor.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
