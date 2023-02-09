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

find_package(Qt5 COMPONENTS Core Network CONFIG REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(spidevice REQUIRED)
find_package(ZeraClasses REQUIRED)

file(GLOB HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    )
file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

set(CONFIG_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/bridge.xml
    )
set(CONFIG_VALIDATOR
    ${CMAKE_CURRENT_SOURCE_DIR}/bridge.xsd
    )

# check for valid config files / display in qt-creator
add_custom_target(com5003-lwl-bridge-configs ALL
    DEPENDS ${CONFIG_VALIDATOR} ${CONFIG_FILES}
    SOURCES ${CONFIG_VALIDATOR} ${CONFIG_FILES}
    COMMAND xmllint --noout --schema ${CONFIG_VALIDATOR} ${CONFIG_FILES}
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

set_target_properties(com5003-lwl-bridge PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(com5003-lwl-bridge PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

enable_testing()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
