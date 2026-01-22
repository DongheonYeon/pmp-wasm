#include <pmp/surface_mesh.h>
#include <pmp/algorithms/remeshing.h>
#include <pmp/algorithms/utilities.h>
#include <pmp/io/io.h>
#include <iostream>

int main() {
    pmp::SurfaceMesh mesh;
    std::string input_file = "test.obj";
    std::cout << "Loading " << input_file << "..." << std::endl;
    
    try {
        pmp::read(mesh, input_file);
    } catch (const std::exception& e) {
        std::cerr << "Error loading mesh: " << e.what() << std::endl;
        return 1;
    }

    // 리메싱 목표 설정
    // 평균 엣지 길이를 계산하여 기준으로 삼거나, 사용자가 원하는 수치를 입력합니다.
    pmp::Scalar mean_edge = pmp::mean_edge_length(mesh);
    pmp::Scalar target_edge_length = mean_edge * 0.5;
    // 만약 더 촘촘하게 하려면: target_edge_length = mean_edge * 0.5;
    // 만약 더 단순하게 하려면: target_edge_length = mean_edge * 2.0;
    
    std::cout << "Current mean edge length: " << mean_edge << std::endl;
    std::cout << "Target edge length: " << target_edge_length << std::endl;

    // Uniform Remeshing
    // 파라미터: (타겟 길이, 반복 횟수, 투영 여부)
    // 반복 횟수(10)와 원래 표면 투영(true)은 일반적인 권장값입니다.
    pmp::uniform_remeshing(mesh, target_edge_length, 10, true);

    std::cout << "Remeshing completed." << std::endl;

    // 결과 저장
    std::string output_file = "test_remeshed.obj";
    pmp::write(mesh, output_file);
    std::cout << "Saved to " << output_file << std::endl;

    return 0;
}
