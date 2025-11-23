#include "Application.hpp"
#include <GL/gl.h>
#include <GL/glew.h>
#include <iostream>

namespace GreyScott {

    Application::Application(const Config& config) :
        m_config(config),
        m_window(nullptr),
        m_glContext(nullptr),
        m_running(false),
        m_initialized(false),
        m_lastFrameTime(0) {}

    Application::~Application() {
        if (m_glContext) { SDL_GL_DeleteContext(m_glContext); }
        if (m_window) { SDL_DestroyWindow(m_window); }
        SDL_Quit();
    }

    bool Application::initialize() {
        if (m_initialized) {
            std::cerr << "Application already initialized!" << std::endl;
            return false;
        }

        if (!initSDL()) { return false; }

        if (!initOpenGL()) { return false; }

        std::cout << "Application initialized successfully" << std::endl;
        std::cout << "  Window: " << m_config.windowWidth << "x"
                  << m_config.windowHeight << std::endl;
        std::cout << "  Grid: " << m_config.gridWidth << "x"
                  << m_config.gridHeight << std::endl;

        m_initialized = true;
        return true;
    }

    bool Application::initSDL() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError()
                      << std::endl;
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
            std::cerr << "Failed to create window: " << SDL_GetError()
                      << std::endl;
            return false;
        }

        std::cout << "SDL initialized successfully" << std::endl;
        return true;
    }

    bool Application::initOpenGL() {
        // Create OpenGL context
        m_glContext = SDL_GL_CreateContext(m_window);
        if (!m_glContext) {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError()
                      << std::endl;
            return false;
        }

        // Initialize GLEW
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW: "
                      << glewGetErrorString(glewError) << std::endl;
            return false;
        }

        // Set VSync
        if (SDL_GL_SetSwapInterval(m_config.vsync ? 1 : 0) < 0) {
            std::cerr << "Warning: Unable to set VSync: " << SDL_GetError()
                      << std::endl;
        }

        // Query OpenGL information
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

        std::cout << "OpenGL initialized successfully" << std::endl;
        std::cout << "  Renderer: " << renderer << std::endl;
        std::cout << "  Version: " << version << std::endl;
        std::cout << "  GLSL Version: " << glslVersion << std::endl;

        // Set initial OpenGL state
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glViewport(0, 0, m_config.windowWidth, m_config.windowHeight);

        return true;
    }

    void Application::run() {
        if (!m_initialized) {
            std::cerr << "Cannot run: Application not initialized!"
                      << std::endl;
            return;
        }

        m_running = true;
        m_lastFrameTime = SDL_GetPerformanceCounter();

        std::cout << "Starting main loop..." << std::endl;

        while (m_running) {
            // Calculate delta time
            uint64_t currentTime = SDL_GetPerformanceCounter();
            float deltaTime = (currentTime - m_lastFrameTime) /
                              (float)SDL_GetPerformanceFrequency();
            m_lastFrameTime = currentTime;

            handleEvents();
            update(deltaTime);
            render();

            // Swap buffers
            SDL_GL_SwapWindow(m_window);
        }

        std::cout << "Main loop ended" << std::endl;
    }

    void Application::handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: quit(); break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) { quit(); }
                break;

            default: break;
            }
        }
    }

    void Application::update(float deltaTime) {
        // TODO: Update simulation
        // This will be implemented in later milestones
        (void)deltaTime; // Suppress unused parameter warning
    }

    void Application::render() {
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: Render simulation
        // This will be implemented in later milestones
    }

    void Application::quit() { m_running = false; }

} // namespace GreyScott
