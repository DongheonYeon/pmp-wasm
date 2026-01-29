interface RemeshModule {
  _remesh(
    vertices: number,
    vertexCount: number,
    indices: number,
    indexCount: number,
    targetEdgeRatio: number,
    iterations: number,
    edgeMode: number,
    useProjection: number
  ): number;
  _get_output_vertex_count(): number;
  _get_output_index_count(): number;
  _get_output_vertices(): number;
  _get_output_indices(): number;
  _analyze_mesh(
    vertices: number,
    vertexCount: number,
    indices: number,
    indexCount: number
  ): number;
  _get_stat_vertices(): number;
  _get_stat_edges(): number;
  _get_stat_faces(): number;
  _get_stat_mean_edge(): number;
  _get_stat_min_edge(): number;
  _get_stat_max_edge(): number;
  // 상세 통계
  _analyze_mesh_detailed(
    vertices: number,
    vertexCount: number,
    indices: number,
    indexCount: number,
    targetEdgeRatio: number,
    edgeMode: number
  ): number;
  _get_detailed_std_dev(): number;
  _get_detailed_median(): number;
  _get_detailed_edges_to_split(): number;
  _get_detailed_edges_to_collapse(): number;
  _get_detailed_histogram(): number;
  _get_detailed_histogram_bins(): number;
  _get_detailed_bin_min(): number;
  _get_detailed_bin_max(): number;
  _malloc(size: number): number;
  _free(ptr: number): void;
  HEAPF32: Float32Array;
  HEAPU32: Uint32Array;
  HEAP32: Int32Array;
}

let modulePromise: Promise<RemeshModule> | null = null;
declare global {
  interface Window {
    createRemeshModule?: (options?: {
      locateFile?: (path: string) => string;
    }) => Promise<RemeshModule>;
  }
}

async function loadScript(src: string): Promise<void> {
  return new Promise((resolve, reject) => {
    if (window.createRemeshModule) {
      resolve();
      return;
    }

    const script = document.createElement("script");
    script.src = src;
    script.onload = () => resolve();
    script.onerror = () => reject(new Error(`Failed to load script: ${src}`));
    document.head.appendChild(script);
  });
}

async function getModule(): Promise<RemeshModule> {
  if (!modulePromise) {
    modulePromise = (async () => {
      await loadScript("/wasm/remesh_module.js");

      if (!window.createRemeshModule) {
        throw new Error("Failed to load WASM module");
      }
      const module = await window.createRemeshModule({
        locateFile: (path: string) => {
          if (path.endsWith(".wasm")) {
            return "/wasm/remesh_module.wasm";
          }
          return path;
        },
      });

      return module;
    })();
  }
  return modulePromise;
}

/**
 * Types
 */
export type EdgeMode = "mean" | "min";

export interface RemeshInput {
  vertices: Float32Array;
  indices: Uint32Array;
  targetEdgeRatio: number;
  iterations: number;
  edgeMode?: EdgeMode;       // 기본값: "mean"
  useProjection?: boolean;   // 기본값: true
}

export interface RemeshOutput {
  vertices: Float32Array;
  indices: Uint32Array;
  vertexCount: number;
  faceCount: number;
}

/**
 * Errors
 */
const ERROR_MESSAGES: Record<number, string> = {
  [-1]: "Standard error during remeshing",
  [-2]: "Index out of bounds",
  [-99]: "Unknown error",
};

/**
 * Main function
 */
export async function remesh(input: RemeshInput): Promise<RemeshOutput> {
  const M = await getModule();

  const { vertices, indices, targetEdgeRatio, iterations, edgeMode = "mean", useProjection = true } = input;
  const vertexCount = vertices.length / 3;
  const indexCount = indices.length;
  const edgeModeNum = edgeMode === "min" ? 1 : 0;

  if (vertices.length % 3 !== 0) {
    throw new Error("Vertices array length must be multiple of 3");
  }
  if (indices.length % 3 !== 0) {
    throw new Error("Indices array length must be multiple of 3 (triangles)");
  }

  // WASM 힙에 메모리 할당
  const vertexBytes = vertices.byteLength;
  const indexBytes = indices.byteLength;

  const vertexPtr = M._malloc(vertexBytes);
  const indexPtr = M._malloc(indexBytes);

  if (!vertexPtr || !indexPtr) {
    if (vertexPtr) M._free(vertexPtr);
    if (indexPtr) M._free(indexPtr);
    throw new Error("Failed to allocate WASM memory");
  }

  try {
    // 데이터를 WASM 힙에 복사
    M.HEAPF32.set(vertices, vertexPtr / 4);
    M.HEAPU32.set(indices, indexPtr / 4);

    // C 함수 호출
    const result = M._remesh(
      vertexPtr,
      vertexCount,
      indexPtr,
      indexCount,
      targetEdgeRatio,
      iterations,
      edgeModeNum,
      useProjection ? 1 : 0
    );

    // 에러 체크
    if (result !== 0) {
      const message = ERROR_MESSAGES[result] || `Remesh failed with code ${result}`;
      throw new Error(message);
    }

    // 결과 크기 조회
    const outVertexCount = M._get_output_vertex_count();
    const outIndexCount = M._get_output_index_count();

    // 결과 포인터 얻기
    const outVertexPtr = M._get_output_vertices();
    const outIndexPtr = M._get_output_indices();

    // 결과를 JS 배열로 복사
    const outVertices = new Float32Array(
      M.HEAPF32.buffer,
      outVertexPtr,
      outVertexCount * 3
    ).slice();

    const outIndices = new Uint32Array(M.HEAPU32.buffer, outIndexPtr, outIndexCount).slice();

    return {
      vertices: outVertices,
      indices: outIndices,
      vertexCount: outVertexCount,
      faceCount: outIndexCount / 3,
    };
  } finally {
    M._free(vertexPtr);
    M._free(indexPtr);
  }
}


/**
 * Utils
 */
export async function preloadWasm(): Promise<void> {
  await getModule();
}


export interface MeshStats {
  vertices: number;
  edges: number;
  faces: number;
  meanEdge: number;
  minEdge: number;
  maxEdge: number;
}

export async function getMeshStats(
  vertices: Float32Array,
  indices: Uint32Array
): Promise<MeshStats> {
  const M = await getModule();

  const vertexPtr = M._malloc(vertices.byteLength);
  const indexPtr = M._malloc(indices.byteLength);

  try {
    M.HEAPF32.set(vertices, vertexPtr / 4);
    M.HEAPU32.set(indices, indexPtr / 4);

    const result = M._analyze_mesh(vertexPtr, vertices.length / 3, indexPtr, indices.length);
    if (result !== 0) {
      throw new Error("Failed to analyze mesh");
    }

    return {
      vertices: M._get_stat_vertices(),
      edges: M._get_stat_edges(),
      faces: M._get_stat_faces(),
      meanEdge: M._get_stat_mean_edge(),
      minEdge: M._get_stat_min_edge(),
      maxEdge: M._get_stat_max_edge(),
    };
  } finally {
    M._free(vertexPtr);
    M._free(indexPtr);
  }
}


export interface MeshStatsDetailed {
  // 엣지 길이 통계
  stdDev: number;
  median: number;
  // 영향받는 엣지 수
  edgesToSplit: number;     // 분할될 엣지 (target_length * 4/3)
  edgesToCollapse: number;  // 병합될 엣지 (target_length * 4/5)
  // 히스토그램
  histogram: number[];      // 각 구간별 엣지 수
  binMin: number;           // 히스토그램 최소값
  binMax: number;           // 히스토그램 최대값
}

export interface DetailedAnalysisInput {
  vertices: Float32Array;
  indices: Uint32Array;
  targetEdgeRatio: number;
  edgeMode?: EdgeMode;
}

export async function getMeshStatsDetailed(
  input: DetailedAnalysisInput
): Promise<MeshStatsDetailed> {
  const M = await getModule();
  const { vertices, indices, targetEdgeRatio, edgeMode = "mean" } = input;
  const edgeModeNum = edgeMode === "min" ? 1 : 0;

  const vertexPtr = M._malloc(vertices.byteLength);
  const indexPtr = M._malloc(indices.byteLength);

  try {
    M.HEAPF32.set(vertices, vertexPtr / 4);
    M.HEAPU32.set(indices, indexPtr / 4);

    const result = M._analyze_mesh_detailed(
      vertexPtr,
      vertices.length / 3,
      indexPtr,
      indices.length,
      targetEdgeRatio,
      edgeModeNum
    );

    if (result !== 0) {
      throw new Error("Failed to analyze mesh");
    }

    // 히스토그램 읽기
    const histogramPtr = M._get_detailed_histogram();
    const histogramBins = M._get_detailed_histogram_bins();
    const histogram = new Int32Array(
      M.HEAP32.buffer,
      histogramPtr,
      histogramBins
    ).slice();

    return {
      stdDev: M._get_detailed_std_dev(),
      median: M._get_detailed_median(),
      edgesToSplit: M._get_detailed_edges_to_split(),
      edgesToCollapse: M._get_detailed_edges_to_collapse(),
      histogram: Array.from(histogram),
      binMin: M._get_detailed_bin_min(),
      binMax: M._get_detailed_bin_max(),
    };
  } finally {
    M._free(vertexPtr);
    M._free(indexPtr);
  }
}
