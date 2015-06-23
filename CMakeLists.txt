cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(QTouch)

find_package(Qt5 REQUIRED COMPONENTS Widgets Qml Quick)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Add module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Include helpers
include(Melpers)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Distinguish between release and debug builds
if(CMAKE_BUILD_TYPE MATCHES "Release")
    # Disable qDebug output in release builds
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
else()
    # Enable verbose makefiles in debug builds
    set(CMAKE_VERBOSE_MAKEFILE ON)
    # QML-Debugging
    # Note: You need to pass e.g. "-qmljsdebugger=port:56346,block" to the application
    # to enable QML-Debugging.
    # You may also consider to export QML_DISABLE_OPTIMIZER=1 in your execution environment.
    add_definitions(-DQT_QML_DEBUG -DQT_DECLARATIVE_DEBUG)

    # TODO: Remove me! (Private headers)
#    include_directories(/usr/include/x86_64-linux-gnu/qt5/QtQml/5.3.2/QtQml)
endif()

# QT_USE_FAST_CONCATENATION and QT_USE_FAST_OPERATOR_PLUS seems to be deprecated
# since 4.8 and replaced by QT_USE_QSTRINGBUILDER
add_definitions(-DQT_USE_QSTRINGBUILDER)

add_subdirectory(src)
add_subdirectory(src/gui)
print_list("Source files:" SRCS)

qt5_add_resources(QRCS
    src/gui/qml.qrc
    resources/resources.qrc
)
print_list("Resource files:" QRCS)

add_executable(QTouch ${SRCS} ${QRCS})
target_link_libraries(QTouch Qt5::Widgets Qt5::Qml Qt5::Quick)
