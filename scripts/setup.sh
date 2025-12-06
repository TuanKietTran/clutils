#!/usr/bin/env bash
set -e

echo "Setting up clutils..."

# Smart vcpkg (cache-aware)
if [ ! -d "vcpkg" ]; then
    echo "Cloning vcpkg..."
    git clone --depth 1 https://github.com/microsoft/vcpkg.git
else
    echo "vcpkg exists → updating..."
    (cd vcpkg && git pull --depth 1)
fi

if [ ! -f "vcpkg/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    ./vcpkg/bootstrap-vcpkg.sh
fi

# Install OpenSSL (smart: works in both classic & manifest mode)
echo "Installing dependencies..."
if [ -f "vcpkg.json" ]; then
    ./vcpkg/vcpkg install               # manifest mode
else
    ./vcpkg/vcpkg install openssl       # classic mode
fi

# Build
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release --parallel

echo "Done! → ./build/clutils uuid"