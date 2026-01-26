#ifdef USE_OPENCL

#include "Simulation.hpp"
#include <iostream>
#include <random>

#include <GL/glew.h>

#ifndef __APPLE__
    #include <CL/cl_gl.h>
#endif

namespace GreyScott {
    Simulation::Simulation(int width, int height,
                           ComputeManager* computeManager) :
        m_width{ width },
        m_height{ height },
        m_computeManager{ computeManager },
        m_sharedTexture{ 0 },
        m_clImageCurrent{ nullptr },
        m_clImageNext{ nullptr },
        m_useGLInterop{ false },
        m_bufferCurrent{ nullptr },
        m_bufferNext{ nullptr },
        m_kernel{ nullptr },
        m_initialized{ false }
        {
            m_hostData.resize(width * height * 2);
        }

    Simulation::~Simulation() {
        if (m_kernel) clReleaseKernel(m_kernel);
        
        if (m_clImageCurrent) clReleaseMemObject(m_clImageCurrent);
        if (m_clImageNext) clReleaseMemObject(m_clImageNext);
        if (m_sharedTexture) glDeleteTextures(1, &m_sharedTexture);
        
        if (m_bufferCurrent) clReleaseMemObject(m_bufferCurrent);
        if (m_bufferNext) clReleaseMemObject(m_bufferNext);
    }

    bool Simulation::initialize() {
        if (!m_computeManager || !m_computeManager->isInitialized()) {
            std::cerr << "ComputeManager not initialized!\n";
            return false;
        }

        m_kernel = m_computeManager->loadKernel("kernels/grey_scott.cl",
                                                "grey_scott_step");
        if (!m_kernel) {
            std::cerr << "Failed to load Grey-Scott kernel!\n";
            return false;
        }

        createBuffers();
        initializeState();

        std::cout << "Simulation initialized\n";
        std::cout << "  Grid: " << m_width << "x" << m_height << '\n';
        std::cout << "  Parameters: F=" << m_params.F << ", k=" << m_params.k
                  << ", Du=" << m_params.Du << ", Dv=" << m_params.Dv << '\n';

        m_initialized = true;
        return true;
    }

    void Simulation::createBuffers() {
        cl_int err{};
        
        m_useGLInterop = m_computeManager->hasGLInterop();
        
#ifndef __APPLE__
        if (m_useGLInterop) {
            std::cout << "Creating GL-shared textures for zero-copy rendering\n";
            
            glGenTextures(1, &m_sharedTexture);
            glBindTexture(GL_TEXTURE_2D, m_sharedTexture);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, m_width, m_height, 0,
                        GL_RG, GL_FLOAT, nullptr);
            
            GLenum glErr = glGetError();
            if (glErr != GL_NO_ERROR) {
                std::cerr << "Failed to create GL texture! Error: " << glErr << '\n';
                m_useGLInterop = false;
                glDeleteTextures(1, &m_sharedTexture);
                m_sharedTexture = 0;
            } else {
                m_clImageCurrent = clCreateFromGLTexture(
                    m_computeManager->getContext(), CL_MEM_WRITE_ONLY,
                    GL_TEXTURE_2D, 0, m_sharedTexture, &err);
                
                if (err != CL_SUCCESS) {
                    std::cerr << "Failed to create CL image from GL texture! Error: " 
                              << err << '\n';
                    m_useGLInterop = false;
                    glDeleteTextures(1, &m_sharedTexture);
                    m_sharedTexture = 0;
                    m_clImageCurrent = nullptr;
                }
            }
            
             if (m_useGLInterop) {
                size_t bufferSize{ m_width * m_height * 2 * sizeof(float) };
                
                m_bufferCurrent = clCreateBuffer(m_computeManager->getContext(), 
                                                CL_MEM_READ_WRITE, bufferSize, 
                                                nullptr, &err);
                if (err != CL_SUCCESS) {
                    std::cerr << "Failed to create current buffer! Error: " << err << '\n';
                    m_useGLInterop = false;
                    if (m_clImageCurrent) {
                        clReleaseMemObject(m_clImageCurrent);
                        m_clImageCurrent = nullptr;
                    }
                    glDeleteTextures(1, &m_sharedTexture);
                    m_sharedTexture = 0;
                } else {
                    m_bufferNext = clCreateBuffer(m_computeManager->getContext(), 
                                                 CL_MEM_READ_WRITE, bufferSize, 
                                                 nullptr, &err);
                    if (err != CL_SUCCESS) {
                        std::cerr << "Failed to create next buffer! Error: " << err << '\n';
                        m_useGLInterop = false;
                        clReleaseMemObject(m_bufferCurrent);
                        m_bufferCurrent = nullptr;
                        if (m_clImageCurrent) {
                            clReleaseMemObject(m_clImageCurrent);
                            m_clImageCurrent = nullptr;
                        }
                        glDeleteTextures(1, &m_sharedTexture);
                        m_sharedTexture = 0;
                    } else {
                        std::cout << "GL-CL interop successfully initialized\n";
                    }
                }
            }
        }
#endif
        
        if (!m_useGLInterop) {
            std::cout << "Creating regular OpenCL buffers (with CPU transfers)\n";
            
            size_t bufferSize{ m_width * m_height * 2 * sizeof(float) };

            m_bufferCurrent =
                clCreateBuffer(m_computeManager->getContext(), CL_MEM_READ_WRITE,
                               bufferSize, nullptr, &err);
            if (err != CL_SUCCESS) {
                std::cerr << "Failed to create current buffer! Error: " << err
                          << '\n';
                return;
            }

            m_bufferNext =
                clCreateBuffer(m_computeManager->getContext(), CL_MEM_READ_WRITE,
                               bufferSize, nullptr, &err);
            if (err != CL_SUCCESS) {
                std::cerr << "Failed to create next buffer! Error: " << err << '\n';
                return;
            }
        }
    }

    void Simulation::initializeState() {
        std::random_device rd{};
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 0.01f);

        for (int i{}; i < m_width * m_height; ++i) {
            m_hostData[i * 2 + 0] = 1.0f;
            m_hostData[i * 2 + 1] = 0.0f;
        }

        int centerX{ m_width / 2 };
        int centerY{ m_height / 2 };
        int radius{ m_width / 10 };

        for (int y{ centerY - radius }; y < centerY + radius; ++y) {
            for (int x{ centerX - radius }; x < centerX + radius; ++x) {
                int dx{ x - centerX };
                int dy{ y - centerY };
                if (dx * dx + dy * dy < radius * radius) {
                    int idx{ (y * m_width + x) * 2 };
                    m_hostData[idx + 0] = 0.5f + dis(gen);
                    m_hostData[idx + 1] = 0.25f + dis(gen);
                }
            }
        }

        cl_int err = clEnqueueWriteBuffer(
            m_computeManager->getQueue(), m_bufferCurrent, CL_TRUE, 0,
            m_width * m_height * 2 * sizeof(float), m_hostData.data(), 0,
            nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to write initial state! Error: " << err
                      << '\n';
        }
    }

    void Simulation::step() {
        if (!m_initialized) return;

        cl_int err{};

        err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &m_bufferCurrent);
        err |= clSetKernelArg(m_kernel, 1, sizeof(cl_mem), &m_bufferNext);
        err |= clSetKernelArg(m_kernel, 2, sizeof(float), &m_params.Du);
        err |= clSetKernelArg(m_kernel, 3, sizeof(float), &m_params.Dv);
        err |= clSetKernelArg(m_kernel, 4, sizeof(float), &m_params.F);
        err |= clSetKernelArg(m_kernel, 5, sizeof(float), &m_params.k);
        err |= clSetKernelArg(m_kernel, 6, sizeof(float), &m_params.dt);
        err |= clSetKernelArg(m_kernel, 7, sizeof(int), &m_width);
        err |= clSetKernelArg(m_kernel, 8, sizeof(int), &m_height);

        if (err != CL_SUCCESS) {
            std::cerr << "Failed to set kernel arguments! Error: " << err
                      << '\n';
            return;
        }

        size_t globalSize[2]{ static_cast<size_t>(m_width), static_cast<size_t>(m_height) };
        cl_event event{};
        err = clEnqueueNDRangeKernel(m_computeManager->getQueue(), m_kernel, 2,
                                     nullptr, globalSize, nullptr, 0, nullptr,
                                     &event);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to enqueue kernel! Error: " << err << '\n';
            return;
        }

        clFinish(m_computeManager->getQueue());

#ifndef __APPLE__
        if (m_useGLInterop) {
            err = clEnqueueAcquireGLObjects(m_computeManager->getQueue(), 1,
                                           &m_clImageCurrent, 0, nullptr, nullptr);
            if (err != CL_SUCCESS) {
                std::cerr << "Failed to acquire GL objects! Error: " << err << '\n';
            } else {
                size_t origin[3] = {0, 0, 0};
                size_t region[3] = {static_cast<size_t>(m_width), 
                                   static_cast<size_t>(m_height), 1};
                
                err = clEnqueueCopyBufferToImage(m_computeManager->getQueue(),
                                                m_bufferNext, m_clImageCurrent,
                                                0, origin, region, 0, nullptr, nullptr);
                if (err != CL_SUCCESS) {
                    std::cerr << "Failed to copy buffer to GL texture! Error: " << err << '\n';
                }
                
                err = clEnqueueReleaseGLObjects(m_computeManager->getQueue(), 1,
                                               &m_clImageCurrent, 0, nullptr, nullptr);
                if (err != CL_SUCCESS) {
                    std::cerr << "Failed to release GL objects! Error: " << err << '\n';
                }
                
                clFinish(m_computeManager->getQueue());
            }
        }
#endif

        cl_ulong time_start, time_end;
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, nullptr);
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, nullptr);
        m_lastComputeTime = (time_end - time_start) / 1000000.0f;

        clReleaseEvent(event);

        std::swap(m_bufferCurrent, m_bufferNext);

        if (!m_useGLInterop) {
            readBackData();
        }
    }

    void Simulation::readBackData() {
        if (m_useGLInterop) return;
        
        cl_int err = clEnqueueReadBuffer(
            m_computeManager->getQueue(), m_bufferCurrent, CL_TRUE, 0,
            m_width * m_height * 2 * sizeof(float), m_hostData.data(), 0,
            nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to read back data! Error: " << err << '\n';
        }
    }

    void Simulation::reset() { initializeState(); }

    void Simulation::syncFrom(const float* data) {
        std::copy(data, data + m_width * m_height * 2, m_hostData.begin());

        cl_int err = clEnqueueWriteBuffer(
            m_computeManager->getQueue(), m_bufferCurrent, CL_TRUE, 0,
            m_width * m_height * 2 * sizeof(float), m_hostData.data(), 0,
            nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to sync data to GPU! Error: " << err << '\n';
        }
    }

    void Simulation::loadPreset(int presetIndex) {
        switch (presetIndex) {
        case 1:
            m_params.F = 0.055f;
            m_params.k = 0.062f;
            break;
        case 2:
            m_params.F = 0.039f;
            m_params.k = 0.058f;
            break;
        case 3:
            m_params.F = 0.026f;
            m_params.k = 0.051f;
            break;
        case 4:
            m_params.F = 0.018f;
            m_params.k = 0.051f;
            break;
        case 5:
            m_params.F = 0.014f;
            m_params.k = 0.047f;
            break;
        default:
            break;
        }
    }

} // namespace GreyScott

#endif // USE_OPENCL
