#include <filesystem>
#include <iostream>
#include <unordered_set>
#include "Insta360SDK.h"  // dépend du SDK fourni

namespace fs = std::filesystem;

int main() {
    std::string source = std::getenv("SOURCE_DIR") ? std::getenv("SOURCE_DIR") : "/data/input";
    std::string target = std::getenv("OUTPUT_DIR") ? std::getenv("OUTPUT_DIR") : "/data/output";

    std::unordered_set<std::string> converted;

    for (auto& f : fs::directory_iterator(target)) {
        converted.insert(f.path().stem().string());
    }

    for (auto& f : fs::directory_iterator(source)) {
        if (f.path().extension() == ".insv" || f.path().extension() == ".insp") {
            std::string base = f.path().stem().string();
            if (converted.find(base) == converted.end()) {
                std::cout << "Conversion de " << f.path() << "...\n";
                
                Insta360SDK::convert(f.path().string(), target + "/" + base + ".mp4");

                converted.insert(base);
            }
        }
    }

    return 0;
}
