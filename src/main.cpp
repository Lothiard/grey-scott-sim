#include "Application.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "==================================\n";
    std::cout << "Grey-Scott Simulation\n";
    std::cout << "==================================\n\n";

    GreyScott::Application::Config config{};

    GreyScott::Application app(config);
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!\n";
        return 1;
    }
    std::cout << '\n';

    std::cout << "Initialization complete! Press ESC to quit.\n";
    std::cout << "==================================\n\n";

    app.run();

    std::cout << std::endl;
    std::cout << "Shutting down...\n";
    std::cout << "==================================\n";

    return 0;
}
