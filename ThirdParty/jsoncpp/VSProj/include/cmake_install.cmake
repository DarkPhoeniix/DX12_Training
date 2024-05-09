# Install script for directory: D:/DX12_Training/ThirdParty/jsoncpp/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/jsoncpp")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/json" TYPE FILE FILES
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/allocator.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/assertions.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/config.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/forwards.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/json.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/json_features.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/reader.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/value.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/version.h"
    "D:/DX12_Training/ThirdParty/jsoncpp/include/json/writer.h"
    )
endif()

