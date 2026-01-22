cmake_minimum_required(VERSION 3.16)
project(pmp_wasm_remesh CXX)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# (선택) 빌드 타입 기본값
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# PMP: 불필요한 타겟 빌드 끄기
set(PMP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PMP_BUILD_VIS      OFF CACHE BOOL "" FORCE)
set(PMP_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(PMP_BUILD_TESTS    OFF CACHE BOOL "" FORCE)

# submodule 추가
add_subdirectory(pmp-library)

# 실행 파일
add_executable(remesh-js main.cpp)

# PMP 링크
target_link_libraries(remesh-js PRIVATE pmp)
