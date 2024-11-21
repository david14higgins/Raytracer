#include <iostream>
#include <chrono>
#include "../IO/IO.h"


int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <scene_file.json>" << std::endl;
            std::cout << "Please provide a path to a scene file." << std::endl;
            return 0;
        }

        std::string filename = argv[1];
        bool useBVH = false;  // You can add this as a command line argument later if needed
        
        Renderer renderer = IO::loadRendererDetails(filename, useBVH);
        
        auto start = std::chrono::high_resolution_clock::now();
        renderer.renderScene();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Render time: " << duration.count() << " milliseconds" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 1;
    }
}