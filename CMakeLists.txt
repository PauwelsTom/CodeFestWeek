cmake_minimum_required(VERSION 3.14)

# set the project name
project(EPITA_CODEFEST_WEEK)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Find required packages
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc calib3d)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Pseudo target to OpenNI
# add_library(OpenNI INTERFACE)
# target_include_directories(OpenNI INTERFACE thirdparty/OpenNI/Include)
# target_link_libraries(OpenNI INTERFACE ${CMAKE_SOURCE_DIR}/thirdparty/OpenNI/Redist/libOpenNI2.so)

add_library(libfreenect INTERFACE)
target_link_libraries(libfreenect INTERFACE ${CMAKE_SOURCE_DIR}/lib/libfreenect.a -lm -lusb-1.0)
target_include_directories(libfreenect INTERFACE include_lib)

# Add the executable
add_executable(glview src/glview.c)
target_link_libraries(glview PRIVATE libfreenect GLUT::GLUT OpenGL::GL)

add_executable(test-cv src/test-cv.cpp)

add_library(opencv_kinect src/capture-cv.hpp src/capture-cv.cpp src/utils.cpp src/calibration-utils.hpp src/calibration-utils.cpp)
target_link_libraries(opencv_kinect PRIVATE opencv_imgproc opencv_calib3d libfreenect Qt6::Core)
target_link_libraries(opencv_kinect PUBLIC opencv_core)

target_link_libraries(test-cv PRIVATE opencv_highgui opencv_kinect)

add_executable(calibration src/testZZZ.hh src/traitement.h src/calibrate-qt.cpp src/calibrate-qt-main.cpp src/our_test.cpp src/testZZZ.cpp src/traitement.cpp)
target_link_libraries(calibration PRIVATE Qt6::Gui Qt6::Widgets opencv_kinect ${OpenCV_LIBRARIES})
target_include_directories(calibration PRIVATE ${OpenCV_INCLUDE_DIRS})
