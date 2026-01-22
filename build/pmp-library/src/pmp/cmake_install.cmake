# Install script for directory: /home/user/Projects/pmp-wasm/pmp-library/src/pmp

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpmp.so.3.0.0" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpmp.so.3.0.0")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpmp.so.3.0.0"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/user/Projects/pmp-wasm/build/pmp-library/libpmp.so.3.0.0")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpmp.so.3.0.0" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpmp.so.3.0.0")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpmp.so.3.0.0")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/user/Projects/pmp-wasm/build/pmp-library/libpmp.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pmp" TYPE FILE FILES
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./bounding_box.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./exceptions.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./mat_vec.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./memory_usage.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./properties.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./stop_watch.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./surface_mesh.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./types.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pmp/algorithms" TYPE FILE FILES
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/barycentric_coordinates.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/curvature.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/decimation.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/differential_geometry.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/distance_point_triangle.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/fairing.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/features.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/geodesics.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/hole_filling.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/laplace.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/normals.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/numerics.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/parameterization.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/remeshing.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/shapes.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/smoothing.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/subdivision.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/triangulation.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./algorithms/utilities.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pmp/io" TYPE FILE FILES
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/helpers.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/io.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/io_flags.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/read_obj.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/read_off.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/read_pmp.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/read_stl.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/write_obj.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/write_off.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/write_pmp.h"
    "/home/user/Projects/pmp-wasm/pmp-library/src/pmp/./io/write_stl.h"
    )
endif()

