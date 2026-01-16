// clang-format off
#include <GL/glew.h>
// Note: GL/gl.h is included by glew.h
#include "Application.hpp"
#ifdef USE_OPENCL
#include "ComputeManager.hpp"
#include "Simulation.hpp"
#endif
#include "Renderer.hpp"
#include "SimulationCPU.hpp"
#include "SimulationParams.hpp"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <algorithm>
// clang-format on

// Platform-specific OpenGL version
#ifdef __APPLE__
    #define GL_MAJOR_VERSION_REQUIRED 4
    #define GL_MINOR_VERSION_REQUIRED 1
    #define GLSL_VERSION_STRING "#version 410"
#else
    #define GL_MAJOR_VERSION_REQUIRED 4
    #define GL_MINOR_VERSION_REQUIRED 6
    #define GLSL_VERSION_STRING "#version 460"
#endif

namespace GreyScott {
    Application::Application(const Config& config) :
        m_config{ config },
        m_window{ nullptr },
        m_glContext{ nullptr },
        m_running{ false },
        m_initialized{ false },
        m_lastFrameTime{ 0 },
        m_frameCount{ 0 },
        m_fpsTimer{ 0.0f }
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

#ifdef USE_OPENCL
        m_computeManager = std::make_unique<ComputeManager>();
        if (!m_computeManager->initialize()) {
            std::cerr << "Failed to initialize compute manager!\n";
            return false;
        }
#endif

        m_renderer =
            std::make_unique<Renderer>(m_config.gridWidth, m_config.gridHeight);
        if (!m_renderer->initialize()) {
            std::cerr << "Failed to initialize renderer!\n";
            return false;
        }

#ifdef USE_OPENCL
        m_simulation = std::make_unique<Simulation>(
            m_config.gridWidth, m_config.gridHeight, m_computeManager.get());
        if (!m_simulation->initialize()) {
            std::cerr << "Failed to initialize simulation!\n";
            return false;
        }
#endif

        m_simulationCPU = std::make_unique<SimulationCPU>(
            m_config.gridWidth, m_config.gridHeight);
        m_simulationCPU->initialize();

#ifndef USE_OPENCL
        // Force CPU mode when OpenCL is not available
        m_useCPU = true;
        std::cout << "OpenCL not available - using CPU-only mode\n";
#endif

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io{ ImGui::GetIO() };
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
        ImGui_ImplOpenGL3_Init(GLSL_VERSION_STRING);

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

        // Set OpenGL attributes (platform-specific version)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GL_MAJOR_VERSION_REQUIRED);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GL_MINOR_VERSION_REQUIRED);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef __APPLE__
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

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
#ifdef USE_OPENCL
                    if (!m_useCPU && m_simulation) {
                        m_simulation->reset();
                    } else
#endif
                    if (m_simulationCPU) {
                        m_simulationCPU->reset();
                    }
                    std::cout << "Simulation reset\n";
                    break;
                case SDLK_SPACE:
                    m_paused = !m_paused;
                    std::cout << (m_paused ? "Paused\n" : "Resumed\n");
                    break;
                case SDLK_UP:
#ifdef USE_OPENCL
                    if (!m_useCPU && m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.F += 0.001f;
                        params.F = std::min(params.F, 0.1f);
                        m_simulation->setParams(params);
                        std::cout << "F = " << params.F << '\n';
                    } else
#endif
                    if (m_simulationCPU) {
                        auto params{ m_simulationCPU->getParams() };
                        params.F += 0.001f;
                        params.F = std::min(params.F, 0.1f);
                        m_simulationCPU->setParams(params);
                        std::cout << "F = " << params.F << '\n';
                    }
                    break;
                case SDLK_DOWN:
#ifdef USE_OPENCL
                    if (!m_useCPU && m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.F -= 0.001f;
                        params.F = std::max(params.F, 0.0f);
                        m_simulation->setParams(params);
                        std::cout << "F = " << params.F << '\n';
                    } else
#endif
                    if (m_simulationCPU) {
                        auto params{ m_simulationCPU->getParams() };
                        params.F -= 0.001f;
                        params.F = std::max(params.F, 0.0f);
                        m_simulationCPU->setParams(params);
                        std::cout << "F = " << params.F << '\n';
                    }
                    break;
                case SDLK_RIGHT:
#ifdef USE_OPENCL
                    if (!m_useCPU && m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.k += 0.001f;
                        params.k = std::min(params.k, 0.1f);
                        m_simulation->setParams(params);
                        std::cout << "k = " << params.k << '\n';
                    } else
#endif
                    if (m_simulationCPU) {
                        auto params{ m_simulationCPU->getParams() };
                        params.k += 0.001f;
                        params.k = std::min(params.k, 0.1f);
                        m_simulationCPU->setParams(params);
                        std::cout << "k = " << params.k << '\n';
                    }
                    break;
                case SDLK_LEFT:
#ifdef USE_OPENCL
                    if (!m_useCPU && m_simulation) {
                        auto params{ m_simulation->getParams() };
                        params.k -= 0.001f;
                        params.k = std::max(params.k, 0.0f);
                        m_simulation->setParams(params);
                        std::cout << "k = " << params.k << '\n';
                    } else
#endif
                    if (m_simulationCPU) {
                        auto params{ m_simulationCPU->getParams() };
                        params.k -= 0.001f;
                        params.k = std::max(params.k, 0.0f);
                        m_simulationCPU->setParams(params);
                        std::cout << "k = " << params.k << '\n';
                    }
                    break;
                case SDLK_F1:
                case SDLK_F2:
                case SDLK_F3:
                case SDLK_F4:
                case SDLK_F5: {
                    int preset = event.key.keysym.sym - SDLK_F1 + 1;
#ifdef USE_OPENCL
                    if (!m_useCPU && m_simulation) {
                        m_simulation->loadPreset(preset);
                    } else
#endif
                    if (m_simulationCPU) {
                        m_simulationCPU->loadPreset(preset);
                    }
                    const char* presetNames[] = { "", "Spots", "Stripes", "Waves", "Chaos", "Holes" };
                    std::cout << "Loaded preset " << preset << ": " << presetNames[preset] << '\n';
                    break;
                }
                case SDLK_c:
#ifdef USE_OPENCL
                    if (m_useCPU) {
                        m_simulation->syncFrom(m_simulationCPU->getData());
                    } else {
                        m_simulationCPU->syncFrom(m_simulation->getData());
                    }
                    m_useCPU = !m_useCPU;
                    std::cout << "Switched to " << (m_useCPU ? "CPU" : "GPU") << " mode\n";
#else
                    std::cout << "OpenCL not available - CPU-only mode\n";
#endif
                    break;
                default: break;
                }
                break;

            default: break;
            }
        }
    }

    void Application::update(float deltaTime) {
        if (!m_paused) {
#ifdef USE_OPENCL
            if (!m_useCPU && m_simulation) {
                m_simulation->step();
                m_computeTimeMs = m_simulation->getLastComputeTime();
            } else
#endif
            if (m_simulationCPU) {
                m_simulationCPU->step(m_simulationCPU->getParams());
                m_computeTimeMs = m_simulationCPU->getLastComputeTime();
            }

            m_avgComputeTimeMs = (m_avgComputeTimeMs * m_computeSamples + m_computeTimeMs) / (m_computeSamples + 1);
            ++m_computeSamples;
            if (m_computeSamples > 100) {
                m_computeSamples = 50;
                m_avgComputeTimeMs = m_computeTimeMs;
            }
        }

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

        if (m_renderer) {
            const float* data;
#ifdef USE_OPENCL
            data = m_useCPU ? m_simulationCPU->getData() : m_simulation->getData();
#else
            data = m_simulationCPU->getData();
#endif
            m_renderer->updateTexture(data);
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

        SimulationParams params;
#ifdef USE_OPENCL
        if (!m_useCPU && m_simulation) {
            params = m_simulation->getParams();
        } else
#endif
        if (m_simulationCPU) {
            params = m_simulationCPU->getParams();
        }

        ImGui::Text("Feed Rate (F): %.4f", params.F);
        ImGui::Text("Kill Rate (k): %.4f", params.k);
        ImGui::Text("Diffusion U: %.4f", params.Du);
        ImGui::Text("Diffusion V: %.4f", params.Dv);
        ImGui::Separator();

#ifdef USE_OPENCL
        ImGui::Text("Implementation: %s", m_useCPU ? "CPU (Serial)" : "GPU (OpenCL)");
#else
        ImGui::Text("Implementation: CPU (Serial)");
#endif
        ImGui::Text("Compute Time: %.3f ms", m_avgComputeTimeMs);
        ImGui::Text("Compute FPS: %.1f", 1000.0f / m_avgComputeTimeMs);
        ImGui::Separator();

        ImGui::Text("Status: %s", m_paused ? "PAUSED" : "Running");
        ImGui::Separator();

        ImGui::Text("Controls:");
        ImGui::BulletText("Space: Pause/Resume");
        ImGui::BulletText("R: Reset");
        ImGui::BulletText("Up/Down: Adjust F");
        ImGui::BulletText("Left/Right: Adjust k");
        ImGui::BulletText("F1-F5: Load Presets");
#ifdef USE_OPENCL
        ImGui::BulletText("C: Toggle CPU/GPU");
#endif
        ImGui::BulletText("ESC: Quit");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Application::quit() { m_running = false; }

} // namespace GreyScott
