#include "Application.hpp"
#include "ComputeManager.hpp"
#include <iostream>

/**
 * @brief Grey-Scott Reaction-Diffusion Simulation
 *
 * A cellular automata simulation using the Grey-Scott model.
 *
 * @author Lothiard
 * @date November 23, 2025
 */

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "==================================" << std::endl;
    std::cout << "Grey-Scott Simulation v0.1.0" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;

    // Create compute manager and query available devices
    GreyScott::ComputeManager computeManager;
    computeManager.printDeviceInfo();

    // Initialize OpenCL
    if (!computeManager.initialize()) {
        std::cerr << "Failed to initialize OpenCL!" << std::endl;
        return 1;
    }
    std::cout << std::endl;

    // Configure application
    GreyScott::Application::Config config;
    config.windowTitle = "Grey-Scott Simulation";
    config.windowWidth = 1024;
    config.windowHeight = 1024;
    config.gridWidth = 512;
    config.gridHeight = 512;
    config.vsync = true;

    // Create and initialize application
    GreyScott::Application app(config);
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return 1;
    }
    std::cout << std::endl;

    std::cout << "Initialization complete! Press ESC to quit." << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;

    // Run main loop
    app.run();

    std::cout << std::endl;
    std::cout << "Shutting down..." << std::endl;
    std::cout << "==================================" << std::endl;

    return 0;
}
