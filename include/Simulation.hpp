#pragma once

#include "ComputeManager.hpp"
#include <vector>

namespace GreyScott {
    struct SimulationParams {
        float Du{ 0.16f };
        float Dv{ 0.08f };
        float F{ 0.055f };
        float k{ 0.062f };
        float dt{ 1.0f };
    };

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

        const float* getData() const { return m_hostData.data(); }
        const SimulationParams& getParams() const { return m_params; }

        void setParams(const SimulationParams& params) { m_params = params; }

        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

    private:
        void initializeState();
        void createBuffers();
        void readBackData();

        int m_width;
        int m_height;
        ComputeManager* m_computeManager;
        SimulationParams m_params;

        cl_mem m_bufferCurrent;
        cl_mem m_bufferNext;
        cl_kernel m_kernel;

        std::vector<float> m_hostData;
        bool m_initialized;
    };

} // namespace GreyScott
