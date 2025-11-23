#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>

namespace GreyScott {
    class ComputeManager;
    class Renderer;
    class Simulation;

    class Application {
    public:
        struct Config {
            const std::string windowTitle{ "Grey-Scott Simulation" };
            constexpr static int windowWidth{ 1024 };
            constexpr static int windowHeight{ 1024 };
            constexpr static int gridWidth{ 512 };
            constexpr static int gridHeight{ 512 };
            constexpr static bool vsync{ true };
        };

        explicit Application(const Config& config);

        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool initialize();
        void run();
        void quit();

        const Config& getConfig() const { return m_config; }

    private:
        bool initSDL();
        bool initOpenGL();
        void handleEvents();
        void update(float deltaTime);
        void render();

        Config m_config{};
        SDL_Window* m_window{};
        SDL_GLContext m_glContext{};
        bool m_running{};
        bool m_initialized{};

        uint64_t m_lastFrameTime{};
        int m_frameCount{};
        float m_fpsTimer{};

        std::unique_ptr<ComputeManager> m_computeManager{};
        std::unique_ptr<Renderer> m_renderer{};
        std::unique_ptr<Simulation> m_simulation;
    };

} // namespace GreyScott
