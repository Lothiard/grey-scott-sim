#pragma once

#include "SimulationParams.hpp"
#include <vector>

namespace GreyScott {
    class SimulationCPU {
    public:
        SimulationCPU(int width, int height);
        ~SimulationCPU() = default;

        SimulationCPU(const SimulationCPU&) = delete;
        SimulationCPU& operator=(const SimulationCPU&) = delete;

        void initialize();
        void step(const SimulationParams& params);
        void reset();
        void syncFrom(const float* data);

        const float* getData() const { return m_data.data(); }
        const SimulationParams& getParams() const { return m_params; }
        void setParams(const SimulationParams& params) { m_params = params; }
        void loadPreset(int presetIndex);
        float getLastComputeTime() const { return m_lastComputeTime; }
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }

    private:
        void initializeState();
        float computeLaplacian(const std::vector<float>& field, int x, int y, int component);

        int m_width{};
        int m_height{};
        std::vector<float> m_data{};
        std::vector<float> m_dataNext{};
        SimulationParams m_params{};
        float m_lastComputeTime{};
    };

} // namespace GreyScott
