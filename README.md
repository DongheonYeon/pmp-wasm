A WebAssembly port of the Polygon Mesh Processing library for Three.js

---

## Project Structure

```
pmp-wasm/
├── pmp-library/             # git submodule
├── src/
│   └── remesh_api.cpp       # C API wrapper
├── build/
│   ├── remesh_module.js
│   └── remesh_module.wasm
└── CMakeLists.txt           # WASM 빌드 설정
```

---
## 1. C API Wrapper
`src/remesh_api.cpp`

WASM으로 컴파일된 C++ 코드를 JS로 호출하려면 인터페이스가 필요하다. `extern "C" API`를 통해 name mangling 없이 컴파일하여 JS가 함수이름만으로 직접 호출할 수 있도록 한다. `EMSCRIPTEN_KEEPALIVE` 매크로는 Emscripten 컴파일러가 사용되지 않는 함수로 판단해서 제거하는 것을 방지한다.


### Key Points

| 항목 | 설명 |
|------|------|
| `extern "C"` | C++ name mangling 방지, JS에서 `_remesh`로 호출 가능 |
| `EMSCRIPTEN_KEEPALIVE` | 컴파일러가 함수 제거하지 않도록 보존 |
| 전역 버퍼 | WASM에서 동적 배열 반환 불가 → 전역 저장 후 포인터 접근 |
| 에러 코드 | 0=성공, -1=일반에러, -2=인덱스초과, -99=알수없음 |

---

## 2: CMake Configuration
`CMakeLists.txt`

- CMake 옵션 설명
  | 옵션 | 설명 |
  |------|------|
  | `EXPORTED_FUNCTIONS` | JS에서 호출할 C 함수 목록 (`_` prefix 필수) |
  | `EXPORTED_RUNTIME_METHODS` | HEAPF32, HEAPU32 등 메모리 접근에 필요 |
  | `ALLOW_MEMORY_GROWTH` | 동적 메모리 확장 허용 |
  | `MODULARIZE` | `createPMPModule()` 형태로 모듈 생성 |
  | `DISABLE_EXCEPTION_CATCHING=0` | PMP 내부 예외 처리 활성화 |

### 주요 옵션 설명
- EXPORTED_FUNCTIONS

  `"SHELL:-s EXPORTED_FUNCTIONS=['_remesh','_malloc','_free',...]"`

  - JS에서 Module._remesh() 형태로 호출할 함수들
  - C 함수명 앞에 _ 붙여야 함
  - _malloc, _free는 JS에서 WASM 힙 메모리 관리에 필수

- EXPORTED_RUNTIME_METHODS
  
  `cmake"SHELL:-s EXPORTED_RUNTIME_METHODS=['HEAPF32','HEAPU32',...]"`
  
  메서드/용도
  - HEAPF32Float32 데이터 읽기/쓰기
  - HEAPU32Uint32 데이터 읽기/쓰기
  - ccallC 함수 직접 호출
  - cwrapC 함수를 JS 함수로 래핑

- MODULARIZE + EXPORT_NAME
  ```
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s EXPORT_NAME='createPMPModule'"
  ```
  출력된 JS 파일을 이렇게 사용 가능:
  ```
  javascriptimport createPMPModule from './pmp_remesh.js';
  const Module = await createPMPModule();
  ```

- DISABLE_EXCEPTION_CATCHING=0
  ```
  "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
  ```
  - PMP 내부에서 예외를 사용하므로 예외 처리 활성화
  - 없으면 런타임 에러 발생

---

## 3. Build WASM
emcc 실행
```bash
cd emsdk
source ./emsdk_env.sh
```

CMake 실행

```bash
mkdir build
cd build
emcmake cmake ..
```

성공하면 이런 메시지가 보임:
```bash
-- Configuring done
-- Generating done
-- Build files have been written to: .../pmp-wasm/build-wasm
```

빌드 실행
```bash
emmake make
```

Verify output (Optional)
```bash
ls -la remesh_module.*
# remesh_module.js         (JS glue code)
# remesh_module.wasm       (WASM binary)
```

---

## 4: Frontend Integration

- FE Project Structure (Vite + TypeScript)

  ```
  root/
  ├── public/
  │   └── wasm/
  │       ├── pmp_remesh.js
  │       └── pmp_remesh.wasm
  └── src/
      └── .../utils/
          ├── pmpRemesh.ts         # WASM wrapper
          └── remeshThreeMesh.ts   # Three.js helper
  ```

### 구조
```
const result = await remesh(vertices, indices, 0.5, 10);

┌─────────────────────────────────────────────────────────────┐
│  Three.js 코드                                              │
│                                                             │
│  const result = await remesh(vertices, indices, 0.5, 10);  │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  JS Wrapper (remesh.ts)                                     │
│                                                             │
│  - TypedArray → WASM 힙 복사                                │
│  - C 함수 호출                                              │
│  - 결과 → TypedArray 복사                                   │
│  - 메모리 정리                                              │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────┐
│  WASM (remesh_module.wasm)                                  │
└─────────────────────────────────────────────────────────────┘
```


- JavaScript WASM Wrapper
  - `src/lib/pmpRemesh.ts`

- Three.js Helper
  - `src/lib/remeshThreeMesh.ts`

- 사용 예시
  ```
  import { remeshGeometry, preloadWasm } from './lib/remeshThreeMesh';

  // 앱 시작 시 미리 로드 (선택사항)
  await preloadWasm();

  // 특정 메시 리메싱
  const newGeometry = await remeshGeometry(mesh.geometry, {
    targetEdgeRatio: 0.5,  // 더 조밀하게
    iterations: 10
  });

  mesh.geometry.dispose();
  mesh.geometry = newGeometry;
  ```

---

## 5. Usage Example

```typescript
import { remeshGeometry, preloadWasm } from '.../remeshThreeMesh';

// 앱 초기화 시 미리 로드 (선택사항)
await preloadWasm();

// 메시 리메싱
const newGeometry = await remeshGeometry(mesh.geometry, {
  targetEdgeRatio: 0.5,  // 더 조밀하게
  iterations: 10
});

mesh.geometry.dispose();
mesh.geometry = newGeometry;
```

---

## Parameters

| 파라미터 | 설명 |
|----------|------|
| `targetEdgeRatio` | 현재 평균 엣지 길이 / 타겟 엣지 길이 비율 |
| `iterations` | 반복 횟수 |

---

## Reference

- Polygon Mesh Processing Library (https://github.com/pmp-library.git)
- Emscripten SDK (https://github.com/emscripten-core/emsdk.git)