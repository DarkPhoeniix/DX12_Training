# =============================================================================
# Logger - logging facade over spdlog
# =============================================================================

# --- Dependencies ------------------------------------------------------------
find_package(spdlog CONFIG REQUIRED)

# --- Sources -----------------------------------------------------------------
file(GLOB_RECURSE Logger_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
)

# --- Target ------------------------------------------------------------------
add_library(Logger STATIC ${Logger_SOURCES})

# --- Output directories ------------------------------------------------------
foreach(config IN ITEMS DEBUG RELWITHDEBINFO RELEASE)
    set_property(TARGET Logger PROPERTY LIBRARY_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Logger PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}/Libs")
    set_property(TARGET Logger PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config} "${EXE_OUTPUT_DIR}")
endforeach()

# --- Includes / links --------------------------------------------------------
target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Logger PRIVATE spdlog::spdlog)

# --- Build options -----------------------------------------------------------
target_compile_definitions(Logger PRIVATE FILE_LOG)

if (MSVC)
    target_compile_definitions(Logger PRIVATE MSVC_LOG)
endif()

# --- IDE source grouping -----------------------------------------------------
include(${CMAKE_SOURCE_DIR}/Scripts/GroupSourcesByFolder.cmake)
GroupSourcesByFolder(Logger)
