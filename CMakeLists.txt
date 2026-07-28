cmake_minimum_required(VERSION 3.1)

project(com5003-lwl-bridge LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "19")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

include(FeatureSummary)
include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(FindPkgConfig)

find_package(Qt5 COMPONENTS Core Network CONFIG REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(spidevice REQUIRED)
find_package(ZeraClasses REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED libsystemd)
# better suggestions?
get_filename_component(SYSTEMD_UNIT_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/../lib/systemd/system" ABSOLUTE)

file(GLOB HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    )
file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

add_executable(com5003-lwl-bridge
    ${HEADER}
    ${SOURCES}
    )

target_link_libraries(com5003-lwl-bridge
    PUBLIC
    Qt5::Core
    Qt5::Network
    Zera::zera-math
    Zera::zera-xmlconfig
    spidevice
    )

target_include_directories(com5003-lwl-bridge
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

install(TARGETS com5003-lwl-bridge
    EXPORT com5003-lwl-bridge
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

enable_testing()

add_subdirectory(configs)
add_subdirectory(systemd)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
