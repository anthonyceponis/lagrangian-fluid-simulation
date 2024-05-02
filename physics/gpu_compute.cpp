#include "gpu_compute.hpp"
#include <CL/cl2.hpp>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>

GpuCompute::GpuCompute(std::string file_path) {
  // Get the default platform (driver).
  std::vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);
  if (all_platforms.size() == 0) {
    throw std::runtime_error("No platforms found. Check OpenCL installation!");
  }
  this->platform = all_platforms[0];
  std::cout << "Using platform: " << this->platform.getInfo<CL_PLATFORM_NAME>()
            << "\n";

  // Get the default device (gpu) for the default platform.
  std::vector<cl::Device> all_devices;
  this->platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
  if (all_devices.size() == 0) {
    throw std::runtime_error("No devices found. Check OpenCL installation!");
  }
  this->device = all_devices[0];
  std::cout << "Using device: " << this->device.getInfo<CL_DEVICE_NAME>()
            << "\n";

  // Create compute context.
  this->context = cl::Context({this->device});

  // Read kernel source code from file.
  cl::Program::Sources sources;

  std::ifstream file(file_path);

  if (!file.is_open()) {
    throw std::invalid_argument(
        "Failed to open file. Check that source is correct.");
  }

  std::stringstream ss;
  ss << file.rdbuf();

  file.close();

  std::string source_code = ss.str();
  sources.push_back({source_code.c_str(), source_code.size()});

  // Compile source to create GPU program.
  this->program = cl::Program(this->context, sources);
  if (program.build({this->device}) != CL_SUCCESS) {
    std::stringstream msg;
    msg << "Error building: "
        << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(this->device);
    throw std::runtime_error(msg.str());
  }

  // Create queue for sending buffers and running kernels.
  this->queue = cl::CommandQueue(this->context, this->device);
}
