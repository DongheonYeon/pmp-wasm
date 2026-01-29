import * as THREE from "three";

import { preloadWasm, remesh } from "./remeshWasmWrapper";

export { preloadWasm };

export interface RemeshOptions {
  targetEdgeRatio?: number;
  iterations?: number;
}

/**
 * Three.js BufferGeometry에서 vertices/indices 추출
 */
function extractGeometryData(geometry: THREE.BufferGeometry): {
  vertices: Float32Array;
  indices: Uint32Array;
} {
  const posAttr = geometry.getAttribute("position");
  if (!posAttr) {
    throw new Error("Geometry has no position attribute");
  }

  const vertices = new Float32Array(posAttr.array);

  let indices: Uint32Array;
  if (geometry.index) {
    indices = new Uint32Array(geometry.index.array);
  } else {
    // Non-indexed geometry -> indexed로 변환
    indices = new Uint32Array(posAttr.count);
    for (let i = 0; i < posAttr.count; i++) {
      indices[i] = i;
    }
  }

  return { vertices, indices };
}

/**
 * Three.js Mesh를 리메싱
 * @returns 새로운 BufferGeometry (원본은 유지)
 */
export async function createRemeshedGeometry(
  geometry: THREE.BufferGeometry,
  options: RemeshOptions = {}
): Promise<THREE.BufferGeometry> {
  const { targetEdgeRatio = 0.5, iterations = 10 } = options;

  const { vertices, indices } = extractGeometryData(geometry);

  // console.log(`[Remesh] Input: ${vertices.length / 3} vertices, ${indices.length / 3} faces`);

  const result = await remesh({
    vertices,
    indices,
    targetEdgeRatio,
    iterations,
  });

  // console.log(`[Remesh] Output: ${result.vertexCount} vertices, ${result.faceCount} faces`);

  // 새 BufferGeometry 생성
  const newGeometry = new THREE.BufferGeometry();
  newGeometry.setAttribute("position", new THREE.BufferAttribute(result.vertices, 3));
  newGeometry.setIndex(new THREE.BufferAttribute(result.indices, 1));
  newGeometry.computeVertexNormals();

  return newGeometry;
}

/**
 * Mesh 객체의 geometry를 직접 교체
 */
export async function remeshGeometry(mesh: THREE.Mesh, options: RemeshOptions = {}): Promise<void> {
  const oldGeometry = mesh.geometry as THREE.BufferGeometry;
  const newGeometry = await createRemeshedGeometry(oldGeometry, options);

  mesh.geometry = newGeometry;
  oldGeometry.dispose(); // 메모리 해제
}
