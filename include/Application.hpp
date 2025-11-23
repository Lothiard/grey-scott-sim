#pragma once

#include <SDL2/SDL.h>
#include <string>

namespace GreyScott {

    /**
     * @brief Main application class managing window, event loop, and simulation
     * lifecycle
     *
     * Handles SDL initialization, window creation, OpenGL context setup, and
     * the main loop.
     */
    class Application {
    public:
        /**
         * @brief Configuration for the application window and simulation
         */
        struct Config {
            std::string windowTitle = "Grey-Scott Simulation";
            int windowWidth = 1024;
            int windowHeight = 1024;
            int gridWidth = 512;
            int gridHeight = 512;
            bool vsync = true;
        };

        /**
         * @brief Construct application with given configuration
         * @param config Application configuration
         */
        explicit Application(const Config& config);

        /**
         * @brief Destructor - cleans up SDL and OpenGL resources
         */
        ~Application();

        // Delete copy constructor and assignment operator
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /**
         * @brief Initialize SDL, OpenGL, and all subsystems
         * @return true if initialization succeeds, false otherwise
         */
        bool initialize();

        /**
         * @brief Run the main application loop
         */
        void run();

        /**
         * @brief Request application shutdown
         */
        void quit();

        /**
         * @brief Get the current configuration
         */
        const Config& getConfig() const { return m_config; }

    private:
        /**
         * @brief Initialize SDL and create window
         * @return true on success
         */
        bool initSDL();

        /**
         * @brief Initialize OpenGL context
         * @return true on success
         */
        bool initOpenGL();

        /**
         * @brief Process SDL events
         */
        void handleEvents();

        /**
         * @brief Update simulation state
         * @param deltaTime Time since last update in seconds
         */
        void update(float deltaTime);

        /**
         * @brief Render the current frame
         */
        void render();

        Config m_config;
        SDL_Window* m_window;
        SDL_GLContext m_glContext;
        bool m_running;
        bool m_initialized;

        // Timing
        uint64_t m_lastFrameTime;
    };

} // namespace GreyScott
