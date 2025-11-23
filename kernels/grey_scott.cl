/**
 * Grey-Scott reaction-diffusion simulation kernel
 * 
 * dU/dt = Du * ∇²U - UV² + F(1-U)
 * dV/dt = Dv * ∇²V + UV² - (F+k)V
 * 
 * Where:
 * - Du, Dv: diffusion rates
 * - F: feed rate
 * - k: kill rate
 * - ∇²: Laplacian operator (discrete approximation)
 */
__kernel void grey_scott_step(
    __global const float2* current,  // Current state (U, V)
    __global float2* next,           // Next state (U, V)
    const float Du,                  // Diffusion rate for U
    const float Dv,                  // Diffusion rate for V
    const float F,                   // Feed rate
    const float k,                   // Kill rate
    const float dt,                  // Time step
    const int width,
    const int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);
    
    if (x >= width || y >= height) return;
    
    int idx = y * width + x;
    float2 uv = current[idx];
    float u = uv.x;
    float v = uv.y;
    
    // Periodic boundary conditions (toroidal topology)
    int xm1 = (x - 1 + width) % width;
    int xp1 = (x + 1) % width;
    int ym1 = (y - 1 + height) % height;
    int yp1 = (y + 1) % height;
    
    // 5-point stencil Laplacian
    float2 center = current[idx];
    float2 left   = current[y * width + xm1];
    float2 right  = current[y * width + xp1];
    float2 up     = current[ym1 * width + x];
    float2 down   = current[yp1 * width + x];
    
    float laplacian_u = left.x + right.x + up.x + down.x - 4.0f * center.x;
    float laplacian_v = left.y + right.y + up.y + down.y - 4.0f * center.y;
    
    // Grey-Scott equations
    float uvv = u * v * v;
    float du = Du * laplacian_u - uvv + F * (1.0f - u);
    float dv = Dv * laplacian_v + uvv - (F + k) * v;
    
    // Forward Euler integration
    next[idx].x = u + du * dt;
    next[idx].y = v + dv * dt;
    
    // Clamp to [0, 1] for stability
    next[idx].x = clamp(next[idx].x, 0.0f, 1.0f);
    next[idx].y = clamp(next[idx].y, 0.0f, 1.0f);
}
