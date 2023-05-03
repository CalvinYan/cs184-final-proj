# CMakeLists.txt

option(BUILD_LIBCGL    "Build with libCGL"            ON)

# Older versions of CMake are likely to work just fine but, since
# I don't know where to cut off I just use the version I'm using
cmake_minimum_required(VERSION "3.17")

# name of this example project
project(cs184-final-proj)

# set OpenCV_DIR variable equal to the path to the cmake
# files within the previously installed opencv program
set(OpenCV_DIR ~/install/lib/cmake/opencv4)

# Tell compiler to use C++ 14 features which is needed because
# Clang version is often behind in the XCode installation
set(CMAKE_CXX_STANDARD 14)

# configure the necessary common CMake environment variables
# needed to include and link the OpenCV program into this
# demo project, namely OpenCV_INCLUDE_DIRS and OpenCV_LIBS
find_package( OpenCV REQUIRED )

# tell the build to include the headers from OpenCV
include_directories( ${OpenCV_INCLUDE_DIRS} )

# specify the executable target to be built
add_executable(ar-live src/main.cpp)

# tell it to link the executable target against OpenCV
target_link_libraries(ar-live ${OpenCV_LIBS} )

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


#-------------------------------------------------------------------------------
# Find dependencies
#-------------------------------------------------------------------------------

# Required packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glfw3 3.3 REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(ar-live ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

# target_link_libraries(ar-live OpenGL::GL)
target_link_libraries(ar-live glfw)
# find_package(Threads REQUIRED)
# if(NOT WIN32)
#   find_package(Freetype REQUIRED)
# endif()

# CGL
if(BUILD_LIBCGL)
  add_subdirectory(CGL)
  include_directories(CGL/include)
else(BUILD_LIBCGL)
  find_package(CGL REQUIRED)
endif(BUILD_LIBCGL)
