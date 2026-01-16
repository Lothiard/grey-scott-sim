#pragma once

namespace GreyScott {
    struct SimulationParams {
        float Du{ 0.16f };
        float Dv{ 0.08f };
        float F{ 0.055f };
        float k{ 0.062f };
        float dt{ 1.0f };
    };
} // namespace GreyScott
