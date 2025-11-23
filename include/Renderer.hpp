#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

namespace GreyScott {
    /**
     * @brief Handles OpenGL rendering of the simulation grid
     *
     * This class manages:
     * - Texture creation for displaying simulation data
     * - Shader programs for rendering
     * - Quad geometry for full-screen display
     */
    class Renderer {
    public:
        Renderer(int width, int height);

        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        bool initialize();
        void updateTexture(const float* data);
        void render();
        void renderText(const char* text, float x, float y, float scale);
        GLuint getTextureID() const { return m_texture; }

    private:
        bool createShaders();
        bool createTexture();
        bool createQuad();

        int m_width{};
        int m_height{};
        bool m_initialized{};

        // OpenGL resources
        GLuint m_texture{};
        GLuint m_vao{}; // Vertex Array Object
        GLuint m_vbo{}; // Vertex Buffer Objec{}t
        GLuint m_shaderProgram{};
        GLuint m_vertexShader{};
        GLuint m_fragmentShader{};
    };

} // namespace GreyScott
