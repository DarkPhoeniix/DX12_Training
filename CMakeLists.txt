
cmake_minimum_required(VERSION 3.29)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows")

set(CMAKE_CONFIGURATION_TYPES Debug Profile Release CACHE STRING "" FORCE)

# Define Profile configuration type
# Copy Release flags as baseline
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)

# Add profiling/debug info and PROFILE define
string(APPEND CMAKE_CXX_FLAGS_DEBUG " /Zi /D_DEBUG")

# Add profiling/debug info and PROFILE define
string(APPEND CMAKE_CXX_FLAGS_PROFILE " /Zi /DPROFILE")

if(MSVC)
    add_compile_options(/MP)
    add_compile_definitions(
		WIN32_LEAN_AND_MEAN
		"SOLUTION_DIR=R\"(${CMAKE_SOURCE_DIR})\""
		"SOLUTION_DIR_W=LR\"(${CMAKE_SOURCE_DIR})\""
    )
endif()

option(USE_DX12 "Enable DirectX12 backend" ON)  # TODO: it doesn't work for now, fix it when implement Vulkan backend
option(USE_AFTERMATH "Enable Nvidia Aftermath SDK usage for crash tracking (Profile only)" ON)

# Fetch all dependencies
include(FetchContent)

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/External")
set(EXE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin")
set(LIB_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/Bin/Libs")
set(FETCHCONTENT_BASE_DIR "${EXTERNAL_DIR}/Source")

if (USE_AFTERMATH)
  set(AFTERMATH_SDK_DIR "${EXTERNAL_DIR}/NsightAftermathSDK")

  if(NOT EXISTS "${AFTERMATH_DIR}/include")  # or check for a .lib file, etc.
      message(STATUS "Running Aftermath SDK setup script...")
      execute_process(
          COMMAND cmd /c "${CMAKE_SOURCE_DIR}/Scripts/DownloadAftermathSDK.bat"
          WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Scripts"
          RESULT_VARIABLE result
      )
      if(NOT result EQUAL 0)
          message(FATAL_ERROR "Aftermath SDK setup failed with code ${result}")
      endif()
  endif()
  
  set(AFTERMATH_DIR "${EXTERNAL_DIR}/NsightAftermathSDK")

  add_library(Aftermath STATIC IMPORTED)
  set_target_properties(Aftermath PROPERTIES
      IMPORTED_LOCATION "${AFTERMATH_DIR}/lib/x64/GFSDK_Aftermath_Lib.x64.lib"
      INTERFACE_INCLUDE_DIRECTORIES "${AFTERMATH_DIR}/include"
  )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(EquinoxEngine)

add_subdirectory(Source/RHI)
add_subdirectory(Source/Editor)
add_subdirectory(Source/GPUCrashTracker)
add_subdirectory(Source/Logger)
add_subdirectory(Source/Renderer)
add_subdirectory(Source/RenderGraph)
add_subdirectory(Source/Utility)
