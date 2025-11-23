// clang-format off
#include <GL/glew.h>
#include <GL/gl.h>
#include "Application.hpp"
#include "ComputeManager.hpp"
#include "Renderer.hpp"
#include "Simulation.hpp"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <algorithm>
// clang-format on

namespace GreyScott {

    Application::Application(const Config& config) :
        m_config{ config },
        m_window{ nullptr },
        m_glContext{ nullptr },
        m_running{ false },
        m_initialized{ false },
        m_lastFrameTime{},
        m_frameCount{},
        m_fpsTimer{}
        {}

    Application::~Application() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        if (m_glContext) { SDL_GL_DeleteContext(m_glContext); }
        if (m_window) { SDL_DestroyWindow(m_window); }
        SDL_Quit();
    }

    bool Application::initialize() {
        if (m_initialized) {
            std::cerr << "Application already initialized!\n";
            return false;
        }

        if (!initSDL()) { return false; }
        if (!initOpenGL()) { return false; }

        m_computeManager = std::make_unique<ComputeManager>();
        if (!m_computeManager->initialize()) {
            std::cerr << "Failed to initialize compute manager!\n";
            return false;
        }

        m_renderer =
            std::make_unique<Renderer>(m_config.gridWidth, m_config.gridHeight);
        if (!m_renderer->initialize()) {
            std::cerr << "Failed to initialize renderer!\n";
            return false;
        }

        m_simulation = std::make_unique<Simulation>(
            m_config.gridWidth, m_config.gridHeight, m_computeManager.get());
        if (!m_simulation->initialize()) {
            std::cerr << "Failed to initialize simulation!\n";
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io{ ImGui::GetIO() };
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
        ImGui_ImplOpenGL3_Init("#version 460");

        std::cout << "Application initialized successfully\n";
        std::cout << "  Window: " << m_config.windowWidth << "x"
                  << m_config.windowHeight << '\n';
        std::cout << "  Grid: " << m_config.gridWidth << "x"
                  << m_config.gridHeight << '\n';

        m_initialized = true;
        return true;
    }

    bool Application::initSDL() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
            return false;
        }

        // Set OpenGL attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        // Create window
        m_window = SDL_CreateWindow(
            m_config.windowTitle.c_str(), SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, m_config.windowWidth, m_config.windowHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        if (!m_window) {
            std::cerr << "Failed to create window: " << SDL_GetError() << '\n';
            return false;
        }

        std::cout << "SDL initialized successfully\n";
        return true;
    }

    bool Application::initOpenGL() {
        // Create OpenGL context
        m_glContext = SDL_GL_CreateContext(m_window);
        if (!m_glContext) {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError()
                      << '\n';
            return false;
        }

        // Initialize GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW: "
                      << glewGetErrorString(glewError) << '\n';
            return false;
        }

        // Set VSync
        if (SDL_GL_SetSwapInterval(m_config.vsync ? 1 : 0) < 0) {
            std::cerr << "Warning: Unable to set VSync: " << SDL_GetError()
                      << '\n';
        }

        // Query OpenGL information
        const GLubyte* renderer{ glGetString(GL_RENDERER) };
        const GLubyte* version{ glGetString(GL_VERSION) };
        const GLubyte* glslVersion{ glGetString(GL_SHADING_LANGUAGE_VERSION) };

        std::cout << "OpenGL initialized successfully\n";
        std::cout << "  Renderer: " << renderer << '\n';
        std::cout << "  Version: " << version << '\n';
        std::cout << "  GLSL Version: " << glslVersion << '\n';

        // Set initial OpenGL state
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glViewport(0, 0, m_config.windowWidth, m_config.windowHeight);

        return true;
    }

    void Application::run() {
        if (!m_initialized) {
            std::cerr << "Cannot run: Application not initialized!\n";
            return;
        }

        m_running = true;
        m_lastFrameTime = SDL_GetPerformanceCounter();

        std::cout << "Starting main loop...\n";

        while (m_running) {
            // Calculate delta time
            uint64_t currentTime{ SDL_GetPerformanceCounter() };
            float deltaTime{ (currentTime - m_lastFrameTime) /
                              (float)SDL_GetPerformanceFrequency() };
            m_lastFrameTime = currentTime;

            handleEvents();
            update(deltaTime);
            render();

            // Swap buffers
            SDL_GL_SwapWindow(m_window);
        }

        std::cout << "Main loop ended\n";
    }

    void Application::handleEvents() {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            switch (event.type) {
            case SDL_QUIT: quit(); break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: quit(); break;
                case SDLK_r:
                    if (m_simulation) {
                        m_simulation->reset();
                        std::cout << "Simulation reset\n";
                    }
                    break;
                case SDLK_SPACE:
                    m_paused = !m_paused;
                    std::cout << (m_paused ? "Paused\n" : "Resumed\n");
                    break;
                case SDLK_UP:
                    if (m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.F += 0.001f;
                        params.F = std::min(params.F, 0.1f);
                        m_simulation->setParams(params);
                        std::cout << "F = " << params.F << '\n';
                    }
                    break;
                case SDLK_DOWN:
                    if (m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.F -= 0.001f;
                        params.F = std::max(params.F, 0.0f);
                        m_simulation->setParams(params);
                        std::cout << "F = " << params.F << '\n';
                    }
                    break;
                case SDLK_RIGHT:
                    if (m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.k += 0.001f;
                        params.k = std::min(params.k, 0.1f);
                        m_simulation->setParams(params);
                        std::cout << "k = " << params.k << '\n';
                    }
                    break;
                case SDLK_LEFT:
                    if (m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.k -= 0.001f;
                        params.k = std::max(params.k, 0.0f);
                        m_simulation->setParams(params);
                        std::cout << "k = " << params.k << '\n';
                    }
                    break;
                case SDLK_F1:
                    if (m_simulation) {
                        m_simulation->loadPreset(1);
                        std::cout << "Loaded preset 1: Spots\n";
                    }
                    break;
                case SDLK_F2:
                    if (m_simulation) {
                        m_simulation->loadPreset(2);
                        std::cout << "Loaded preset 2: Stripes\n";
                    }
                    break;
                case SDLK_F3:
                    if (m_simulation) {
                        m_simulation->loadPreset(3);
                        std::cout << "Loaded preset 3: Waves\n";
                    }
                    break;
                case SDLK_F4:
                    if (m_simulation) {
                        m_simulation->loadPreset(4);
                        std::cout << "Loaded preset 4: Chaos\n";
                    }
                    break;
                case SDLK_F5:
                    if (m_simulation) {
                        m_simulation->loadPreset(5);
                        std::cout << "Loaded preset 5: Holes\n";
                    }
                    break;
                default: break;
                }
                break;

            default: break;
            }
        }
    }

    void Application::update(float deltaTime) {
        if (m_simulation && !m_paused) { m_simulation->step(); }

        ++m_frameCount;
        m_fpsTimer += deltaTime;

        if (m_fpsTimer >= 1.0f) {
            m_currentFps = m_frameCount;
            std::cout << "FPS: " << m_frameCount << '\n';
            m_frameCount = 0;
            m_fpsTimer = 0.0f;
        }
    }

    void Application::render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_renderer && m_simulation) {
            m_renderer->updateTexture(m_simulation->getData());
            m_renderer->render();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        ImGui::Begin("Simulation Info", nullptr, ImGuiWindowFlags_NoCollapse);

        ImGui::Text("FPS: %d", m_currentFps);
        ImGui::Separator();

        if (m_simulation) {
            auto params = m_simulation->getParams();
            ImGui::Text("Feed Rate (F): %.4f", params.F);
            ImGui::Text("Kill Rate (k): %.4f", params.k);
            ImGui::Text("Diffusion U: %.4f", params.Du);
            ImGui::Text("Diffusion V: %.4f", params.Dv);
            ImGui::Separator();
        }

        ImGui::Text("Status: %s", m_paused ? "PAUSED" : "Running");
        ImGui::Separator();

        ImGui::Text("Controls:");
        ImGui::BulletText("Space: Pause/Resume");
        ImGui::BulletText("R: Reset");
        ImGui::BulletText("Up/Down: Adjust F");
        ImGui::BulletText("Left/Right: Adjust k");
        ImGui::BulletText("F1-F5: Load Presets");
        ImGui::BulletText("ESC: Quit");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Application::quit() { m_running = false; }

} // namespace GreyScott
