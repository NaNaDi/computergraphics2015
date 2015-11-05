cmake_minimum_required (VERSION 2.8)
# allows access to environment variables with the name 
project (OpenGL_Framework)

#set the build type if its not set
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif()

# build options
option(DYNAMIC_LINKING "Link libraries at runtime" ON)

# glbindings
if(DYNAMIC_LINKING)
    set(OPTION_BUILD_STATIC OFF CACHE BOOL "Build static libraries" FORCE)
else()
    set(OPTION_BUILD_STATIC ON CACHE BOOL "Build static libraries" FORCE)
    set(GLBINDING_STATIC ON CACHE BOOL "No help, variable specified on the command line." FORCE)
	add_definitions(-DGLBINDING_STATIC)
    # stb_image
    add_definitions(-DSTB_IMAGE_STATIC)
endif()    
# glfw
set(BUILD_SHARED_LIBS ${DYNAMIC_LINKING} CACHE BOOL "Build shared libraries" FORCE)


#before adding libraries, define the output paths, MSVC automatically creates the build-type folders
if(MSVC OR CMAKE_GENERATOR STREQUAL Xcode)
  SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
  SET(LIBRARY_OUTPUT_PATH	 ${PROJECT_BINARY_DIR})
else()
  SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
  SET(LIBRARY_OUTPUT_PATH	 ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

# include glm, as system header to suppress warnings
include_directories(SYSTEM external/glm-0.9.6.3)

# include stb_image, as system header to suppress warnings
include_directories(SYSTEM external/stb_image-2.0.6)

# add tinyobjloader
include_directories(external/tinyobjloader-aa07206)
# just one file so it can be directly compiled into main exe
file(GLOB TINYOBJLOADER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader-aa07206/*.cc)

# add glbindings
option(OPTION_BUILD_TESTS OFF)
add_subdirectory(external/glbinding-1.1.0)
include_directories(external/glbinding-1.1.0/source/glbinding/include)

# configure glfw building
option(GLFW_BUILD_DOCS     OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS    OFF)
option(GLFW_INSTALL        OFF)
# add glfw build system
add_subdirectory(external/glfw-3.1.1)
# include glfw headers
include_directories(external/glfw-3.1.1/include)

# include framework headers
include_directories(framework/include)
# output executables
file(GLOB FRAMEWORK_SOURCES framework/source/*.cpp)
add_executable(OpenGLFramework application/solar_system.cpp ${FRAMEWORK_SOURCES} ${TINYOBJLOADER_SOURCES})
target_include_directories(OpenGLFramework PRIVATE include)
target_link_libraries(OpenGLFramework glfw ${GLFW_LIBRARIES} glbinding)

# set build type dependent flags
if(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2")
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi")
endif()

# activate C++ 11
if(NOT MSVC)
    add_definitions(-std=c++11)
    # show warnings but suppress those caused by glm
    add_definitions(-Wall -Wconversion)
    # force linking  with c++11 lib
    if(APPLE)
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
        add_definitions(-stdlib=libc++)
    endif()
else()
    # build in parallel, show warnings and suppress one caused by glbinding
    add_definitions(/MP /W3 /wd4251)
endif()

# remove external configuration vars from cmake gui
mark_as_advanced(OPTION_BUILD_TESTS GMOCK_LIBRARY GMOCK_LIBRARY_DEBUG GTEST_LIBRARY
  GTEST_LIBRARY_DEBUG OPTION_BUILD_EXAMPLES OPTION_BUILD_TESTS OPTION_BUILD_STATIC
  OPTION_BUILD_TOOLS OPTION_GL_BY_STRINGS OPTION_PACK_GENERATOR OPTION_PORTABLE_INSTALL
  OPTION_STRINGS_BY_GL PYTHON INSTALL_MSVC_REDIST_FILEPATH glbinding_DIR GLBINDING_STATIC)
mark_as_advanced(GLFW_BUILD_DOCS GLFW_BUILD_TESTS GLFW_INSTALL GLFW_BUILD_EXAMPLES
 GLFW_DOCUMENT_INTERNALS GLFW_USE_EGL GLFW_USE_MIR GLFW_USE_WAYLAND GLFW_LIBRARIES
 LIB_SUFFIX BUILD_SHARED_LIBS)

#if user didnt set install dir, override it and write it to the cache -> Type and description necessary,
#overwrite variable, not just write it to cache
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE STRING "Install path prefix, prepended onto install directories." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# installation rules, copy over binaries to bin
install(TARGETS OpenGLFramework DESTINATION bin)
