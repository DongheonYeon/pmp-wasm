#include <pmp/surface_mesh.h>
#include <pmp/algorithms/remeshing.h>
#include <pmp/algorithms/utilities.h>
#include <emscripten.h>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>


extern "C" {

// ============================================================
// 전역 출력 버퍼
// ------------------------------------------------------------
// WASM -> JS로 동적 크기 배열 반환 불가
// C++ 내부에서는 vector로 전역 동적 크기 배열을 사용하고
// JS에서 결과 크기를 먼저 조회 -> 포인터로 데이터 접근
// ============================================================
static std::vector<float> g_out_vertices;
static std::vector<uint32_t> g_out_indices;

EMSCRIPTEN_KEEPALIVE
int remesh(
    const float* vertices,      // 입력 정점 [x0,y0,z0, x1,y1,z1, ...]
    int vertex_count,           // 버텍스 개수
    const uint32_t* indices,    // 입력 인덱스 [i0,i1,i2, ...] (삼각형)
    int index_count,            // 인덱스 개수
    float target_edge_ratio,    // 목표 엣지 길이 비율, 작을수록 세밀함
    int iterations,             // 반복 횟수
    int edge_mode,              // 기준 엣지: 0=mean, 1=min
    int use_projection          // 표면 투영 여부: 0=false, 1=true
) {
    try {
        pmp::SurfaceMesh mesh;

        // 1) 버텍스 추가
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

        // 2) 페이스 추가
        int num_faces = index_count / 3;
        for (int i = 0; i < num_faces; i++) {
            uint32_t i0 = indices[i * 3 + 0];
            uint32_t i1 = indices[i * 3 + 1];
            uint32_t i2 = indices[i * 3 + 2];
            
            if (i0 >= vertex_count || i1 >= vertex_count || i2 >= vertex_count) {
                return -2; // 인덱스 범위 초과
            }
            
            mesh.add_triangle(vhandles[i0], vhandles[i1], vhandles[i2]);
        }

        // 3) 리메싱 실행
        pmp::Scalar base_edge = (edge_mode == 1)
            ? pmp::min_edge_length(mesh)
            : pmp::mean_edge_length(mesh);
        pmp::Scalar target_length = base_edge * target_edge_ratio;

        pmp::uniform_remeshing(mesh, target_length, iterations, use_projection != 0);

        // 4) 결과 추출
        g_out_vertices.clear();
        g_out_indices.clear();
        
        // 버텍스 인덱스 재매핑 (삭제된 정점이 있을 수 있음)
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

        return 0;
        
    } catch (const std::exception& e) {
        return -1; // 표준 에러
    } catch (...) {
        return -99; // unkown 에러
    }
}


// ============================================================
// Util
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
// Helper
// ============================================================
inline float calc_edge_length(const pmp::SurfaceMesh& mesh, pmp::Edge e) {
    auto h = mesh.halfedge(e, 0);
    auto v0 = mesh.from_vertex(h);
    auto v1 = mesh.to_vertex(h);
    return pmp::norm(mesh.position(v1) - mesh.position(v0));
}

inline float calc_max_edge_length(const pmp::SurfaceMesh& mesh) {
    float max_len = 0;
    for (auto e : mesh.edges()) {
        float len = calc_edge_length(mesh, e);
        if (len > max_len) max_len = len;
    }
    return max_len;
}

// ============================================================
// 일반 통계 버퍼
// ============================================================
static struct {
    int n_vertices;
    int n_edges;
    int n_faces;
    float mean_edge;
    float min_edge;
    float max_edge;
} g_mesh_stats;

EMSCRIPTEN_KEEPALIVE
int analyze_mesh(
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

        // 통계 저장
        g_mesh_stats.n_vertices = mesh.n_vertices();
        g_mesh_stats.n_edges = mesh.n_edges();
        g_mesh_stats.n_faces = mesh.n_faces();
        g_mesh_stats.mean_edge = pmp::mean_edge_length(mesh);
        g_mesh_stats.min_edge = pmp::min_edge_length(mesh);
        g_mesh_stats.max_edge = calc_max_edge_length(mesh);

        return 0;
    } catch (...) {
        return -1;
    }
}

// 통계 getter
EMSCRIPTEN_KEEPALIVE int get_stat_vertices() { return g_mesh_stats.n_vertices; }
EMSCRIPTEN_KEEPALIVE int get_stat_edges() { return g_mesh_stats.n_edges; }
EMSCRIPTEN_KEEPALIVE int get_stat_faces() { return g_mesh_stats.n_faces; }
EMSCRIPTEN_KEEPALIVE float get_stat_mean_edge() { return g_mesh_stats.mean_edge; }
EMSCRIPTEN_KEEPALIVE float get_stat_min_edge() { return g_mesh_stats.min_edge; }
EMSCRIPTEN_KEEPALIVE float get_stat_max_edge() { return g_mesh_stats.max_edge; }

// ============================================================
// 상세 통계 버퍼
// ============================================================
constexpr int HISTOGRAM_BINS = 10;

static struct {
    // 기본 통계
    float std_dev;          // 표준편차
    float median;           // 중앙값
    // 영향받는 엣지 (target_length 기준)
    int edges_to_split;     // 분할될 엣지 (> target * 4/3)
    int edges_to_collapse;  // 병합될 엣지 (< target * 4/5)
    // 히스토그램
    int histogram[HISTOGRAM_BINS];
    float bin_min;          // 히스토그램 최소값
    float bin_max;          // 히스토그램 최대값
} g_detailed_stats;

EMSCRIPTEN_KEEPALIVE
int analyze_mesh_detailed(
    const float* vertices, int vertex_count,
    const uint32_t* indices, int index_count,
    float target_edge_ratio,    // 목표 엣지 비율
    int edge_mode               // 0=mean, 1=min 기준
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

        // 모든 엣지 길이 수집
        std::vector<float> edge_lengths;
        edge_lengths.reserve(mesh.n_edges());
        for (auto e : mesh.edges()) {
            edge_lengths.push_back(calc_edge_length(mesh, e));
        }

        if (edge_lengths.empty()) return -1;

        // 기본 통계
        float min_e = *std::min_element(edge_lengths.begin(), edge_lengths.end());
        float max_e = *std::max_element(edge_lengths.begin(), edge_lengths.end());
        float mean_e = pmp::mean_edge_length(mesh);

        // 표준편차
        float variance = 0;
        for (float len : edge_lengths) {
            float diff = len - mean_e;
            variance += diff * diff;
        }
        g_detailed_stats.std_dev = std::sqrt(variance / edge_lengths.size());

        // 중앙값
        std::vector<float> sorted = edge_lengths;
        std::sort(sorted.begin(), sorted.end());
        size_t n = sorted.size();
        g_detailed_stats.median = (n % 2 == 0)
            ? (sorted[n/2 - 1] + sorted[n/2]) / 2.0f
            : sorted[n/2];

        // target_length 계산
        float base_edge = (edge_mode == 1) ? min_e : mean_e;
        float target_length = base_edge * target_edge_ratio;

        // 영향받는 엣지 수
        float split_threshold = target_length * (4.0f / 3.0f);
        float collapse_threshold = target_length * (4.0f / 5.0f);

        g_detailed_stats.edges_to_split = 0;
        g_detailed_stats.edges_to_collapse = 0;
        for (float len : edge_lengths) {
            if (len > split_threshold) g_detailed_stats.edges_to_split++;
            if (len < collapse_threshold) g_detailed_stats.edges_to_collapse++;
        }

        // 히스토그램
        g_detailed_stats.bin_min = min_e;
        g_detailed_stats.bin_max = max_e;
        std::fill(std::begin(g_detailed_stats.histogram),
                  std::end(g_detailed_stats.histogram), 0);

        float range = max_e - min_e;
        if (range > 0) {
            for (float len : edge_lengths) {
                int bin = static_cast<int>((len - min_e) / range * HISTOGRAM_BINS);
                bin = std::min(bin, HISTOGRAM_BINS - 1);
                g_detailed_stats.histogram[bin]++;
            }
        } else {
            g_detailed_stats.histogram[0] = edge_lengths.size();
        }

        return 0;
    } catch (...) {
        return -1;
    }
}

// 상세 통계 getter
EMSCRIPTEN_KEEPALIVE float get_detailed_std_dev() { return g_detailed_stats.std_dev; }
EMSCRIPTEN_KEEPALIVE float get_detailed_median() { return g_detailed_stats.median; }
EMSCRIPTEN_KEEPALIVE int get_detailed_edges_to_split() { return g_detailed_stats.edges_to_split; }
EMSCRIPTEN_KEEPALIVE int get_detailed_edges_to_collapse() { return g_detailed_stats.edges_to_collapse; }
EMSCRIPTEN_KEEPALIVE int* get_detailed_histogram() { return g_detailed_stats.histogram; }
EMSCRIPTEN_KEEPALIVE int get_detailed_histogram_bins() { return HISTOGRAM_BINS; }
EMSCRIPTEN_KEEPALIVE float get_detailed_bin_min() { return g_detailed_stats.bin_min; }
EMSCRIPTEN_KEEPALIVE float get_detailed_bin_max() { return g_detailed_stats.bin_max; }

} // extern "C"