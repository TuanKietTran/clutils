#!/usr/bin/env bash
set -e

echo "Setting up clutils..."

# === Smart vcpkg handling (respects cache) ===
if [ ! -d "vcpkg" ]; then
    echo "Cloning vcpkg (first time or cache miss)..."
    git clone --depth 1 https://github.com/microsoft/vcpkg.git
else
    echo "vcpkg directory exists → updating..."
    (cd vcpkg && git pull --depth 1)
fi

# Bootstrap only if vcpkg executable is missing
if [ ! -f "vcpkg/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    ./vcpkg/bootstrap-vcpkg.sh
else
    echo "vcpkg already bootstrapped"
fi

# Install OpenSSL (fast if cached)
echo "Installing OpenSSL via vcpkg..."
./vcpkg/vcpkg install openssl

# Build
echo "Building clutils..."
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release --parallel

echo "Done! → ./build/clutils uuid"