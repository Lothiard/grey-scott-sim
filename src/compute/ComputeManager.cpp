#include "ComputeManager.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

namespace GreyScott {

    ComputeManager::ComputeManager() :
        m_initialized(false),
        m_platform(nullptr),
        m_device(nullptr),
        m_context(nullptr),
        m_queue(nullptr) {}

    ComputeManager::~ComputeManager() {
        if (m_queue) { clReleaseCommandQueue(m_queue); }
        if (m_context) { clReleaseContext(m_context); }
    }

    bool ComputeManager::initialize() {
        if (m_initialized) {
            std::cerr << "ComputeManager already initialized!" << std::endl;
            return false;
        }

        cl_int err;
        cl_uint numPlatforms;

        // Get number of platforms
        err = clGetPlatformIDs(0, nullptr, &numPlatforms);
        if (err != CL_SUCCESS || numPlatforms == 0) {
            std::cerr << "Failed to find OpenCL platforms!" << std::endl;
            return false;
        }

        // Get platform IDs
        std::vector<cl_platform_id> platforms(numPlatforms);
        err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to get platform IDs!" << std::endl;
            return false;
        }

        std::cout << "Found " << numPlatforms << " OpenCL platform(s)"
                  << std::endl;

        // Try to find a GPU device, fall back to any device
        bool foundDevice = false;
        for (cl_uint i = 0; i < numPlatforms && !foundDevice; ++i) {
            cl_uint numDevices;
            err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, nullptr,
                                 &numDevices);

            if (err == CL_SUCCESS && numDevices > 0) {
                std::vector<cl_device_id> devices(numDevices);
                err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU,
                                     numDevices, devices.data(), nullptr);

                if (err == CL_SUCCESS) {
                    m_platform = platforms[i];
                    m_device = devices[0];
                    foundDevice = true;
                }
            }
        }

        // If no GPU found, try any device
        if (!foundDevice) {
            for (cl_uint i = 0; i < numPlatforms && !foundDevice; ++i) {
                cl_uint numDevices;
                err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0,
                                     nullptr, &numDevices);

                if (err == CL_SUCCESS && numDevices > 0) {
                    std::vector<cl_device_id> devices(numDevices);
                    err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL,
                                         numDevices, devices.data(), nullptr);

                    if (err == CL_SUCCESS) {
                        m_platform = platforms[i];
                        m_device = devices[0];
                        foundDevice = true;
                    }
                }
            }
        }

        if (!foundDevice) {
            std::cerr << "Failed to find any OpenCL device!" << std::endl;
            return false;
        }

        // Create context
        m_context =
            clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to create OpenCL context! Error: " << err
                      << std::endl;
            return false;
        }

        // Create command queue
        m_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                     nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to create command queue! Error: " << err
                      << std::endl;
            clReleaseContext(m_context);
            m_context = nullptr;
            return false;
        }

        // Get device info
        m_currentDeviceInfo = getDeviceInfo(m_device);

        std::cout << "OpenCL initialized successfully" << std::endl;
        std::cout << "  Device: " << m_currentDeviceInfo.name << std::endl;
        std::cout << "  Vendor: " << m_currentDeviceInfo.vendor << std::endl;
        std::cout << "  Type: " << getDeviceTypeString(m_currentDeviceInfo.type)
                  << std::endl;
        std::cout << "  Compute Units: " << m_currentDeviceInfo.maxComputeUnits
                  << std::endl;
        std::cout << "  Max Work Group Size: "
                  << m_currentDeviceInfo.maxWorkGroupSize << std::endl;
        std::cout << "  Global Memory: "
                  << (m_currentDeviceInfo.globalMemSize / (1024 * 1024))
                  << " MB" << std::endl;
        std::cout << "  Local Memory: "
                  << (m_currentDeviceInfo.localMemSize / 1024) << " KB"
                  << std::endl;

        m_initialized = true;
        return true;
    }

    bool ComputeManager::initialize(int platformIndex, int deviceIndex) {
        // TODO: Implement specific platform/device selection
        // For now, just use default initialization
        (void)platformIndex;
        (void)deviceIndex;
        return initialize();
    }

    std::vector<DeviceInfo> ComputeManager::queryDevices() const {
        std::vector<DeviceInfo> devices;
        cl_int err;
        cl_uint numPlatforms;

        err = clGetPlatformIDs(0, nullptr, &numPlatforms);
        if (err != CL_SUCCESS || numPlatforms == 0) { return devices; }

        std::vector<cl_platform_id> platforms(numPlatforms);
        err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
        if (err != CL_SUCCESS) { return devices; }

        for (cl_uint i = 0; i < numPlatforms; ++i) {
            cl_uint numDevices;
            err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr,
                                 &numDevices);

            if (err == CL_SUCCESS && numDevices > 0) {
                std::vector<cl_device_id> platformDevices(numDevices);
                err =
                    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices,
                                   platformDevices.data(), nullptr);

                if (err == CL_SUCCESS) {
                    for (cl_uint j = 0; j < numDevices; ++j) {
                        devices.push_back(getDeviceInfo(platformDevices[j]));
                    }
                }
            }
        }

        return devices;
    }

    void ComputeManager::printDeviceInfo() const {
        auto devices = queryDevices();

        std::cout << "\n=== Available OpenCL Devices ===" << std::endl;
        for (size_t i = 0; i < devices.size(); ++i) {
            const auto& dev = devices[i];
            std::cout << "\nDevice " << i << ":" << std::endl;
            std::cout << "  Name: " << dev.name << std::endl;
            std::cout << "  Vendor: " << dev.vendor << std::endl;
            std::cout << "  Version: " << dev.version << std::endl;
            std::cout << "  Type: " << getDeviceTypeString(dev.type)
                      << std::endl;
            std::cout << "  Compute Units: " << dev.maxComputeUnits
                      << std::endl;
            std::cout << "  Max Work Group Size: " << dev.maxWorkGroupSize
                      << std::endl;
            std::cout << "  Global Memory: "
                      << (dev.globalMemSize / (1024 * 1024)) << " MB"
                      << std::endl;
            std::cout << "  Local Memory: " << (dev.localMemSize / 1024)
                      << " KB" << std::endl;
            std::cout << "  Available: " << (dev.available ? "Yes" : "No")
                      << std::endl;
        }
        std::cout << "================================\n" << std::endl;
    }

    DeviceInfo ComputeManager::getDeviceInfo(cl_device_id device) const {
        DeviceInfo info;
        char buffer[1024];
        cl_uint uintVal;
        cl_ulong ulongVal;
        cl_device_type typeVal;
        cl_bool boolVal;

        clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buffer), buffer,
                        nullptr);
        info.name = buffer;

        clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buffer), buffer,
                        nullptr);
        info.vendor = buffer;

        clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(buffer), buffer,
                        nullptr);
        info.version = buffer;

        clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(typeVal), &typeVal,
                        nullptr);
        info.type = typeVal;

        clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(info.maxWorkGroupSize), &info.maxWorkGroupSize,
                        nullptr);

        clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(uintVal),
                        &uintVal, nullptr);
        info.maxComputeUnits = uintVal;

        clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ulongVal),
                        &ulongVal, nullptr);
        info.globalMemSize = ulongVal;

        clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(ulongVal),
                        &ulongVal, nullptr);
        info.localMemSize = ulongVal;

        clGetDeviceInfo(device, CL_DEVICE_AVAILABLE, sizeof(boolVal), &boolVal,
                        nullptr);
        info.available = (boolVal == CL_TRUE);

        return info;
    }

    std::string ComputeManager::getDeviceTypeString(cl_device_type type) const {
        switch (type) {
        case CL_DEVICE_TYPE_CPU: return "CPU";
        case CL_DEVICE_TYPE_GPU: return "GPU";
        case CL_DEVICE_TYPE_ACCELERATOR: return "Accelerator";
        case CL_DEVICE_TYPE_DEFAULT: return "Default";
        default: return "Unknown";
        }
    }

    std::string
    ComputeManager::readKernelSource(const std::string& filename) const {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open kernel file: " << filename
                      << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    cl_kernel ComputeManager::loadKernel(const std::string& filename,
                                         const std::string& kernelName) {
        if (!m_initialized) {
            std::cerr << "ComputeManager not initialized!" << std::endl;
            return nullptr;
        }

        // Read kernel source
        std::string source = readKernelSource(filename);
        if (source.empty()) { return nullptr; }

        cl_int err;
        const char* sourcePtr = source.c_str();
        size_t sourceSize = source.length();

        // Create program
        cl_program program = clCreateProgramWithSource(m_context, 1, &sourcePtr,
                                                       &sourceSize, &err);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to create program! Error: " << err
                      << std::endl;
            return nullptr;
        }

        // Build program
        err = clBuildProgram(program, 1, &m_device, nullptr, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to build program! Error: " << err << std::endl;

            // Get build log
            size_t logSize;
            clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &logSize);
            std::vector<char> log(logSize);
            clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG,
                                  logSize, log.data(), nullptr);
            std::cerr << "Build log:\n" << log.data() << std::endl;

            clReleaseProgram(program);
            return nullptr;
        }

        // Create kernel
        cl_kernel kernel = clCreateKernel(program, kernelName.c_str(), &err);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to create kernel '" << kernelName
                      << "'! Error: " << err << std::endl;
            clReleaseProgram(program);
            return nullptr;
        }

        clReleaseProgram(program);

        std::cout << "Loaded kernel '" << kernelName << "' from " << filename
                  << std::endl;
        return kernel;
    }

} // namespace GreyScott
