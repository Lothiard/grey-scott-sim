#pragma once

#include <GL/gl.h>
#include <GL/glew.h>

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
        /**
         * @brief Construct a renderer for a specific grid size
         * @param width Grid width in cells
         * @param height Grid height in cells
         */
        Renderer(int width, int height);

        /**
         * @brief Destructor - cleans up OpenGL resources
         */
        ~Renderer();

        // Delete copy constructor and assignment operator
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        /**
         * @brief Initialize OpenGL resources (textures, shaders, buffers)
         * @return true on success
         */
        bool initialize();

        /**
         * @brief Update the texture with new simulation data
         * @param data Pointer to float array (width * height * 2 for U and V)
         */
        void updateTexture(const float* data);

        /**
         * @brief Render the current texture to the screen
         */
        void render();

        /**
         * @brief Get the OpenGL texture ID (useful for OpenCL-OpenGL interop
         * later)
         */
        GLuint getTextureID() const { return m_texture; }

    private:
        /**
         * @brief Create and compile shader program
         * @return true on success
         */
        bool createShaders();

        /**
         * @brief Create the texture for simulation data
         */
        bool createTexture();

        /**
         * @brief Create the quad geometry for rendering
         */
        bool createQuad();

        int m_width;
        int m_height;
        bool m_initialized;

        // OpenGL resources
        GLuint m_texture;
        GLuint m_vao; // Vertex Array Object
        GLuint m_vbo; // Vertex Buffer Object
        GLuint m_shaderProgram;
        GLuint m_vertexShader;
        GLuint m_fragmentShader;
    };

} // namespace GreyScott
