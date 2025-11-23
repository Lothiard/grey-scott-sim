#pragma once

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <string>
#include <vector>

namespace GreyScott {

    /**
     * @brief Device information structure
     */
    struct DeviceInfo {
        std::string name;
        std::string vendor;
        std::string version;
        cl_device_type type;
        size_t maxWorkGroupSize;
        cl_uint maxComputeUnits;
        cl_ulong globalMemSize;
        cl_ulong localMemSize;
        bool available;
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

        // Delete copy constructor and assignment operator
        ComputeManager(const ComputeManager&) = delete;
        ComputeManager& operator=(const ComputeManager&) = delete;

        /**
         * @brief Initialize OpenCL with the best available device
         * @return true on success
         */
        bool initialize();

        /**
         * @brief Initialize OpenCL with a specific device
         * @param platformIndex Index of the OpenCL platform to use
         * @param deviceIndex Index of the device within that platform
         * @return true on success
         */
        bool initialize(int platformIndex, int deviceIndex);

        /**
         * @brief Query and return all available OpenCL platforms and devices
         * @return Vector of device information for all available devices
         */
        std::vector<DeviceInfo> queryDevices() const;

        /**
         * @brief Print information about all available devices
         */
        void printDeviceInfo() const;

        /**
         * @brief Get information about the currently selected device
         */
        const DeviceInfo& getCurrentDeviceInfo() const {
            return m_currentDeviceInfo;
        }

        /**
         * @brief Check if OpenCL is initialized
         */
        bool isInitialized() const { return m_initialized; }

        /**
         * @brief Load and compile an OpenCL kernel from file
         * @param filename Path to the kernel source file
         * @param kernelName Name of the kernel function
         * @return Compiled kernel, or nullptr on failure
         */
        cl_kernel loadKernel(const std::string& filename,
                             const std::string& kernelName);

        /**
         * @brief Get the OpenCL context
         */
        cl_context getContext() const { return m_context; }

        /**
         * @brief Get the OpenCL command queue
         */
        cl_command_queue getQueue() const { return m_queue; }

        /**
         * @brief Get the OpenCL device
         */
        cl_device_id getDevice() const { return m_device; }

    private:
        /**
         * @brief Query device information from a device ID
         */
        DeviceInfo getDeviceInfo(cl_device_id device) const;

        /**
         * @brief Get device type as string
         */
        std::string getDeviceTypeString(cl_device_type type) const;

        /**
         * @brief Read kernel source from file
         */
        std::string readKernelSource(const std::string& filename) const;

        bool m_initialized;
        cl_platform_id m_platform;
        cl_device_id m_device;
        cl_context m_context;
        cl_command_queue m_queue;
        DeviceInfo m_currentDeviceInfo;
    };

} // namespace GreyScott
