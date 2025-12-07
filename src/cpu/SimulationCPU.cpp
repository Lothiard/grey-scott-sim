#include "SimulationCPU.hpp"
#include "Simulation.hpp"
#include <algorithm>
#include <random>

namespace GreyScott {
    SimulationCPU::SimulationCPU(int width, int height) :
        m_width{ width },
        m_height{ height }
    {
        m_data.resize(width * height * 2);
        m_dataNext.resize(width * height * 2);
    }

    void SimulationCPU::initialize() {
        initializeState();
    }

    void SimulationCPU::initializeState() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-0.05f, 0.05f);

        for (int i{}; i < m_width * m_height * 2; i += 2) {
            m_data[i + 0] = 1.0f;
            m_data[i + 1] = 0.0f;
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
                    m_data[idx + 0] = 0.5f + dis(gen);
                    m_data[idx + 1] = 0.25f + dis(gen);
                }
            }
        }
    }

    float SimulationCPU::computeLaplacian(const std::vector<float>& field, int x, int y, int component) {
        int xm1{ (x - 1 + m_width) % m_width };
        int xp1{ (x + 1) % m_width };
        int ym1{ (y - 1 + m_height) % m_height };
        int yp1{ (y + 1) % m_height };

        int idx{ (y * m_width + x) * 2 + component };
        int left{ (y * m_width + xm1) * 2 + component };
        int right{ (y * m_width + xp1) * 2 + component };
        int up{ (ym1 * m_width + x) * 2 + component };
        int down{ (yp1 * m_width + x) * 2 + component };

        return field[left] + field[right] + field[up] + field[down] - 4.0f * field[idx];
    }

    void SimulationCPU::step(const SimulationParams& params) {
        for (int y{}; y < m_height; ++y) {
            for (int x{}; x < m_width; ++x) {
                int idx{ (y * m_width + x) * 2 };

                float u{ m_data[idx + 0] };
                float v{ m_data[idx + 1] };

                float laplacian_u{ computeLaplacian(m_data, x, y, 0) };
                float laplacian_v{ computeLaplacian(m_data, x, y, 1) };

                float uvv{ u * v * v };
                float du{ params.Du * laplacian_u - uvv + params.F * (1.0f - u) };
                float dv{ params.Dv * laplacian_v + uvv - (params.F + params.k) * v };

                m_dataNext[idx + 0] = std::clamp(u + du * params.dt, 0.0f, 1.0f);
                m_dataNext[idx + 1] = std::clamp(v + dv * params.dt, 0.0f, 1.0f);
            }
        }

        std::swap(m_data, m_dataNext);
    }

    void SimulationCPU::reset() {
        initializeState();
    }

    void SimulationCPU::syncFrom(const float* data) {
        std::copy(data, data + m_width * m_height * 2, m_data.begin());
    }

} // namespace GreyScott
