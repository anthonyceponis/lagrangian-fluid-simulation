#pragma once
#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl2.hpp>

struct GpuCompute {
  cl::Platform platform;
  cl::Device device;
  cl::Context context;
  cl::Program program;
  cl::CommandQueue queue;

  GpuCompute(std::string file_path);
};
