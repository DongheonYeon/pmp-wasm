A WebAssembly port of the Polygon Mesh Processing library for Three.js

### Prepare
No need to build pmp lib.
Install Emscripten

### Build test case
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

### Usage:
`./build/remesht-js`

input: obj file

output: obj file

### Build wasm:

### Directory Structure
```
pmp-wasm
├── emsdk/
├── pmp-library/
├── src/
│   └── remesh_api.cpp
└── 
```


### Reference
https://github.com/pmp-library/pmp-library.git