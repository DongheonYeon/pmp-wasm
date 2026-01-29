cmake_minimum_required(VERSION 3.16)
project(pmp_wasm
    DESCRIPTION "A WebAssembly port of the Polygon Mesh Processing library for Three.js"
    HOMEPAGE_URL "https://github.com/DongheonYeon/pmp-wasm"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(pmp-library)
set(PMP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(PMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(PMP_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(PMP_BUILD_VIS OFF CACHE BOOL "" FORCE)

add_executable(remesh_module src/remesh_api.cpp)

target_link_libraries(remesh_module PRIVATE pmp)

target_include_directories(remesh_module PRIVATE 
    ${CMAKE_SOURCE_DIR}/pmp-library/src
)

if(EMSCRIPTEN)
    set_target_properties(remesh_module PROPERTIES
        SUFFIX ".js"
    )
    
    target_link_options(remesh_module PRIVATE
        # Export C functions
        "SHELL:-s EXPORTED_FUNCTIONS=['_remesh','_get_output_vertex_count','_get_output_index_count','_get_output_vertices','_get_output_indices','_analyze_mesh','_get_stat_vertices','_get_stat_edges','_get_stat_faces','_get_stat_mean_edge','_get_stat_min_edge','_get_stat_max_edge','_analyze_mesh_detailed','_get_detailed_std_dev','_get_detailed_median','_get_detailed_edges_to_split','_get_detailed_edges_to_collapse','_get_detailed_histogram','_get_detailed_histogram_bins','_get_detailed_bin_min','_get_detailed_bin_max','_malloc','_free']"

        # JS runtime methods
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAPF32','HEAPU32','HEAP32']"
        
        # 메모리 설정
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s INITIAL_MEMORY=67108864"
        
        # modularize
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s EXPORT_NAME='createRemeshModule'"
        
        -O3
        "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
    )
endif()