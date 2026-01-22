// src/remesh_api.cpp
#include <pmp/surface_mesh.h>
#include <pmp/algorithms/remeshing.h>
#include <pmp/algorithms/utilities.h>
#include <emscripten.h>
#include <vector>
#include <cstdint>

extern "C" {

// ============================================================
// 전역 출력 버퍼
// ------------------------------------------------------------
// 왜 전역인가?
// - WASM에서 동적 크기 배열을 반환하기 어려움
// - JS에서 결과 크기를 먼저 조회 → 포인터로 데이터 접근
// ============================================================
static std::vector<float> g_out_vertices;
static std::vector<uint32_t> g_out_indices;

// ============================================================
// 메인 리메싱 함수
// ============================================================
EMSCRIPTEN_KEEPALIVE
int remesh(
    const float* vertices,      // 입력 정점 [x0,y0,z0, x1,y1,z1, ...]
    int vertex_count,           // 정점 개수
    const uint32_t* indices,    // 입력 인덱스 [i0,i1,i2, ...] (삼각형)
    int index_count,            // 인덱스 개수
    float target_edge_ratio,    // 목표 엣지 길이 비율 (0.5=조밀, 2.0=단순)
    int iterations              // 리메싱 반복 횟수
) {
    try {
        pmp::SurfaceMesh mesh;

        // --------------------------------------------------------
        // 1) 정점 추가
        // --------------------------------------------------------
        std::vector<pmp::Vertex> vhandles;
        vhandles.reserve(vertex_count);
        
        for (int i = 0; i < vertex_count; i++) {
            pmp::Point p(
                vertices[i * 3 + 0],
                vertices[i * 3 + 1],
                vertices[i * 3 + 2]
            );
            vhandles.push_back(mesh.add_vertex(p));
        }

        // --------------------------------------------------------
        // 2) 삼각형 면 추가
        // --------------------------------------------------------
        int num_faces = index_count / 3;
        for (int i = 0; i < num_faces; i++) {
            uint32_t i0 = indices[i * 3 + 0];
            uint32_t i1 = indices[i * 3 + 1];
            uint32_t i2 = indices[i * 3 + 2];
            
            // 범위 체크
            if (i0 >= vertex_count || i1 >= vertex_count || i2 >= vertex_count) {
                return -2; // 인덱스 범위 초과
            }
            
            mesh.add_triangle(vhandles[i0], vhandles[i1], vhandles[i2]);
        }

        // --------------------------------------------------------
        // 3) 리메싱 실행
        // --------------------------------------------------------
        pmp::Scalar mean_edge = pmp::mean_edge_length(mesh);
        pmp::Scalar target_length = mean_edge * target_edge_ratio;
        
        // uniform_remeshing(mesh, 목표길이, 반복횟수, 표면투영여부)
        pmp::uniform_remeshing(mesh, target_length, iterations, true);

        // --------------------------------------------------------
        // 4) 결과 추출
        // --------------------------------------------------------
        g_out_vertices.clear();
        g_out_indices.clear();
        
        // 정점 인덱스 재매핑 (삭제된 정점이 있을 수 있음)
        std::vector<uint32_t> vertex_map(mesh.vertices_size(), UINT32_MAX);
        uint32_t new_idx = 0;
        
        for (auto v : mesh.vertices()) {
            pmp::Point p = mesh.position(v);
            g_out_vertices.push_back(p[0]);
            g_out_vertices.push_back(p[1]);
            g_out_vertices.push_back(p[2]);
            vertex_map[v.idx()] = new_idx++;
        }
        
        for (auto f : mesh.faces()) {
            for (auto v : mesh.vertices(f)) {
                g_out_indices.push_back(vertex_map[v.idx()]);
            }
        }

        return 0; // 성공
        
    } catch (const std::exception& e) {
        return -1; // 일반 에러
    } catch (...) {
        return -99; // 알 수 없는 에러
    }
}

// ============================================================
// 결과 조회 함수들
// ============================================================

EMSCRIPTEN_KEEPALIVE
int get_output_vertex_count() {
    return static_cast<int>(g_out_vertices.size() / 3);
}

EMSCRIPTEN_KEEPALIVE
int get_output_index_count() {
    return static_cast<int>(g_out_indices.size());
}

EMSCRIPTEN_KEEPALIVE
float* get_output_vertices() {
    return g_out_vertices.data();
}

EMSCRIPTEN_KEEPALIVE
uint32_t* get_output_indices() {
    return g_out_indices.data();
}

// ============================================================
// 유틸리티 (디버깅용)
// ============================================================

EMSCRIPTEN_KEEPALIVE
float get_mean_edge_length(
    const float* vertices, int vertex_count,
    const uint32_t* indices, int index_count
) {
    try {
        pmp::SurfaceMesh mesh;
        
        std::vector<pmp::Vertex> vhandles;
        for (int i = 0; i < vertex_count; i++) {
            vhandles.push_back(mesh.add_vertex(pmp::Point(
                vertices[i*3], vertices[i*3+1], vertices[i*3+2]
            )));
        }
        
        int num_faces = index_count / 3;
        for (int i = 0; i < num_faces; i++) {
            mesh.add_triangle(
                vhandles[indices[i*3]],
                vhandles[indices[i*3+1]],
                vhandles[indices[i*3+2]]
            );
        }
        
        return pmp::mean_edge_length(mesh);
    } catch (...) {
        return -1.0f;
    }
}

} // extern "C"