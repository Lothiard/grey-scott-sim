#pragma once

#ifdef USE_OPENCL

#include "ComputeManager.hpp"
#include "SimulationParams.hpp"
#include <vector>

namespace GreyScott {
    /**
     * @brief Manages the Grey-Scott reaction-diffusion simulation state and
     * computation
     *
     * This class handles:
     * - OpenCL buffer management for U and V concentrations
     * - Kernel execution for each simulation step
     * - Initial conditions and state reset
     * - Parameter management (F, k, Du, Dv, dt)
     * - Data readback for visualization
     */
    class Simulation {
    public:
        Simulation(int width, int height, ComputeManager* computeManager);
        ~Simulation();

        Simulation(const Simulation&) = delete;
        Simulation& operator=(const Simulation&) = delete;

        bool initialize();
        void step();
        void reset();
        void syncFrom(const float* data);
        void forceReadBack();

        const float* getData() const { return m_hostData.data(); }
        const SimulationParams& getParams() const { return m_params; }
        unsigned int getSharedTexture() const { return m_sharedTexture; }
        bool usesGLInterop() const { return m_useGLInterop; }

        void setParams(const SimulationParams& params) { m_params = params; }
        void loadPreset(int presetIndex);
        float getLastComputeTime() const { return m_lastComputeTime; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

    private:
        void initializeState();
        void createBuffers();
        void readBackData();

        int m_width{};
        int m_height{};
        ComputeManager* m_computeManager{};
        SimulationParams m_params{};

        unsigned int m_sharedTexture{};
        cl_mem m_clImageCurrent{};
        cl_mem m_clImageNext{};
        bool m_useGLInterop{};

        cl_mem m_bufferCurrent{};
        cl_mem m_bufferNext{};
        
        cl_kernel m_kernel{};

        std::vector<float> m_hostData{};
        bool m_initialized{};
        float m_lastComputeTime{};
    };

} // namespace GreyScott

#endif // USE_OPENCL
