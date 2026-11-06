# ZC

[![CI](https://github.com/Paul272007/ZC/actions/workflows/ci.yml/badge.svg)](https://github.com/paul/ZC/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++26](https://img.shields.io/badge/C%2B%2B-26-blue.svg)](https://en.cppreference.com/w/cpp/compiler_support)
[![C23](https://img.shields.io/badge/C-23-blue.svg)](https://en.cppreference.com/w/c/23)

The package manager and build system for C and C++ !

---

## 🛠️ Build Prerequisites

* **OS**: macOS or Linux (it will most certainly not work on Windows)
* **C++ Compiler**: Clang 23+ or GCC 16+ with C++26 support
* **Build System**: [CMake](https://cmake.org) (>= 3.25) and [Ninja](https://ninja-build.org)
* **Tooling**: `clang-format`, `clang-tidy`, `pre-commit`
* **Dependencies**:
  * LLVM & Clang development libraries
  * `libcurl`
  * `libarchive`
  * `OpenSSL`
  * `nlohmann/json`

### Installing Dependencies

#### macOS
```bash
brew install cmake ninja llvm libarchive nlohmann-json openssl
```

#### Ubuntu / Debian
```bash
sudo apt-get update && sudo apt-get install -y \
  clang clang-tidy ninja-build \
  llvm-dev libclang-dev \
  libarchive-dev libcurl4-openssl-dev libssl-dev \
  nlohmann-json3-dev
```

---

## 🚀 Building & Testing

This project uses **CMake Presets** for standardized builds.

### 1. Configure
```bash
# Debug build (with ASan & UBSan enabled)
cmake --preset debug

# Or Release build
cmake --preset release
```

### 2. Build
```bash
# Build Debug
cmake --build --preset debug

# Or Build Release
cmake --build --preset release
```

### 3. Run Tests
```bash
ctest --preset debug
```

---

## 🧹 Code Quality & Pre-commit

Before committing, install and run pre-commit hooks to ensure formatting and linting pass:

```bash
# Install git hooks
pre-commit install --hook-type pre-commit --hook-type commit-msg

# Run all checks manually
pre-commit run --all-files
```

This repository adheres to [Conventional Commits](https://www.conventionalcommits.org/) (enforced by `commitlint`).

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).
