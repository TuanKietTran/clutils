#!/usr/bin/env bash
set -e

echo "Setting up clutils..."

# Always fresh vcpkg
rm -rf vcpkg
git clone --depth 1 https://github.com/microsoft/vcpkg.git

if [[ "$OSTYPE" == "darwin"* ]]; then
    ./vcpkg/bootstrap-vcpkg.sh
else
    ./vcpkg/bootstrap-vcpkg.sh 2>/dev/null || ./vcpkg/bootstrap-vcpkg.bat
fi

cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release --parallel

echo "Done! → ./build/clutils uuid"