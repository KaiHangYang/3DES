CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(3DES)

SET(IS_TEST FALSE)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Ceres REQUIRED)
find_package(OpenCV 3.1.0 REQUIRED)
find_package(Caffe REQUIRED)
#CONFIGURE_FILE (
    #"${PROJECT_SOURCE_DIR}/config.h.in"
    #"${PROJECT_SOURCE_DIR}/config.h"
    #)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
    #message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
# 根据Caffe的config的的说法，Caffe_INCLUDE_DIRS 未定义 因为包含在了IMPORTED中了
get_property(Caffe_INCLUDE_DIRS TARGET caffe PROPERTY INTERFACE_INCLUDE_DIRECTORIES)

OPTION (IS_TEST
    "Use the test code" ON
    )

IF (IS_TEST)
    SET(LINK_LIBS ${CERES_LIBRARIES})
    SET(INCLUDE_DIRS ${CERES_INCLUDE_DIRS})
    SET(CMAKE_CXX_COMPILER "g++")
    SET(CMAKE_CXX_FLAGS "-std=c++11")
    SET(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib")
    SET(CMAKE_BUILD_TYPE Debug)
ELSE()
    SET(LINK_LIBS ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} glfw ${ASSIMP_LIBRARIES} opencv_videoio ${CERES_LIBRARIES} ${Caffe_LIBRARIES})
    SET(INCLUDE_DIRS ${INCLUDE_DIRS} "./include" ${CERES_INCLUDE_DIRS} ${Caffe_INCLUDE_DIRS})
    SET(CMAKE_CXX_COMPILER "g++")
    SET(CMAKE_CXX_FLAGS "-std=c++11")
    SET(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib")
    SET(CMAKE_EXE_STATIC_FLAGS "-L/usr/local/lib")
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

IF (IS_TEST)
    ADD_EXECUTABLE(main test.cpp)
    # 下面必须使用caffe包的LIBRARIES因为 这里可以满足caffe的所有依赖，如果手动添加链接的话会需要手动添加依赖库
    TARGET_LINK_LIBRARIES(main ${LINK_LIBS})
ELSE()
    # 这个cmake 是真的神奇 有时间需要好好看看
    ADD_SUBDIRECTORY(src/utils)
    ADD_SUBDIRECTORY(src/vnect)
    ADD_EXECUTABLE(main main.cpp)
    TARGET_LINK_LIBRARIES(main utils vnect ${LINK_LIBS})
ENDIF()
