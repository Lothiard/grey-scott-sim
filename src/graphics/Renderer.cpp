#include "Renderer.hpp"
#include <GL/glew.h>
#include <iostream>

namespace GreyScott {
    // Simple vertex shader for full-screen quad
    const char* vertexShaderSource = R"(
#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

    const char* fragmentShaderSource = R"(
#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

vec3 heatMap(float t) {
    vec3 purple = vec3(0.15, 0.0, 0.2);
    vec3 green = vec3(0.0, 0.6, 0.2);
    vec3 yellow = vec3(1.0, 0.95, 0.3);
    
    t = 1.0 - t;
    
    if (t < 0.5) {
        return mix(purple, green, t * 2.0);
    } else {
        return mix(green, yellow, (t - 0.5) * 2.0);
    }
}

void main() {
    float u = texture(uTexture, TexCoord).r;
    vec3 color = heatMap(u);
    FragColor = vec4(color, 1.0);
}
)";

    Renderer::Renderer(int width, int height) :
        m_width{ width },
        m_height{ height },
        m_initialized{},
        m_texture{},
        m_vao{},
        m_vbo{},
        m_shaderProgram{},
        m_vertexShader{},
        m_fragmentShader{}
        {}

    Renderer::~Renderer() {
        if (m_shaderProgram) { glDeleteProgram(m_shaderProgram); }
        if (m_vertexShader) { glDeleteShader(m_vertexShader); }
        if (m_fragmentShader) { glDeleteShader(m_fragmentShader); }
        if (m_vbo) { glDeleteBuffers(1, &m_vbo); }
        if (m_vao) { glDeleteVertexArrays(1, &m_vao); }
        if (m_texture) { glDeleteTextures(1, &m_texture); }
    }

    bool Renderer::initialize() {
        if (m_initialized) {
            std::cerr << "Renderer already initialized!\n";
            return false;
        }

        if (!createTexture()) { return false; }
        if (!createShaders()) { return false; }
        if (!createQuad()) { return false; }

        std::cout << "Renderer initialized successfully\n";
        std::cout << "  Grid size: " << m_width << "x" << m_height;

        m_initialized = true;
        return true;
    }

    bool Renderer::createTexture() {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Allocate texture memory (RG format for U and V concentrations)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, m_width, m_height, 0, GL_RG,
                     GL_FLOAT, nullptr);

        GLenum error{ glGetError() };
        if (error != GL_NO_ERROR) {
            std::cerr << "Failed to create texture! OpenGL error: " << error
                      << '\n';
            return false;
        }

        std::cout << "Created texture: " << m_width << "x" << m_height << '\n';
        return true;
    }

    bool Renderer::createShaders() {
        GLint success{};
        GLchar infoLog[512]{};

        // Compile vertex shader
        m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(m_vertexShader);

        glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_vertexShader, 512, nullptr, infoLog);
            std::cerr << "Vertex shader compilation failed:\n"
                      << infoLog << '\n';
            return false;
        }

        // Compile fragment shader
        m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(m_fragmentShader);

        glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_fragmentShader, 512, nullptr, infoLog);
            std::cerr << "Fragment shader compilation failed:\n"
                      << infoLog << '\n';
            return false;
        }

        // Link shader program
        m_shaderProgram = glCreateProgram();
        glAttachShader(m_shaderProgram, m_vertexShader);
        glAttachShader(m_shaderProgram, m_fragmentShader);
        glLinkProgram(m_shaderProgram);

        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
            std::cerr << "Shader program linking failed:\n"
                      << infoLog << '\n';
            return false;
        }

        std::cout << "Shaders compiled and linked successfully\n";
        return true;
    }

    bool Renderer::createQuad() {
        // Full-screen quad vertices (position + texture coordinates)
        float vertices[] = {
            // positions   // texCoords
            -1.0f, 1.0f,  0.0f, 1.0f, // top-left
            -1.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  -1.0f, 1.0f, 0.0f, // bottom-right

            -1.0f, 1.0f,  0.0f, 1.0f, // top-left
            1.0f,  -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f, 1.0f  // top-right
        };

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "Failed to create quad geometry! OpenGL error: "
                      << error << '\n';
            return false;
        }

        std::cout << "Created quad geometry\n";
        return true;
    }

    void Renderer::updateTexture(const float* data) {
        if (!m_initialized || !data) { return; }

        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RG,
                        GL_FLOAT, data);
    }

    void Renderer::render() {
        if (!m_initialized) { return; }

        // Use shader program
        glUseProgram(m_shaderProgram);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        // Set texture uniform (already 0 by default, but explicit is better)
        GLint texLocation = glGetUniformLocation(m_shaderProgram, "uTexture");
        glUniform1i(texLocation, 0);

        // Draw quad
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void Renderer::renderText(const char* text, float x, float y, float scale) {
        // Simple text rendering using window title for now
        // Full implementation would require font texture atlas
        (void)text;
        (void)x;
        (void)y;
        (void)scale;
    }

} // namespace GreyScott
