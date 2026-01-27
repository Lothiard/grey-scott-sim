#pragma once

#ifdef USE_OPENCL

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <string>
#include <vector>

namespace GreyScott {
    struct DeviceInfo {
        std::string name{};
        std::string vendor{};
        std::string version{};
        cl_device_type type{};
        size_t maxWorkGroupSize{};
        cl_uint maxComputeUnits{};
        cl_ulong globalMemSize{};
        cl_ulong localMemSize{};
        bool available{};
    };

    /**
     * @brief Manages OpenCL initialization, context, command queue, and kernel
     * execution
     *
     * This class handles all OpenCL-related operations including:
     * - Platform and device enumeration
     * - Context and command queue creation
     * - Kernel compilation and execution
     * - Memory buffer management
     */
    class ComputeManager {
    public:
        ComputeManager();
        ~ComputeManager();

        ComputeManager(const ComputeManager&) = delete;
        ComputeManager& operator=(const ComputeManager&) = delete;

        bool initialize();
        std::vector<DeviceInfo> queryDevices() const;
        void printDeviceInfo() const;

        const DeviceInfo& getCurrentDeviceInfo() const {
            return m_currentDeviceInfo;
        }

        bool isInitialized() const { return m_initialized; }
        bool hasGLInterop() const { return m_hasGLInterop; }

        cl_kernel loadKernel(const std::string& filename,
                             const std::string& kernelName);

        cl_context getContext() const { return m_context; }
        cl_command_queue getQueue() const { return m_queue; }
        cl_device_id getDevice() const { return m_device; }

    private:
        DeviceInfo getDeviceInfo(cl_device_id device) const;
        bool checkGLInteropSupport() const;

        std::string getDeviceTypeString(cl_device_type type) const;
        std::string readKernelSource(const std::string& filename) const;

        bool m_initialized{};
        bool m_hasGLInterop{};
        cl_platform_id m_platform{};
        cl_device_id m_device{};
        cl_context m_context{};
        cl_command_queue m_queue{};
        DeviceInfo m_currentDeviceInfo{};
    };

} // namespace GreyScott

#endif // USE_OPENCL
