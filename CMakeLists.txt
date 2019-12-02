cmake_minimum_required(VERSION 3.5)
project(gles)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
include_directories(#../rk_opengl/native/include
					/usr/lib64/libmali/include
					/usr/include/libdrm
					./)
#include_directories(/usr/lib64/libmali/include)

#link_directories(./native/lib)
#link_directories(/usr/lib)
set(link_libs /usr/lib/aarch64-linux-gnu/libdrm.so
			  /usr/lib/libmali-gbm.so
			  /usr/lib/aarch64-linux-gnu/libpthread.so
			  )
add_executable(gles gles_main.cpp gles_base.cpp gles_drm.cpp v4l2.cpp)
#add_executable(glsample_wl main_wayland.cpp)

#target_include_directories(gles PUBLIC ./native/include/EGLs/winsys_gbm)
#target_include_directories(glsample_wl PUBLIC ./native/include/EGLs/winsys_wayland)

target_link_libraries(gles ${link_libs})
#target_link_libraries(glsample_wl mali-wayland drm)
