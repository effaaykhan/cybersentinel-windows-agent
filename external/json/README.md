# nlohmann/json Dependency

This directory should contain the nlohmann/json header-only library.

## Installation Options

### Option 1: vcpkg (Recommended)
```bash
vcpkg install nlohmann-json:x64-windows
```

### Option 2: Manual Download
Download the single-header version:
```bash
curl -o include/nlohmann/json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
```

### Option 3: Git Submodule
```bash
cd external/json
git init
git submodule add https://github.com/nlohmann/json.git .
```

## Usage

The CMakeLists.txt includes this directory automatically:
```cmake
include_directories(${CMAKE_SOURCE_DIR}/external/json/include)
```

If using vcpkg, the package manager will handle the include paths automatically.
